/*
  Boots the ROM path it is given, checks that a frame comes back, then
  autostarts a tiny program and checks the entry breakpoint stops in RAM.

  Exit 77 when no ROM was given and none of the local images exist.
*/

#include "pist_libretro_abi.h"

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ikbd.h"

uint32_t pist_hatari_pc(void);

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

static int start_session(const char *tos, const char *program, const char *monitor)
{
	PistHatariSession session;
	char err[512];

	memset(&session, 0, sizeof(session));
	session.abi = PIST_HATARI_ABI;
	session.tosPath = tos;
	session.machine = "st";
	session.memSizeMiB = 1;
	session.monitor = monitor;
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

static int frame_has_colour(const PistHatariFrame *frame)
{
	const uint32_t *pixels;
	int n, i;

	if (!frame->pixels || frame->width <= 0 || frame->height <= 0)
		return 0;
	pixels = (const uint32_t *)frame->pixels;
	n = frame->width * frame->height;
	for (i = 0; i < n; i++) {
		const uint32_t p = pixels[i] & 0x00FFFFFFu;
		const uint32_t r = (p >> 16) & 0xffu;
		const uint32_t g = (p >> 8) & 0xffu;
		const uint32_t b = p & 0xffu;
		if (r != g || g != b)
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

	if (pist_hatari_key(SDLK_a, 0, 1) == 0) {
		fprintf(stderr, "a key was accepted before the machine was up\n");
		return 1;
	}
	if (pist_hatari_mouse(1, 1, 1) == 0) {
		fprintf(stderr, "the mouse was accepted before the machine was up\n");
		return 1;
	}
	{
		int16_t buf[16 * 2];
		if (pist_hatari_audio(buf, 16) != 0) {
			fprintf(stderr, "audio returned samples before the machine was up\n");
			return 1;
		}
	}
	{
		char out[64];
		if (pist_hatari_command("info video", out, (int)sizeof(out), NULL) == 0) {
			fprintf(stderr, "a debugger line was accepted before the machine was up\n");
			return 1;
		}
	}

	{
		PistHatariSession session;
		char err[512];

		memset(&session, 0, sizeof(session));
		session.abi = PIST_HATARI_ABI;
		session.tosPath = rom;
		session.machine = "st";
		session.memSizeMiB = 1;
		session.monitor = "nope";
		err[0] = '\0';
		if (pist_hatari_start(&session, err, (int)sizeof(err)) == 0) {
			fprintf(stderr, "an unknown monitor was accepted\n");
			pist_hatari_stop();
			return 1;
		}
	}

	if (start_session(rom, NULL, NULL) != 0)
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
	{
		int16_t buf[4096 * 2];
		int n = pist_hatari_audio(buf, 4096);
		if (n <= 0) {
			fprintf(stderr, "desktop produced no audio\n");
			pist_hatari_stop();
			return 1;
		}
		printf("audio frames %d\n", n);
	}
	if (!ink) {
		fprintf(stderr, "300 frames stayed black\n");
		pist_hatari_stop();
		return 1;
	}
	{
		const int mono_w = frame.width;
		const int mono_h = frame.height;
		int colour = 0;

		pist_hatari_stop();
		if (start_session(rom, NULL, "rgb") != 0)
			return 1;
		for (i = 0; i < 300 && !colour; i++) {
			memset(&frame, 0, sizeof(frame));
			stopped = 0;
			if (pist_hatari_run(&frame, &stopped) != 0) {
				fprintf(stderr, "rgb run failed on frame %d\n", i);
				pist_hatari_stop();
				return 1;
			}
			colour = frame_has_colour(&frame);
		}
		printf("rgb frame %dx%d after %d vb, colour %d\n",
		       frame.width, frame.height, i, colour);
		pist_hatari_stop();
		if (!colour) {
			fprintf(stderr, "rgb monitor produced no colour pixel\n");
			return 1;
		}
		if (frame.width == mono_w && frame.height == mono_h) {
			fprintf(stderr, "rgb frame is the same size as mono (%dx%d)\n",
			        mono_w, mono_h);
			return 1;
		}
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
	if (start_session(rom, prg, NULL) != 0)
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
	if (!stopped || pc >= 0xe00000u) {
		fprintf(stderr, "entry breakpoint did not stop in RAM\n");
		pist_hatari_stop();
		return 1;
	}

	/* 'a' is ST scancode 0x1e. F12 is a Hatari shortcut unless that table
	 * was cleared; the ST key is Undo, scancode 0x61. */
	if (pist_hatari_key(SDLK_a, 0, 1) != 0 || !Keyboard.KeyStates[0x1e]) {
		fprintf(stderr, "key down did not press ST scancode 0x1e\n");
		pist_hatari_stop();
		return 1;
	}
	if (pist_hatari_key(SDLK_a, 0, 0) != 0 || Keyboard.KeyStates[0x1e]) {
		fprintf(stderr, "key up did not release ST scancode 0x1e\n");
		pist_hatari_stop();
		return 1;
	}
	if (pist_hatari_key(SDLK_F12, 0, 1) != 0 || !Keyboard.KeyStates[0x61]) {
		fprintf(stderr, "F12 did not reach the ST\n");
		pist_hatari_stop();
		return 1;
	}
	pist_hatari_key(SDLK_F12, 0, 0);

	/* Motion is in ST pixels and accumulates until the next VBL. Buttons are
	 * the current state, not an edge. */
	{
		const int dx = KeyboardProcessor.Mouse.dx;
		const int dy = KeyboardProcessor.Mouse.dy;
		if (pist_hatari_mouse(4, -3, 1) != 0 ||
		    KeyboardProcessor.Mouse.dx != dx + 4 ||
		    KeyboardProcessor.Mouse.dy != dy - 3 ||
		    (Keyboard.bLButtonDown & BUTTON_MOUSE) == 0) {
			fprintf(stderr, "mouse down did not move or press\n");
			pist_hatari_stop();
			return 1;
		}
		if (pist_hatari_mouse(0, 0, 2) != 0 ||
		    (Keyboard.bLButtonDown & BUTTON_MOUSE) != 0 ||
		    (Keyboard.bRButtonDown & BUTTON_MOUSE) == 0) {
			fprintf(stderr, "mouse buttons did not follow the host\n");
			pist_hatari_stop();
			return 1;
		}
		pist_hatari_mouse(0, 0, 0);
	}

	/* The entry condition is `pc = TEXT`, so the base and the PC agree.
	 * A short register buffer must report how many were needed and write
	 * nothing. */
	{
		uint32_t text = 0, data = 0, bss = 0;
		const char *names[32];
		uint32_t values[32];
		int needed = -1;
		int n, r;
		uint32_t reg_pc = 0xffffffffu;

		if (pist_hatari_registers(NULL, NULL, 0, &needed) != -1 || needed < 20) {
			fprintf(stderr, "registers did not report a short buffer (needed %d)\n", needed);
			pist_hatari_stop();
			return 1;
		}
		/* The break is noticed as the matching instruction finishes, so the
		 * PC we observe is the next one. This program is nop; bra.s to
		 * itself, and `pc = TEXT` matches the nop, leaving the bra. */
		if (pist_hatari_basepage(&text, &data, &bss) != 0 || pc != text + 2) {
			fprintf(stderr, "basepage text %08x, pc %08x\n", text, pc);
			pist_hatari_stop();
			return 1;
		}
		n = pist_hatari_registers(names, values, 32, &needed);
		if (n < 20) {
			fprintf(stderr, "registers returned %d\n", n);
			pist_hatari_stop();
			return 1;
		}
		for (r = 0; r < n; r++) {
			if (names[r] && strcmp(names[r], "PC") == 0)
				reg_pc = values[r];
		}
		if (reg_pc != pc) {
			fprintf(stderr, "register PC %08x, cpu PC %08x\n", reg_pc, pc);
			pist_hatari_stop();
			return 1;
		}

		/* info, disassembly and history are the panes. An unknown line
		 * prints an error and stays stopped. `c` is checked after pause,
		 * because it leaves the debugger. */
		{
			char out[65536];
			int cmdNeeded = -1;
			int rc;

			if (pist_hatari_command("", out, (int)sizeof(out), &cmdNeeded) == 0) {
				fprintf(stderr, "an empty debugger line was accepted\n");
				pist_hatari_stop();
				return 1;
			}
			cmdNeeded = -1;
			rc = pist_hatari_command("info video", out, (int)sizeof(out), &cmdNeeded);
			if (rc != 0 || cmdNeeded < 1 || !strstr(out, "Video")) {
				fprintf(stderr, "info video returned %d (%d bytes): %s\n",
				        rc, cmdNeeded, out);
				pist_hatari_stop();
				return 1;
			}
			cmdNeeded = -1;
			rc = pist_hatari_command("d", out, (int)sizeof(out), &cmdNeeded);
			if (rc != 0 || !strstr(out, "bra")) {
				fprintf(stderr, "disassembly returned %d: %s\n", rc, out);
				pist_hatari_stop();
				return 1;
			}
			cmdNeeded = -1;
			rc = pist_hatari_command("history 4", out, (int)sizeof(out), &cmdNeeded);
			if (rc != 0 || strstr(out, "No history")) {
				fprintf(stderr, "history returned %d: %s\n", rc, out);
				pist_hatari_stop();
				return 1;
			}
			cmdNeeded = -1;
			rc = pist_hatari_command("thisisnotacommand", out, (int)sizeof(out), &cmdNeeded);
			if (rc != 0 || !strstr(out, "not found")) {
				fprintf(stderr, "unknown command returned %d: %s\n", rc, out);
				pist_hatari_stop();
				return 1;
			}
			if (pist_hatari_pc() != pc) {
				fprintf(stderr, "a query moved the PC to %08x\n", pist_hatari_pc());
				pist_hatari_stop();
				return 1;
			}
		}

		/* Collect across one step of the bra, then the same disassembler
		 * switch the IDE does before a save. The switch must leave the
		 * counts in place, and the save file must hold the instruction
		 * rather than only the header. */
		{
			char path[512];
			char cmd[576];
			char out[4096];
			char got[65536];
			FILE *pf;
			size_t nread;

			snprintf(path, sizeof(path), "%s/profile.txt", dir);
			if (pist_hatari_command("profile on", out, (int)sizeof(out), NULL) != 0
			    || pist_hatari_step() != 0) {
				fprintf(stderr, "profile on or its step failed: %s\n", out);
				pist_hatari_stop();
				return 1;
			}
			pist_hatari_command("setopt --disasm ext", out, (int)sizeof(out), NULL);
			if (pist_hatari_command("setopt --disasm uae", out, (int)sizeof(out), NULL) != 0) {
				fprintf(stderr, "could not select the uae disassembler: %s\n", out);
				pist_hatari_stop();
				return 1;
			}
			snprintf(cmd, sizeof(cmd), "profile save %s", path);
			if (pist_hatari_command(cmd, out, (int)sizeof(out), NULL) != 0) {
				fprintf(stderr, "profile save failed: %s\n", out);
				pist_hatari_stop();
				return 1;
			}
			pf = fopen(path, "r");
			if (!pf) {
				fprintf(stderr, "profile file missing: %s\n", path);
				pist_hatari_stop();
				return 1;
			}
			nread = fread(got, 1, sizeof(got) - 1, pf);
			got[nread] = '\0';
			fclose(pf);
			if (!strstr(got, "Hatari CPU profile") || !strstr(got, "%")) {
				fprintf(stderr, "profile file has no instruction counts:\n%s\n", got);
				pist_hatari_stop();
				return 1;
			}
			if (pist_hatari_command("profile off", out, (int)sizeof(out), NULL) != 0
			    || pist_hatari_pc() != text + 2) {
				fprintf(stderr, "profile off moved the PC to %08x\n", pist_hatari_pc());
				pist_hatari_stop();
				return 1;
			}
		}

		/* The bra branches to itself, so a step and a step-over both
		 * stay on it. step-over is not a subroutine here; it is the
		 * one-instruction path of `n`. */
		if (pist_hatari_step_over() != 0 || pist_hatari_pc() != text + 2) {
			fprintf(stderr, "step over bra.s landed at %08x, wanted %08x\n",
			        pist_hatari_pc(), text + 2);
			pist_hatari_stop();
			return 1;
		}
		if (pist_hatari_step() != 0 || pist_hatari_pc() != text + 2) {
			fprintf(stderr, "step of bra.s landed at %08x\n", pist_hatari_pc());
			pist_hatari_stop();
			return 1;
		}

		/* Armed at the bra, resume stops there again. Cleared, the loop runs. */
		{
			char condition[64];
			snprintf(condition, sizeof(condition), "b pc = $%x", text + 2);
			if (pist_hatari_clear_breakpoints() != 0 ||
			    pist_hatari_arm_breakpoint(condition) != 0 ||
			    pist_hatari_resume() != 0) {
				fprintf(stderr, "could not arm %s\n", condition);
				pist_hatari_stop();
				return 1;
			}
		}
		stopped = 0;
		for (i = 0; i < 5 && !stopped; i++) {
			memset(&frame, 0, sizeof(frame));
			if (pist_hatari_run(&frame, &stopped) != 0) {
				fprintf(stderr, "run failed after arming\n");
				pist_hatari_stop();
				return 1;
			}
		}
		if (!stopped || pist_hatari_pc() != text + 2) {
			fprintf(stderr, "armed breakpoint did not stop at the bra (stopped %d pc %08x)\n",
			        stopped, pist_hatari_pc());
			pist_hatari_stop();
			return 1;
		}

		if (pist_hatari_clear_breakpoints() != 0 || pist_hatari_resume() != 0) {
			fprintf(stderr, "could not resume the cleared loop\n");
			pist_hatari_stop();
			return 1;
		}
		stopped = 1;
		for (i = 0; i < 3; i++) {
			memset(&frame, 0, sizeof(frame));
			if (pist_hatari_run(&frame, &stopped) != 0 || stopped) {
				fprintf(stderr, "cleared breakpoints still stopped the loop\n");
				pist_hatari_stop();
				return 1;
			}
		}
		/* One running frame, after the ring has been emptied, is one
		 * VBL of samples and then nothing until the next frame. */
		{
			int16_t buf[4096 * 2];
			int got;
			int guard;
			for (guard = 0; guard < 8; guard++) {
				if (pist_hatari_audio(buf, 4096) <= 0)
					break;
			}
			if (guard == 8) {
				fprintf(stderr, "audio ring did not drain\n");
				pist_hatari_stop();
				return 1;
			}
			stopped = 0;
			memset(&frame, 0, sizeof(frame));
			if (pist_hatari_run(&frame, &stopped) != 0 || stopped) {
				fprintf(stderr, "run failed while checking audio\n");
				pist_hatari_stop();
				return 1;
			}
			got = pist_hatari_audio(buf, 4096);
			if (got < 100 || got > 4000) {
				fprintf(stderr, "one frame produced %d audio frames\n", got);
				pist_hatari_stop();
				return 1;
			}
			if (pist_hatari_audio(buf, 4096) != 0) {
				fprintf(stderr, "audio was not empty after one pull\n");
				pist_hatari_stop();
				return 1;
			}
		}

		/* Host motion is delivered by the IKBD autosend interrupt.
		 * Ending a frame sets the quit flag, and that interrupt used
		 * to treat the flag as a real quit and never arm itself again.
		 * A key press makes the two coincide. The pointer must still
		 * move on the frames after the key. */
		{
			int sweep;
			int streak = 0;
			int worst = 0;
			int consumed_after_key = 0;
			for (sweep = 0; sweep < 40; sweep++) {
				if (pist_hatari_mouse(2, 0, 0) != 0) {
					fprintf(stderr, "mouse rejected while running\n");
					pist_hatari_stop();
					return 1;
				}
				if (sweep == 10) {
					pist_hatari_key(SDLK_a, 0, 1);
					pist_hatari_key(SDLK_a, 0, 0);
				}
				stopped = 0;
				memset(&frame, 0, sizeof(frame));
				if (pist_hatari_run(&frame, &stopped) != 0 || stopped) {
					fprintf(stderr, "run stopped during the mouse sweep\n");
					pist_hatari_stop();
					return 1;
				}
				/* The autosend is about one frame. A single frame can
				 * end before it runs; a dead interrupt leaves every
				 * later frame unconsumed. */
				if (KeyboardProcessor.Mouse.dx != 0) {
					if (++streak > worst)
						worst = streak;
				} else {
					streak = 0;
					if (sweep > 10)
						consumed_after_key++;
				}
			}
			if (worst > 3 || consumed_after_key == 0) {
				fprintf(stderr, "mouse stopped after the key (streak %d, consumed after %d)\n",
				        worst, consumed_after_key);
				pist_hatari_stop();
				return 1;
			}
		}

		if (pist_hatari_pause() != 0) {
			fprintf(stderr, "pause failed\n");
			pist_hatari_stop();
			return 1;
		}
		stopped = 0;
		for (i = 0; i < 5 && !stopped; i++) {
			memset(&frame, 0, sizeof(frame));
			if (pist_hatari_run(&frame, &stopped) != 0) {
				fprintf(stderr, "run failed after pause\n");
				pist_hatari_stop();
				return 1;
			}
		}
		if (!stopped) {
			fprintf(stderr, "pause did not stop the core\n");
			pist_hatari_stop();
			return 1;
		}
		{
			char out[4096];
			int cmdNeeded = -1;
			if (pist_hatari_command("c", out, (int)sizeof(out), &cmdNeeded) != 2) {
				fprintf(stderr, "cont did not leave the debugger: %s\n", out);
				pist_hatari_stop();
				return 1;
			}
			stopped = 1;
			memset(&frame, 0, sizeof(frame));
			if (pist_hatari_run(&frame, &stopped) != 0 || stopped) {
				fprintf(stderr, "cont did not resume\n");
				pist_hatari_stop();
				return 1;
			}
		}
		printf("keys, step, registers, breakpoint, audio, debugger and pause ok, pc %08x\n", pist_hatari_pc());
	}

	pist_hatari_stop();
	return 0;
}
