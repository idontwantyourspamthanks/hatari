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
#include "audio.h"
#include "breakcond.h"
#include "configuration.h"
#include "debugcpu.h"
#include "debugInfo.h"
#include "debugui.h"
#include "ikbd.h"
#include "keymap.h"
#include "m68000.h"
#include "newcpu.h"
#include "screen.h"
#include "stMemory.h"
#include "tos.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

static int sUp;
static int sStarted;
static int sStopped;

/* M68000_Start arms a CPU reset. Hatari calls it once; a later call would
 * reboot on every frame. m68k_go continues the machine that is already up. */
extern void m68k_go(int may_quit);
static uint8_t *sFrame;
static size_t sFrameCap;

/* Hatari calls this instead of reading the debugger console. The CPU loop
 * then returns to pist_hatari_run. */
static bool on_debugger(void)
{
	sStopped = 1;
	bQuitProgram = true;
	M68000_SetSpecial(SPCFLAG_BRK);
	return true;
}

static void copy_frame(PistHatariFrame *frame)
{
	const uint8_t *src;
	int x, y, w, h, pitch;
	size_t bytes;

	frame->pixels = NULL;
	frame->width = 0;
	frame->height = 0;
	frame->pitch = 0;
	if (!sdlscrn || !sdlscrn->pixels || sdlscrn->format->BytesPerPixel != 4)
		return;

	w = sdlscrn->w;
	h = sdlscrn->h;
	if (w <= 0 || h <= 0)
		return;
	pitch = w * 4;
	bytes = (size_t)pitch * (size_t)h;
	if (bytes > sFrameCap) {
		uint8_t *grown = realloc(sFrame, bytes);
		if (!grown)
			return;
		sFrame = grown;
		sFrameCap = bytes;
	}
	src = sdlscrn->pixels;
	for (y = 0; y < h; y++) {
		const uint32_t *in = (const uint32_t *)(src + (size_t)y * (size_t)sdlscrn->pitch);
		uint32_t *out = (uint32_t *)(sFrame + (size_t)y * (size_t)pitch);
		for (x = 0; x < w; x++)
			out[x] = in[x] | 0xFF000000u;
	}
	frame->pixels = sFrame;
	frame->width = w;
	frame->height = h;
	frame->pitch = pitch;
}

static const char *monitor_arg(const char *monitor)
{
	if (!monitor || !monitor[0])
		return "mono";
	if (strcmp(monitor, "mono") == 0 || strcmp(monitor, "rgb") == 0
	    || strcmp(monitor, "vga") == 0 || strcmp(monitor, "tv") == 0)
		return monitor;
	return NULL;
}

static void set_error(char *err, int errCap, const char *text)
{
	if (err && errCap > 0)
		snprintf(err, (size_t)errCap, "%s", text ? text : "");
}

int pist_hatari_tos_version(void);
uint32_t pist_hatari_pc(void);

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
	const char *monitor;
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
	monitor = monitor_arg(session->monitor);
	if (!monitor) {
		set_error(err, errCap,
		          "Unknown monitor. Expected mono, rgb, vga or tv.");
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
	argv[argc++] = monitor;
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

	/* PiST owns the shortcuts. Hatari's defaults swallow F11, F12 and
	 * Pause, which the ST keyboard still has. */
	{
		int i;
		for (i = 0; i < SHORTCUT_KEYS; i++) {
			ConfigureParams.Shortcut.withModifier[i] = SDLK_UNKNOWN;
			ConfigureParams.Shortcut.withoutModifier[i] = SDLK_UNKNOWN;
		}
	}

	/* Replaces the HRDB break loop Main_Init registered. A breakpoint
	 * returns here instead of waiting on TCP or stdin. */
	DebugUI_RegisterRemoteDebug(on_debugger);
	/* A previous session in this process leaves its breakpoints behind.
	 * The same entry stop the subprocess bootstrap script arms. TEXT is a
	 * Hatari variable, so this matches the program base once GEMDOS loads
	 * it and does not match ROM. */
	BreakCond_Command("all", false);
	if (!DebugUI_ParseLine("b pc = TEXT && pc < $e00000 :once")) {
		Main_LibretroShutdown();
		set_error(err, errCap, "Could not arm the entry breakpoint");
		return 1;
	}

	sStopped = 0;
	sStarted = 0;
	sUp = 1;
	/* The subprocess bootstrap script arms this before the program runs.
	 * Without it the history pane has nothing to show. */
	if (pist_hatari_command("history cpu", NULL, 0, NULL) != 0) {
		pist_hatari_stop();
		set_error(err, errCap, "Could not enable CPU history");
		return 1;
	}
	Main_UnPauseEmulation();
	return 0;
}

