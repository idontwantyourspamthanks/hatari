/* The functions behind src/pist_sdl/SDL.h. A window is an in-memory
 * surface. Nothing here opens a device. */
#include "pist_sdl/SDL.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static Uint32 s_inited;

static int mask_shift(Uint32 mask)
{
	int shift = 0;
	if (!mask)
		return 0;
	while ((mask & 1u) == 0) {
		mask >>= 1;
		shift++;
	}
	return shift;
}

static int mask_loss(Uint32 mask)
{
	int bits = 0;
	if (!mask)
		return 8;
	while ((mask & 1u) == 0)
		mask >>= 1;
	while (mask & 1u) {
		bits++;
		mask >>= 1;
	}
	return bits >= 8 ? 0 : 8 - bits;
}

static SDL_Surface *make_surface(int w, int h, int depth, Uint32 Rmask, Uint32 Gmask,
                                 Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *surface;
	SDL_PixelFormat *format;
	int bpp;

	if (w < 0 || h < 0)
		return NULL;
	bpp = depth / 8;
	if (bpp < 1)
		bpp = 1;
	surface = calloc(1, sizeof *surface);
	format = calloc(1, sizeof *format);
	if (!surface || !format) {
		free(surface);
		free(format);
		return NULL;
	}
	format->BitsPerPixel = (Uint8)depth;
	format->BytesPerPixel = (Uint8)bpp;
	format->Rmask = Rmask;
	format->Gmask = Gmask;
	format->Bmask = Bmask;
	format->Amask = Amask;
	format->Rshift = (Uint8)mask_shift(Rmask);
	format->Gshift = (Uint8)mask_shift(Gmask);
	format->Bshift = (Uint8)mask_shift(Bmask);
	format->Ashift = (Uint8)mask_shift(Amask);
	format->Rloss = (Uint8)mask_loss(Rmask);
	format->Gloss = (Uint8)mask_loss(Gmask);
	format->Bloss = (Uint8)mask_loss(Bmask);
	format->Aloss = (Uint8)mask_loss(Amask);
	format->refcount = 1;
	if (depth <= 8) {
		format->palette = calloc(1, sizeof *format->palette);
		if (format->palette) {
			format->palette->ncolors = 256;
			format->palette->colors = calloc(256, sizeof(SDL_Color));
			format->palette->refcount = 1;
		}
	}
	surface->format = format;
	surface->w = w;
	surface->h = h;
	surface->pitch = w * bpp;
	surface->pixels = calloc((size_t)h, (size_t)surface->pitch ? (size_t)surface->pitch : 1);
	surface->refcount = 1;
	if (!surface->pixels) {
		SDL_FreeSurface(surface);
		return NULL;
	}
	return surface;
}

struct SDL_Window {
	int w, h;
	Uint32 flags;
	SDL_Surface *surface;
};

void *SDL_malloc(size_t size) { return malloc(size); }
void SDL_free(void *ptr) { free(ptr); }

char *SDL_getenv(const char *name) { return getenv(name); }

size_t SDL_strlen(const char *s) { return s ? strlen(s) : 0; }

size_t SDL_strlcpy(char *dst, const char *src, size_t maxlen)
{
	size_t n;
	if (!dst || maxlen == 0)
		return src ? strlen(src) : 0;
	if (!src)
		src = "";
	n = strlen(src);
	if (n >= maxlen)
		n = maxlen - 1;
	memcpy(dst, src, n);
	dst[n] = '\0';
	return strlen(src);
}

Uint16 SDL_Swap16(Uint16 x) { return (Uint16)((x << 8) | (x >> 8)); }
Uint32 SDL_Swap32(Uint32 x)
{
	return (x << 24) | ((x << 8) & 0x00ff0000u) | ((x >> 8) & 0x0000ff00u) | (x >> 24);
}
Uint16 SDL_SwapBE16(Uint16 x)
{
	return SDL_BYTEORDER == SDL_BIG_ENDIAN ? x : SDL_Swap16(x);
}
Uint16 SDL_SwapLE16(Uint16 x)
{
	return SDL_BYTEORDER == SDL_LIL_ENDIAN ? x : SDL_Swap16(x);
}
Uint32 SDL_SwapLE32(Uint32 x)
{
	return SDL_BYTEORDER == SDL_LIL_ENDIAN ? x : SDL_Swap32(x);
}

