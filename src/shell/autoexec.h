// SPDX-FileCopyrightText:  2020-2025 The DOSBox Staging Team
// SPDX-FileCopyrightText:  2002-2021 The DOSBox Team
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DOSBOX_AUTOEXEC_H
#define DOSBOX_AUTOEXEC_H

#include "config/setup.h"

#include <string>
#include <vector>

// Creates or refreshes the AUTOEXEC.BAT file on the emulated Z: drive.
void AUTOEXEC_RefreshFile();

// Adds/updates environment variable to the AUTOEXEC.BAT file. Empty value
// removes the variable. If a shell is already running, it the environment is
// updated accordingly.
// The 'name' and 'value' have to be a printable, 7-bit ASCII variables.
void AUTOEXEC_SetVariable(const std::string& name, const std::string& value);

// Appends lines supplied by an embedding host to the end of AUTOEXEC.BAT
// (after the [autoexec] section content, before any '@EXIT') and regenerates
// the virtual file, including any variables recorded with
// AUTOEXEC_SetVariable() since the file was first generated. An empty list
// regenerates the file without adding lines.
//
// Only valid after AUTOEXEC_Init() and before the first shell exists: once
// COMMAND.COM is executing the file its offsets must not change. Returns
// false, without modifying anything, if called too early or too late.
bool AUTOEXEC_AppendHostLines(const std::vector<std::string>& lines);

void AUTOEXEC_Init();

#endif
