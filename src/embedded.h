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

// Releases graphics and console resources. Safe to call more than once, and
// safe to call if DOSBOX_RunEmbedded() threw or was never invoked.
void DOSBOX_ShutdownEmbedded(void);

#ifdef __cplusplus
}
#endif

#endif // DOSBOX_EMBEDDED_H
