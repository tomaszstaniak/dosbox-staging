// SPDX-FileCopyrightText:  2026 Boxer NG contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "host_renderer.h"

#include <cassert>
#include <stdexcept>

#include "misc/video.h"
#include "utils/checks.h"
#include "utils/string_utils.h"

CHECK_NARROWING();

HostRenderer::HostRenderer(const int x, const int y, const int width,
                           const int height, const SDL_WindowFlags sdl_window_flags,
                           const DOSBOX_HostVideo& host_video)
        : host(host_video)
{
	// Hidden: the host draws. Never fullscreen: there is nothing to show.
	SDL_WindowFlags flags = (sdl_window_flags | SDL_WINDOW_HIDDEN) &
	                        ~static_cast<SDL_WindowFlags>(SDL_WINDOW_FULLSCREEN);

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, DOSBOX_NAME);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
	window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);

	if (!window) {
		const auto msg = format_str("HOST: Error creating hidden window: %s",
		                            SDL_GetError());
		LOG_ERR("%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	LOG_MSG("HOST: Frames are delivered to the embedding host; SDL window stays hidden");
}

HostRenderer::~HostRenderer()
{
	if (window) {
		SDL_DestroyWindow(window);
		window = {};
	}
}

SDL_Window* HostRenderer::GetWindow()
{
	return window;
}

DosBox::Rect HostRenderer::GetCanvasSizeInPixels()
{
	int w = 0;
	int h = 0;
	host.canvas_size(host.context, &w, &h);

	// The GUI layer asserts a positive canvas. Fall back to the current
	// render size, then to a VGA-ish default, rather than trip that.
	if (w <= 0 || h <= 0) {
		w = render_width_px > 0 ? render_width_px : 640;
		h = render_height_px > 0 ? render_height_px : 480;
	}
	return {w, h};
}

void HostRenderer::NotifyViewportSizeChanged([[maybe_unused]] const DosBox::Rect draw_rect_px)
{
	// The host decides where the picture goes in its own view.
}

void HostRenderer::NotifyRenderSizeChanged(const int new_render_width_px,
                                           const int new_render_height_px)
{
	assert(new_render_width_px > 0 && new_render_height_px > 0);

	render_width_px  = new_render_width_px;
	render_height_px = new_render_height_px;
	pitch_bytes      = render_width_px * static_cast<int>(sizeof(uint32_t));

	framebuf.assign(static_cast<size_t>(render_width_px) * render_height_px, 0);
	frame_in_progress = false;
	has_new_frame     = false;

	host.render_size_changed(host.context, render_width_px, render_height_px);
}

void HostRenderer::NotifyVideoModeChanged(const VideoMode& video_mode)
{
	host.video_mode_changed(host.context,
	                        video_mode.is_graphics_mode ? 0 : 1,
	                        video_mode.width,
	                        video_mode.height,
	                        static_cast<int>(video_mode.pixel_aspect_ratio.Num()),
	                        static_cast<int>(video_mode.pixel_aspect_ratio.Denom()));
}

HostRenderer::SetShaderResult HostRenderer::SetShader(
        [[maybe_unused]] const std::string& symbolic_shader_descriptor)
{
	// No shader support here; the host applies its own. Reporting anything
	// but success would trigger the fallback chain and a hard exit.
	return SetShaderResult::Ok;
}

void HostRenderer::ForceReloadCurrentShader() {}
ShaderInfo HostRenderer::GetCurrentShaderInfo() { return {}; }
ShaderPreset HostRenderer::GetCurrentShaderPreset() { return {}; }
std::string HostRenderer::GetCurrentSymbolicShaderDescriptor() { return {}; }
ShaderDescriptor HostRenderer::GetCurrentShaderDescriptor() { return {}; }

void HostRenderer::StartFrame(uint32_t*& pixels_out, int& pitch_out)
{
	assert(!framebuf.empty());
	pixels_out        = framebuf.data();
	pitch_out         = pitch_bytes;
	frame_in_progress = true;
}

void HostRenderer::EndFrame()
{
	if (!frame_in_progress) {
		return;
	}
	frame_in_progress = false;

	// The host copies the frame out before returning; after this call the
	// VGA emulation may overwrite the buffer again without tearing anything
	// the host will show.
	host.upload(host.context, framebuf.data(), pitch_bytes, render_width_px,
	            render_height_px);
	has_new_frame = true;
}

void HostRenderer::PrepareFrame()
{
	// Nothing to prepare: upload happened in EndFrame().
}

void HostRenderer::PresentFrame()
{
	if (has_new_frame) {
		has_new_frame = false;
		host.present(host.context);
	}
}

void HostRenderer::SetVsync([[maybe_unused]] const bool is_enabled) {}
void HostRenderer::SetColorSpace([[maybe_unused]] const ColorSpace color_space) {}
void HostRenderer::SetImageAdjustmentSettings([[maybe_unused]] const ImageAdjustmentSettings& settings) {}
void HostRenderer::EnableImageAdjustments([[maybe_unused]] const bool enable) {}
void HostRenderer::SetDeditheringStrength([[maybe_unused]] const float strength) {}

RenderedImage HostRenderer::ReadPixelsPostShader([[maybe_unused]] const DosBox::Rect output_rect_px)
{
	// Post-shader captures are the host's business; never requested here
	// because PresentFrame() does not call GFX_CaptureRenderedImage().
	return {};
}

uint32_t HostRenderer::MakePixel(const uint8_t red, const uint8_t green,
                                 const uint8_t blue)
{
	// Same layout as the other backends: 0xFFRRGGBB, i.e. B,G,R,X in memory.
	return (static_cast<uint32_t>(blue) << 0) | (static_cast<uint32_t>(green) << 8) |
	       (static_cast<uint32_t>(red) << 16) | (static_cast<uint32_t>(255) << 24);
}