int SDL_Init(Uint32 flags)
{
	s_inited |= flags;
	return 0;
}
int SDL_InitSubSystem(Uint32 flags) { return SDL_Init(flags); }
void SDL_Quit(void) { s_inited = 0; }
void SDL_QuitSubSystem(Uint32 flags) { s_inited &= ~flags; }
Uint32 SDL_WasInit(Uint32 flags) { return flags ? (s_inited & flags) : s_inited; }

const char *SDL_GetError(void) { return "pist sdl stub"; }

Uint32 SDL_GetTicks(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (Uint32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void SDL_Delay(Uint32 ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (long)(ms % 1000) * 1000000L;
	nanosleep(&ts, NULL);
}

int SDL_PollEvent(SDL_Event *event)
{
	(void)event;
	return 0;
}
void SDL_PumpEvents(void) {}
int SDL_PushEvent(SDL_Event *event)
{
	(void)event;
	return 0;
}
int SDL_WaitEvent(SDL_Event *event)
{
	(void)event;
	return 0;
}

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{
	SDL_Window *window;
	(void)title;
	(void)x;
	(void)y;
	window = calloc(1, sizeof *window);
	if (!window)
		return NULL;
	window->w = w;
	window->h = h;
	window->flags = flags;
	return window;
}
void SDL_DestroyWindow(SDL_Window *window)
{
	if (!window)
		return;
	SDL_FreeSurface(window->surface);
	free(window);
}
void SDL_SetWindowTitle(SDL_Window *window, const char *title)
{
	(void)window;
	(void)title;
}
void SDL_SetWindowSize(SDL_Window *window, int w, int h)
{
	if (!window)
		return;
	window->w = w;
	window->h = h;
	if (window->surface) {
		SDL_FreeSurface(window->surface);
		window->surface = NULL;
	}
}
void SDL_GetWindowSize(SDL_Window *window, int *w, int *h)
{
	if (w)
		*w = window ? window->w : 0;
	if (h)
		*h = window ? window->h : 0;
}
Uint32 SDL_GetWindowFlags(SDL_Window *window) { return window ? window->flags : 0; }
void SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon)
{
	(void)window;
	(void)icon;
}
void SDL_MinimizeWindow(SDL_Window *window) { (void)window; }

SDL_Surface *SDL_GetWindowSurface(SDL_Window *window)
{
	if (!window)
		return NULL;
	if (!window->surface) {
		window->surface = make_surface(window->w > 0 ? window->w : 640,
		                               window->h > 0 ? window->h : 400, 32,
		                               0x00ff0000u, 0x0000ff00u, 0x000000ffu, 0);
	}
	return window->surface;
}
int SDL_UpdateWindowSurfaceRects(SDL_Window *window, const SDL_Rect *rects, int numrects)
{
	(void)window;
	(void)rects;
	(void)numrects;
	return 0;
}
int SDL_GetWindowWMInfo(SDL_Window *window, SDL_SysWMinfo *info)
{
	(void)window;
	(void)info;
	return 0;
}
int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode)
{
	(void)displayIndex;
	if (!mode)
		return -1;
	mode->format = SDL_PIXELFORMAT_RGB888;
	mode->w = 640;
	mode->h = 400;
	mode->refresh_rate = 50;
	return 0;
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags)
{
	(void)window;
	(void)index;
	(void)flags;
	return calloc(1, 1);
}
void SDL_DestroyRenderer(SDL_Renderer *renderer) { free(renderer); }
int SDL_GetRendererInfo(SDL_Renderer *renderer, SDL_RendererInfo *info)
{
	(void)renderer;
	if (!info)
		return -1;
	memset(info, 0, sizeof *info);
	info->flags = SDL_RENDERER_SOFTWARE;
	return 0;
}
int SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h)
{
	(void)renderer;
	(void)w;
	(void)h;
	return 0;
}
int SDL_RenderSetScale(SDL_Renderer *renderer, float scaleX, float scaleY)
{
	(void)renderer;
	(void)scaleX;
	(void)scaleY;
	return 0;
}
int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	(void)renderer;
	(void)r;
	(void)g;
	(void)b;
	(void)a;
	return 0;
}
int SDL_RenderClear(SDL_Renderer *renderer)
{
	(void)renderer;
	return 0;
}
int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect)
{
	(void)renderer;
	(void)texture;
	(void)srcrect;
	(void)dstrect;
	return 0;
}
void SDL_RenderPresent(SDL_Renderer *renderer) { (void)renderer; }
SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h)
{
	(void)renderer;
	(void)format;
	(void)access;
	(void)w;
	(void)h;
	return calloc(1, 1);
}
void SDL_DestroyTexture(SDL_Texture *texture) { free(texture); }
int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
	(void)texture;
	(void)rect;
	(void)pixels;
	(void)pitch;
	return 0;
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	(void)flags;
	if (depth == 32 && Rmask == 0 && Gmask == 0 && Bmask == 0) {
		Rmask = 0x00ff0000u;
		Gmask = 0x0000ff00u;
		Bmask = 0x000000ffu;
	}
	return make_surface(width, height, depth, Rmask, Gmask, Bmask, Amask);
}

