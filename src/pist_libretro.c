/*
  Hatari - pist_libretro.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  In-process entry point for PiST. The ROM is the path in the session: a TOS
  image the user supplied, or EmuTOS when that is the file the session
  resolved. This file does not look for a ROM of its own.
*/

const char PistLibretro_fileid[] = "Hatari pist_libretro.c";

#include "pist_libretro_abi.h"

#include "main.h"
#include "stMemory.h"
#include "tos.h"

#include <stdio.h>
#include <string.h>

static int sUp;

static void set_error(char *err, int errCap, const char *text)
{
	if (err && errCap > 0)
		snprintf(err, (size_t)errCap, "%s", text ? text : "");
}

int pist_hatari_tos_version(void);

int pist_hatari_abi(void)
{
	return PIST_HATARI_ABI;
}

int pist_hatari_tos_version(void)
{
	return sUp ? (int)TosVersion : 0;
}

int pist_hatari_start(const PistHatariSession *session, char *err, int errCap)
{
	const char *argv[24];
	char mem[16];
	int argc = 0;

	if (sUp)
		pist_hatari_stop();

	if (!session || session->abi != PIST_HATARI_ABI) {
		set_error(err, errCap, "pist_hatari_start: ABI mismatch");
		return 1;
	}
	if (!session->tosPath || !session->tosPath[0]) {
		set_error(err, errCap,
		          "No TOS ROM was given. The core does not contain one.");
		return 1;
	}

	argv[argc++] = "hatari_libretro";
	argv[argc++] = "--tos";
	argv[argc++] = session->tosPath;
	argv[argc++] = "--machine";
	argv[argc++] = (session->machine && session->machine[0]) ? session->machine : "st";
	if (session->memSizeMiB > 0) {
		snprintf(mem, sizeof(mem), "%d", session->memSizeMiB);
		argv[argc++] = "--memsize";
		argv[argc++] = mem;
	}
	argv[argc++] = "--monitor";
	argv[argc++] = "mono";
	argv[argc++] = "--sound";
	argv[argc++] = "off";
	argv[argc++] = "--alert-level";
	argv[argc++] = "fatal";
	argv[argc++] = "--confirm-quit";
	argv[argc++] = "off";
	argv[argc++] = "--fast-forward";
	argv[argc++] = "on";
	if (session->diskA && session->diskA[0]) {
		argv[argc++] = "--disk-a";
		argv[argc++] = session->diskA;
	}
	if (session->diskB && session->diskB[0]) {
		argv[argc++] = "--disk-b";
		argv[argc++] = session->diskB;
	}
	if (session->gemdosDir && session->gemdosDir[0]) {
		argv[argc++] = "-d";
		argv[argc++] = session->gemdosDir;
	}
	if (session->programPath && session->programPath[0])
		argv[argc++] = session->programPath;

	if (Main_LibretroBringUp(argc, argv, err, errCap) != 0)
		return 1;

	sUp = 1;
	return 0;
}

void pist_hatari_stop(void)
{
	if (!sUp)
		return;
	Main_LibretroShutdown();
	sUp = 0;
}

int pist_hatari_run(PistHatariFrame *frame, int *stopped)
{
	if (frame) {
		frame->pixels = NULL;
		frame->width = 0;
		frame->height = 0;
		frame->pitch = 0;
	}
	if (stopped)
		*stopped = sUp ? 1 : 0;
	if (!sUp) {
		return 1;
	}
	/* The machine is up and stopped. Running it to a frame is the next
	 * piece; the CPU loop still blocks inside m68k_go. */
	return 0;
}

void *pist_hatari_ram(size_t *size)
{
	if (!sUp || !STRam) {
		if (size)
			*size = 0;
		return NULL;
	}
	if (size)
		*size = (size_t)STRamEnd;
	return STRam;
}
