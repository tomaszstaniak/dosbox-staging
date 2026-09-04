// SPDX-FileCopyrightText:  2026 The Boxer NG Authors
// SPDX-License-Identifier: GPL-2.0-or-later

// Embedded entry point.
//
// Boxer runs the emulator inside its own process rather than as a standalone
// executable. main() is unsuitable for that: it installs an atexit() handler
// and assumes it owns process teardown.
//
// DOSBOX_RunEmbedded() is main()'s body with those assumptions removed.
//
// Threading: run this on the host's existing emulator path. Boxer documents
// that threads other than the main thread are unsupported (BXEmulator.h), and
// SDL video on macOS is only safe on the main thread. Do not spawn a thread
// for it.
//
// Teardown: DOSBOX_RunEmbedded() already calls GFX_Quit() before returning on
// the success path. Do NOT also call DOSBOX_ShutdownEmbedded() after a normal
// return — that would call GFX_Quit() twice.
//
// DOSBOX_ShutdownEmbedded() is currently only a GFX_Quit(); it does not
// destroy modules or reset `control`, so the emulator cannot yet be restarted
// in-process. Proper teardown (module destruction, control reset, RAII on the
// error paths) is deliberately deferred until after the first successful
// embedded launch.

#ifndef DOSBOX_EMBEDDED_H
#define DOSBOX_EMBEDDED_H

#ifdef __cplusplus
extern "C" {
#endif

// Runs the emulator to completion on the calling thread. Returns the same code
// main() would have returned. Does NOT install an atexit() handler and does not
// terminate the process.
int DOSBOX_RunEmbedded(int argc, char* argv[]);

// Optional callback invoked once the emulator is fully initialised — after
// GFX_InitAndStartGui() and mapper setup, immediately before
// SHELL_InitAndRun() hands control to the shell.
//
// This exists because a host cannot otherwise tell when "started" means
// "ready": DOSBOX_RunEmbedded() blocks until the emulator exits, so anything
// the host signals before calling it is signalled too early. Set this before
// calling DOSBOX_RunEmbedded(); pass NULL to clear.
//
// One-shot: the registration is cleared before the callback is invoked, so a
// stale host pointer can never be called a second time. Re-register if the
// emulator is run again.
//
// Called on the same thread as DOSBOX_RunEmbedded().
typedef void (*DOSBOX_ReadyCallback)(void* context);
void DOSBOX_SetReadyCallback(DOSBOX_ReadyCallback callback, void* context);

// Asks the running emulator to exit. Safe to call from the host's AppKit
// termination handler, including re-entrantly from inside DOSBOX_RunEmbedded()
// (SDL pumps the host run loop, so a Cmd-Q arrives while RunEmbedded is still
// on the stack).
//
// This only *requests* exit: DOSBOX_RunEmbedded() unwinds and returns normally
// afterwards. The host must let that return happen before terminating the
// process -- terminating while the emulator and its audio threads are live is
// what produces the crash reports.
void DOSBOX_RequestExitEmbedded(void);

// Appends host-supplied lines to the end of the generated Z:\AUTOEXEC.BAT and
// regenerates it, together with any environment variables recorded through
// AUTOEXEC_SetVariable() since start-up. This is how a host that mounted its
// own drives in the ready callback can then launch a program on them: the
// host derives the DOS path from the live drive and hands over e.g.
// "C:", "CD \GAME", "GAME.EXE".
//
// Only valid from the ready callback, i.e. after modules are initialised and
// before the shell exists. Lines are executed even under --noautoexec, which
// only suppresses the [autoexec] sections of configuration files. Returns
// false, changing nothing, if called too early or once the shell is running.
// Call it on the DOSBOX_RunEmbedded() thread.
bool DOSBOX_AppendAutoexecLines(const char* const* lines, int count);

// Releases graphics and console resources.
//
// Do NOT call this after a normal DOSBOX_RunEmbedded() return: that path
// already calls GFX_Quit(). It exists for hosts that need to clean up after
// DOSBOX_RunEmbedded() was never invoked, or threw before its own teardown.
// It does not destroy modules or reset `control`, so the emulator cannot yet
// be restarted in-process.
void DOSBOX_ShutdownEmbedded(void);

#ifdef __cplusplus
}
#endif

#endif // DOSBOX_EMBEDDED_H