void pist_hatari_stop(void)
{
	if (!sUp)
		return;
	Main_LibretroShutdown();
	sUp = 0;
	sStarted = 0;
	sStopped = 0;
	free(sFrame);
	sFrame = NULL;
	sFrameCap = 0;
}

uint32_t pist_hatari_pc(void)
{
	return sUp ? M68000_GetPC() : 0;
}

int pist_hatari_run(PistHatariFrame *frame, int *stopped)
{
	if (!sUp)
		return 1;

	/* One frame, or the debugger, whichever comes first. A later call
	 * while stopped does not resume; that is pist_hatari_resume. */
	if (!sStopped) {
		bQuitProgram = false;
		Main_SetRunVBLs(1);
		if (!sStarted) {
			M68000_Start();
			sStarted = 1;
		} else {
			m68k_go(1);
		}
		bQuitProgram = false;
	}

	if (frame)
		copy_frame(frame);
	if (stopped)
		*stopped = sStopped;
	return 0;
}

/* One step, or a step-over that planted a one-shot, runs until the debugger
 * stops us. The frame loop's one-VBL cap must not apply: a step-over of a
 * subroutine would otherwise return at the next blank. */
static int run_until_debugger(void)
{
	sStopped = 0;
	bQuitProgram = false;
	Main_SetRunVBLs(0x7fffffff);
	m68k_go(1);
	bQuitProgram = false;
	return sStopped ? 0 : 1;
}

static const char *const kRegNames[] = {
	"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
	"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
	"PC", "USP", "ISP", "SR"
};
enum { kRegCount = sizeof(kRegNames) / sizeof(kRegNames[0]) };

int pist_hatari_registers(const char **names, uint32_t *values, int valuesCap, int *needed)
{
	int i;
	uint16_t sr;

	if (needed)
		*needed = kRegCount;
	if (!sUp || valuesCap < kRegCount || !names || !values)
		return -1;

	sr = M68000_GetSR();
	for (i = 0; i < 16; i++)
		values[i] = regs.regs[i];
	values[16] = M68000_GetPC();
	values[17] = regs.usp;
	values[18] = regs.isp;
	values[19] = sr;
	for (i = 0; i < kRegCount; i++)
		names[i] = kRegNames[i];
	return kRegCount;
}

int pist_hatari_basepage(uint32_t *textBase, uint32_t *dataBase, uint32_t *bssBase)
{
	uint32_t text;

	if (!sUp)
		return 1;
	text = DebugInfo_GetTEXT();
	if (textBase)
		*textBase = text;
	if (dataBase)
		*dataBase = DebugInfo_GetDATA();
	if (bssBase)
		*bssBase = DebugInfo_GetBSS();
	/* No program loaded yet: a zero text base is not a resolved address. */
	return text ? 0 : 1;
}

int pist_hatari_pause(void)
{
	if (!sUp)
		return 1;
	if (sStopped)
		return 0;
	/* The same always-true one-shot the subprocess arms on the control
	 * socket. The next run stops at the following instruction. */
	return DebugUI_ParseLine("b pc ! 0 :once") ? 0 : 1;
}

int pist_hatari_step(void)
{
	if (!sUp || !sStopped)
		return 1;
	DebugCpu_SetSteps(1);
	DebugCpu_SetDebugging();
	return run_until_debugger();
}

int pist_hatari_step_over(void)
{
	if (!sUp || !sStopped)
		return 1;
	/* "n" returns ENDCONT when it armed one step or a one-shot at the
	 * instruction after a call. ParseLine reports only CMDDONE as success,
	 * and the command path rewrites ENDCONT to END, so a false return here
	 * is the step that is ready to run. CMDDONE means it was rejected. */
	if (DebugUI_ParseLine("n"))
		return 1;
	return run_until_debugger();
}

int pist_hatari_resume(void)
{
	if (!sUp)
		return 1;
	sStopped = 0;
	DebugCpu_SetSteps(0);
	DebugCpu_SetDebugging();
	return 0;
}

int pist_hatari_arm_breakpoint(const char *condition)
{
	if (!sUp || !condition || !condition[0])
		return 1;
	return DebugUI_ParseLine(condition) ? 0 : 1;
}

