/*
  Boots the ROM path it is given, checks that a frame comes back, then
  autostarts a tiny program and checks the entry breakpoint stops in RAM.

  Exit 77 when no ROM was given and none of the local images exist.
*/

#include "pist_libretro_abi.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int pist_hatari_pc(void);

static int file_exists(const char *path)
{
	return path && path[0] && access(path, R_OK) == 0;
}

static const char *find_rom(int argc, char **argv)
{
	static const char *candidates[] = {
		"/usr/share/emutos/etos1024k.img",
		"/home/ryan/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/linux/hatari/etos512us.img",
		"/home/ryan/Code/AtariST/isaac/dev/roms/tos.img",
	};
	const char *env;
	size_t i;

	if (argc > 1 && file_exists(argv[1]))
		return argv[1];
	env = getenv("PIST_TOS");
	if (file_exists(env))
		return env;
	for (i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
		if (file_exists(candidates[i]))
			return candidates[i];
	}
	return NULL;
}

static int start_session(const char *tos, const char *program)
{
	PistHatariSession session;
	char err[512];

	memset(&session, 0, sizeof(session));
	session.abi = PIST_HATARI_ABI;
	session.tosPath = tos;
	session.machine = "st";
	session.memSizeMiB = 1;
	session.programPath = program;
	err[0] = '\0';
	if (pist_hatari_start(&session, err, (int)sizeof(err)) != 0) {
		fprintf(stderr, "start failed: %s\n", err);
		return 1;
	}
	return 0;
}

static int frame_has_ink(const PistHatariFrame *frame)
{
	const uint32_t *pixels;
	int n, i;

	if (!frame->pixels || frame->width <= 0 || frame->height <= 0)
		return 0;
	pixels = (const uint32_t *)frame->pixels;
	n = frame->width * frame->height;
	for (i = 0; i < n; i++) {
		if ((pixels[i] & 0x00FFFFFFu) != 0)
			return 1;
	}
	return 0;
}

static int write_prg(const char *path)
{
	unsigned char prg[0x1c + 4 + 4];
	FILE *out;

	memset(prg, 0, sizeof(prg));
	prg[0] = 0x60;
	prg[1] = 0x1a;
	prg[5] = 4; /* text length */
	prg[0x1c] = 0x4e;
	prg[0x1d] = 0x71; /* nop */
	prg[0x1e] = 0x60;
	prg[0x1f] = 0xfe; /* bra.s *-2 */
	out = fopen(path, "wb");
	if (!out)
		return 1;
	if (fwrite(prg, 1, sizeof(prg), out) != sizeof(prg)) {
		fclose(out);
		return 1;
	}
	fclose(out);
	return 0;
}

int main(int argc, char **argv)
{
	const char *rom = find_rom(argc, argv);
	PistHatariFrame frame;
	int stopped = 0;
	int i;
	int ink = 0;
	char dir[] = "/tmp/pist-frame-XXXXXX";
	char prg[512];
	uint32_t pc;

	if (!rom) {
		fprintf(stderr, "no TOS image (pass a path or set PIST_TOS); skipping\n");
		return 77;
	}
	printf("ROM %s\n", rom);

	if (start_session(rom, NULL) != 0)
		return 1;
	for (i = 0; i < 300 && !ink; i++) {
		memset(&frame, 0, sizeof(frame));
		if (pist_hatari_run(&frame, &stopped) != 0) {
			fprintf(stderr, "run failed on frame %d\n", i);
			pist_hatari_stop();
			return 1;
		}
		if (frame.width <= 0 || frame.height <= 0 || !frame.pixels) {
			fprintf(stderr, "frame %d is empty (%dx%d)\n", i, frame.width, frame.height);
			pist_hatari_stop();
			return 1;
		}
		ink = frame_has_ink(&frame);
	}
	printf("frame %dx%d pitch %d after %d vb, ink %d, stopped %d, pc %08x\n",
	       frame.width, frame.height, frame.pitch, i, ink, stopped, pist_hatari_pc());
	pist_hatari_stop();
	if (!ink) {
		fprintf(stderr, "300 frames stayed black\n");
		return 1;
	}

	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	snprintf(prg, sizeof(prg), "%s/HELLO.PRG", dir);
	if (write_prg(prg) != 0) {
		fprintf(stderr, "could not write %s\n", prg);
		return 1;
	}
	if (start_session(rom, prg) != 0)
		return 1;
	stopped = 0;
	for (i = 0; i < 4000 && !stopped; i++) {
		memset(&frame, 0, sizeof(frame));
		if (pist_hatari_run(&frame, &stopped) != 0) {
			fprintf(stderr, "run failed while waiting for entry\n");
			pist_hatari_stop();
			return 1;
		}
	}
	pc = pist_hatari_pc();
	printf("entry after %d vb, stopped %d, pc %08x\n", i, stopped, pc);
	pist_hatari_stop();
	if (!stopped || pc >= 0xe00000u) {
		fprintf(stderr, "entry breakpoint did not stop in RAM\n");
		return 1;
	}
	return 0;
}
