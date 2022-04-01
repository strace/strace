/*
 * Strauss awareness implementation.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "strauss.h"

static const char *strauss[] = {
	"",
	"     ____",
	"    /    \\",
	"   |-. .-.|",
	"   (_@)(_@)",
	"   .---_  \\",
	"  /..   \\_/",
	"  |__.-^ /",
	"      }  |",
	"     |   [",
	"     [  ]",
	"    ]   |",
	"    |   [",
	"    [  ]",
	"   /   |        __",
	"  \\|   |/     _/ /_",
	" \\ |   |//___/__/__/_",
	"\\\\  \\ /  //    -____/_",
	"//   \"   \\\\      \\___.-",
	" //     \\\\  __.----._/_",
	"/ '/|||\\` .-         __>",
	"[        /         __.-",
	"[        [           }",
	"\\        \\          /",
	" \"-._____ \\.____.--\"",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    {  } {  }",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    /  { |  |",
	" .-\"   / [   -._",
	"/___/ /   \\ \\___\"-.",
	"    -\"     \"-",
	};

const size_t strauss_lines = ARRAY_SIZE(strauss);

void
print_strauss(size_t verbosity)
{
	if (verbosity < STRAUSS_START_VERBOSITY)
		return;

	verbosity = MIN(verbosity - STRAUSS_START_VERBOSITY, strauss_lines);

	for (size_t i = 0; i < verbosity; i++)
		puts(strauss[i]);
}