int pist_hatari_mouse(int dx, int dy, int buttons)
{
	if (!sUp)
		return 1;
	/* Already in ST pixels. Hatari's own motion handler also divides by the
	 * SDL window scale; that scale is the panel's job, not this one's. */
	KeyboardProcessor.Mouse.dx += dx;
	KeyboardProcessor.Mouse.dy += dy;
	if (buttons & 1)
		Keyboard.bLButtonDown |= BUTTON_MOUSE;
	else
		Keyboard.bLButtonDown &= ~BUTTON_MOUSE;
	if (buttons & 2)
		Keyboard.bRButtonDown |= BUTTON_MOUSE;
	else
		Keyboard.bRButtonDown &= ~BUTTON_MOUSE;
	return 0;
}

/* Debugger text is fprintf(stderr). A pipe would deadlock once the command
 * filled it; a short-lived file does not. The window is this thread only.
 * On Windows the file is in the process temp directory and is removed when
 * its handle closes: a MinGW process launched outside an MSYS shell has no
 * /tmp, and an open file there cannot be unlinked the Unix way. */
static int redirect_stderr(int *saved, int *fd)
{
#ifdef _WIN32
	char dir[MAX_PATH];
	char path[MAX_PATH];
	HANDLE handle;

	if (!GetTempPathA(sizeof(dir), dir))
		return 1;
	if (!GetTempFileNameA(dir, "pdbg", 0, path))
		return 1;
	handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return 1;
	*fd = _open_osfhandle((intptr_t)handle, _O_RDWR | _O_BINARY);
	if (*fd < 0) {
		CloseHandle(handle);
		return 1;
	}
#else
	char path[] = "/tmp/pist-dbg-XXXXXX";

	*fd = mkstemp(path);
	if (*fd < 0)
		return 1;
	unlink(path);
#endif
	fflush(stderr);
	*saved = dup(STDERR_FILENO);
	if (*saved < 0) {
		close(*fd);
		*fd = -1;
		return 1;
	}
	if (dup2(*fd, STDERR_FILENO) < 0) {
		close(*saved);
		close(*fd);
		*saved = -1;
		*fd = -1;
		return 1;
	}
	return 0;
}

static void restore_stderr(int saved)
{
	fflush(stderr);
	if (saved >= 0) {
		dup2(saved, STDERR_FILENO);
		close(saved);
	}
}

static void copy_capture(int fd, char *out, int outCap, int *needed)
{
	off_t end;
	int n, got;

	end = lseek(fd, 0, SEEK_END);
	if (end < 0)
		end = 0;
	if (needed)
		*needed = end > 0x7fffffff ? 0x7fffffff : (int)end;
	if (!out || outCap <= 0)
		return;
	n = (int)end;
	if (n > outCap - 1)
		n = outCap - 1;
	if (n < 0)
		n = 0;
	if (n == 0 || lseek(fd, 0, SEEK_SET) < 0) {
		out[0] = '\0';
		return;
	}
	got = 0;
	while (got < n) {
		ssize_t r = read(fd, out + got, (size_t)(n - got));
		if (r <= 0)
			break;
		got += (int)r;
	}
	out[got] = '\0';
}

int pist_hatari_command(const char *line, char *out, int outCap, int *needed)
{
	int saved = -1;
	int fd = -1;
	int wasStopped;
	int leave;

	if (needed)
		*needed = 0;
	if (out && outCap > 0)
		out[0] = '\0';
	if (!sUp || !line || !line[0])
		return 1;
	if (redirect_stderr(&saved, &fd) != 0)
		return 1;

	wasStopped = sStopped;
	/* CMDCONT is a repeatable line such as `d`: it stays stopped. Only END
	 * leaves the debugger (continue, a step, a step-over). ParseLine already
	 * re-armed the per-instruction hook. The CPU is not started from here. */
	leave = DebugUI_ParseLineCode(line) == DEBUGGER_END;
	restore_stderr(saved);
	copy_capture(fd, out, outCap, needed);
	close(fd);

	if (leave && wasStopped) {
		sStopped = 0;
		return 2;
	}
	return 0;
}

int pist_hatari_audio(int16_t *interleaved, int frames)
{
	if (!sUp)
		return 0;
	return Audio_Read(interleaved, frames);
}

int pist_hatari_key(int sym, int mod, int down)
{
	SDL_Keysym key;

	if (!sUp)
		return 1;
	memset(&key, 0, sizeof(key));
	key.sym = sym;
	key.mod = (Uint16)mod;
	if (down)
		Keymap_KeyDown(&key);
	else
		Keymap_KeyUp(&key);
	return 0;
}

int pist_hatari_clear_breakpoints(void)
{
	if (!sUp)
		return 1;
	BreakCond_Command("all", false);
	DebugCpu_SetDebugging();
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
