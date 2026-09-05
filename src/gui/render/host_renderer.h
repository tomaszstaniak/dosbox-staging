// SPDX-FileCopyrightText:  2026 Boxer NG contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DOSBOX_HOST_RENDERER_H
#define DOSBOX_HOST_RENDERER_H

#include "render_backend.h"

#include <string>
#include <vector>

#include "embedded.h"
#include "gui/private/common.h"
#include "gui/render/render.h"
#include "dosbox_config.h"
#include "utils/rect.h"

// must be included after dosbox_config.h
#include <SDL3/SDL.h>

// Render backend for embedding hosts (see embedded.h, DOSBOX_HostVideo).
//
// Owns the DOS-facing framebuffer and hands each finished frame to the host
// in EndFrame(); the host copies it out synchronously, so the buffer is free
// for reuse the moment EndFrame() returns. Creates a hidden SDL window only
// because the rest of the GUI layer requires one; nothing is ever drawn to it.
class HostRenderer : public RenderBackend {
public:
	HostRenderer(const int x, const int y, const int width, const int height,
	             const SDL_WindowFlags sdl_window_flags,
	             const DOSBOX_HostVideo& host);
	~HostRenderer() override;

	SDL_Window* GetWindow() override;
	DosBox::Rect GetCanvasSizeInPixels() override;
	void NotifyViewportSizeChanged(const DosBox::Rect draw_rect_px) override;
	void NotifyRenderSizeChanged(const int new_render_width_px,
	                             const int new_render_height_px) override;
	void NotifyVideoModeChanged(const VideoMode& video_mode) override;

	SetShaderResult SetShader(const std::string& symbolic_shader_descriptor) override;
	void ForceReloadCurrentShader() override;
	ShaderInfo GetCurrentShaderInfo() override;
	ShaderPreset GetCurrentShaderPreset() override;
	std::string GetCurrentSymbolicShaderDescriptor() override;
	ShaderDescriptor GetCurrentShaderDescriptor() override;

	void StartFrame(uint32_t*& pixels_out, int& pitch_out) override;
	void EndFrame() override;
	void PrepareFrame() override;
	void PresentFrame() override;

	void SetVsync(const bool is_enabled) override;
	void SetColorSpace(const ColorSpace color_space) override;
	void SetImageAdjustmentSettings(const ImageAdjustmentSettings& settings) override;
	void EnableImageAdjustments(const bool enable) override;
	void SetDeditheringStrength(const float strength) override;

	RenderedImage ReadPixelsPostShader(const DosBox::Rect output_rect_px) override;

	uint32_t MakePixel(const uint8_t red, const uint8_t green,
	                   const uint8_t blue) override;

	HostRenderer(const HostRenderer&)            = delete;
	HostRenderer& operator=(const HostRenderer&) = delete;

private:
	SDL_Window* window     = {};
	DOSBOX_HostVideo host  = {};

	// 32-bit BGRX pixels, no row padding: pitch = width * 4
	std::vector<uint32_t> framebuf = {};
	int render_width_px  = 0;
	int render_height_px = 0;
	int pitch_bytes      = 0;

	bool frame_in_progress = false;
	bool has_new_frame     = false;
};

#endif // DOSBOX_HOST_RENDERER_H