void SDL_FreeSurface(SDL_Surface *surface)
{
	if (!surface)
		return;
	if (surface->format) {
		if (surface->format->palette) {
			free(surface->format->palette->colors);
			free(surface->format->palette);
		}
		free(surface->format);
	}
	free(surface->pixels);
	free(surface);
}

int SDL_LockSurface(SDL_Surface *surface)
{
	(void)surface;
	return 0;
}
void SDL_UnlockSurface(SDL_Surface *surface) { (void)surface; }

int SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color)
{
	int x, y, x0, y0, x1, y1;
	if (!dst || !dst->pixels || !dst->format)
		return -1;
	x0 = rect ? rect->x : 0;
	y0 = rect ? rect->y : 0;
	x1 = rect ? rect->x + rect->w : dst->w;
	y1 = rect ? rect->y + rect->h : dst->h;
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 > dst->w)
		x1 = dst->w;
	if (y1 > dst->h)
		y1 = dst->h;
	for (y = y0; y < y1; y++) {
		Uint8 *row = (Uint8 *)dst->pixels + (size_t)y * (size_t)dst->pitch;
		for (x = x0; x < x1; x++) {
			if (dst->format->BytesPerPixel == 4)
				((Uint32 *)row)[x] = color;
			else if (dst->format->BytesPerPixel == 1)
				row[x] = (Uint8)color;
		}
	}
	return 0;
}

int SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	(void)src;
	(void)srcrect;
	(void)dst;
	(void)dstrect;
	return 0;
}

Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b)
{
	if (!format)
		return 0;
	return ((Uint32)(r >> format->Rloss) << format->Rshift) |
	       ((Uint32)(g >> format->Gloss) << format->Gshift) |
	       ((Uint32)(b >> format->Bloss) << format->Bshift);
}

int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
	(void)surface;
	(void)flag;
	(void)key;
	return 0;
}
int SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors, int firstcolor, int ncolors)
{
	int i;
	if (!palette || !palette->colors || !colors)
		return -1;
	for (i = 0; i < ncolors; i++) {
		int at = firstcolor + i;
		if (at < 0 || at >= palette->ncolors)
			break;
		palette->colors[at] = colors[i];
	}
	return 0;
}
SDL_Surface *SDL_LoadBMP(const char *file)
{
	(void)file;
	return NULL;
}
SDL_RWops *SDL_RWFromFile(const char *file, const char *mode)
{
	(void)file;
	(void)mode;
	return NULL;
}
int SDL_SaveBMP_RW(SDL_Surface *surface, SDL_RWops *dst, int freedst)
{
	(void)surface;
	(void)dst;
	(void)freedst;
	return -1;
}
int SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority)
{
	(void)name;
	(void)value;
	(void)priority;
	return 1;
}

