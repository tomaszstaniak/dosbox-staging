// SPDX-FileCopyrightText:  2026 The Boxer NG Authors
// SPDX-License-Identifier: GPL-2.0-or-later

// Embedded entry point.
//
// Boxer runs the emulator inside its own process on a secondary thread rather
// than as a standalone executable. main() is unsuitable for that: it installs
// an atexit() handler and assumes it owns process teardown.
//
// DOSBOX_RunEmbedded() is main()'s body with those assumptions removed. The
// host is responsible for calling DOSBOX_ShutdownEmbedded() afterwards.

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