int SDL_ShowCursor(int toggle)
{
	(void)toggle;
	return 0;
}
int SDL_SetRelativeMouseMode(SDL_bool enabled)
{
	(void)enabled;
	return 0;
}
SDL_bool SDL_GetRelativeMouseMode(void) { return SDL_FALSE; }
Uint32 SDL_GetMouseState(int *x, int *y)
{
	if (x)
		*x = 0;
	if (y)
		*y = 0;
	return 0;
}
void SDL_WarpMouseInWindow(SDL_Window *window, int x, int y)
{
	(void)window;
	(void)x;
	(void)y;
}
void SDL_WarpMouse(Uint16 x, Uint16 y)
{
	(void)x;
	(void)y;
}
void SDL_StartTextInput(void) {}
void SDL_StopTextInput(void) {}
void SDL_SetTextInputRect(const SDL_Rect *rect) { (void)rect; }

const Uint8 *SDL_GetKeyboardState(int *numkeys)
{
	static Uint8 keys[512];
	if (numkeys)
		*numkeys = (int)sizeof keys;
	return keys;
}
SDL_Keymod SDL_GetModState(void) { return KMOD_NONE; }
SDL_Keycode SDL_GetKeyFromName(const char *name)
{
	(void)name;
	return SDLK_UNKNOWN;
}
const char *SDL_GetKeyName(SDL_Keycode key)
{
	(void)key;
	return "";
}
SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key)
{
	(void)key;
	return SDL_SCANCODE_UNKNOWN;
}

int SDL_NumJoysticks(void) { return 0; }
SDL_Joystick *SDL_JoystickOpen(int device_index)
{
	(void)device_index;
	return NULL;
}
void SDL_JoystickClose(SDL_Joystick *joystick) { (void)joystick; }
const char *SDL_JoystickName(SDL_Joystick *joystick)
{
	(void)joystick;
	return "";
}
int SDL_JoystickNumAxes(SDL_Joystick *joystick)
{
	(void)joystick;
	return 0;
}
int SDL_JoystickNumButtons(SDL_Joystick *joystick)
{
	(void)joystick;
	return 0;
}
Sint16 SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis)
{
	(void)joystick;
	(void)axis;
	return 0;
}
Uint8 SDL_JoystickGetButton(SDL_Joystick *joystick, int button)
{
	(void)joystick;
	(void)button;
	return 0;
}
Uint8 SDL_JoystickGetHat(SDL_Joystick *joystick, int hat)
{
	(void)joystick;
	(void)hat;
	return 0;
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	/* Success, and the caller's spec stands. There is no device: PiST
	 * pulls the mix ring itself. A failure here makes Audio_Init turn
	 * sound off for the rest of the session. */
	if (obtained && desired)
		*obtained = *desired;
	return 0;
}
Uint32 SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec *desired,
                           SDL_AudioSpec *obtained, int allowed_changes)
{
	(void)device;
	(void)iscapture;
	(void)desired;
	(void)obtained;
	(void)allowed_changes;
	return 0;
}
void SDL_CloseAudio(void) {}
void SDL_CloseAudioDevice(Uint32 dev) { (void)dev; }
void SDL_PauseAudio(int pause_on) { (void)pause_on; }
void SDL_PauseAudioDevice(Uint32 dev, int pause_on)
{
	(void)dev;
	(void)pause_on;
}
void SDL_LockAudio(void) {}
void SDL_UnlockAudio(void) {}

/* Hatari's Windows build opens a console for the debugger. Inside PiST that
 * would be a second window; the debugger text is captured by pist_libretro.c. */
#ifdef _WIN32
void Win_OpenCon(void) {}
void Win_ForceCon(void) {}
#endif
