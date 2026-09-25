/* Stand-in for SDL2, used only by the PiST in-process core.
 *
 * The release dylib links libm and libz. This header is what the Hatari
 * sources include instead of the real SDL, so the window, the audio device
 * and the input grab never leave the process. A frame is the RGB surface
 * Screen_SetSDLVideoMode allocates; pist_libretro.c copies it out.
 *
 * Not a second SDL. The values only have to be distinct, because this build
 * never delivers a real host event.
 */
#ifndef PIST_SDL_STUB_H
#define PIST_SDL_STUB_H

#include <stddef.h>
#include <stdint.h>

/* Real SDL_platform.h defines this, and remotedebug.c uses it to choose
 * freopen over assigning stderr. MinGW's stderr is not an lvalue. */
#if (defined(_WIN32) || defined(__CYGWIN__)) && !defined(__WINDOWS__)
#define __WINDOWS__ 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef int32_t Sint32;
typedef uint64_t Uint64;
typedef int64_t Sint64;

typedef enum { SDL_FALSE = 0, SDL_TRUE = 1 } SDL_bool;

#define SDL_HINT_RENDER_VSYNC "SDL_RENDER_VSYNC"
#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_RENDER_SCALE_QUALITY"
#define SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4 "SDL_WINDOWS_NO_CLOSE_ON_ALT_F4"
#define SDL_HINT_VIDEO_X11_WMCLASS "SDL_VIDEO_X11_WMCLASS"
#define SDL_VIDEO_X11_WMCLASS SDL_HINT_VIDEO_X11_WMCLASS

typedef enum {
	SDL_HINT_DEFAULT,
	SDL_HINT_NORMAL,
	SDL_HINT_OVERRIDE
} SDL_HintPriority;

#define SDL_INIT_TIMER 0x00000001u
#define SDL_INIT_AUDIO 0x00000010u
#define SDL_INIT_VIDEO 0x00000020u
#define SDL_INIT_JOYSTICK 0x00000200u
#define SDL_INIT_EVENTS 0x00004000u

#define SDL_WINDOWPOS_UNDEFINED 0x1FFF0000u
#define SDL_WINDOW_FULLSCREEN 0x00000001u
#define SDL_WINDOW_OPENGL 0x00000002u
#define SDL_WINDOW_SHOWN 0x00000004u
#define SDL_WINDOW_HIDDEN 0x00000008u
#define SDL_WINDOW_BORDERLESS 0x00000010u
#define SDL_WINDOW_RESIZABLE 0x00000020u
#define SDL_WINDOW_MINIMIZED 0x00000040u
#define SDL_WINDOW_MAXIMIZED 0x00000080u
#define SDL_WINDOW_INPUT_GRABBED 0x00000100u
#define SDL_WINDOW_FULLSCREEN_DESKTOP (SDL_WINDOW_FULLSCREEN | 0x00001000u)

#define SDL_WINDOWEVENT_NONE 0
#define SDL_WINDOWEVENT_SHOWN 1
#define SDL_WINDOWEVENT_HIDDEN 2
#define SDL_WINDOWEVENT_EXPOSED 3
#define SDL_WINDOWEVENT_MOVED 4
#define SDL_WINDOWEVENT_RESIZED 5
#define SDL_WINDOWEVENT_SIZE_CHANGED 6
#define SDL_WINDOWEVENT_MINIMIZED 7
#define SDL_WINDOWEVENT_MAXIMIZED 8
#define SDL_WINDOWEVENT_RESTORED 9
#define SDL_WINDOWEVENT_ENTER 10
#define SDL_WINDOWEVENT_LEAVE 11
#define SDL_WINDOWEVENT_FOCUS_GAINED 12
#define SDL_WINDOWEVENT_FOCUS_LOST 13
#define SDL_WINDOWEVENT_CLOSE 14

#define SDL_RENDERER_SOFTWARE 0x00000001u
#define SDL_TEXTUREACCESS_STREAMING 1
#define SDL_PIXELFORMAT_RGB565 1
#define SDL_PIXELFORMAT_RGB888 2

#define SDL_SWSURFACE 0
#define SDL_RLEACCEL 0
#define SDL_PREALLOC 0x00000001u

#define SDL_QUERY -1
#define SDL_DISABLE 0
#define SDL_ENABLE 1

#define SDL_BUTTON(X) (1 << ((X) - 1))
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3

#define AUDIO_U8 0x0008
#define AUDIO_S16SYS 0x8010

#define KMOD_NONE 0x0000
#define KMOD_LSHIFT 0x0001
#define KMOD_RSHIFT 0x0002
#define KMOD_SHIFT (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_LCTRL 0x0040
#define KMOD_RCTRL 0x0080
#define KMOD_LALT 0x0100
#define KMOD_RALT 0x0200
#define KMOD_LGUI 0x0400
#define KMOD_RGUI 0x0800
#define KMOD_NUM 0x1000
#define KMOD_MODE 0x4000

#define SDL_HAT_UP 0x01
#define SDL_HAT_RIGHT 0x02
#define SDL_HAT_DOWN 0x04
#define SDL_HAT_LEFT 0x08

typedef enum {
	SDL_SCANCODE_UNKNOWN = 0,
	SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D,
	SDL_SCANCODE_E, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H,
	SDL_SCANCODE_I, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L,
	SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O, SDL_SCANCODE_P,
	SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T,
	SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X,
	SDL_SCANCODE_Y, SDL_SCANCODE_Z,
	SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
	SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8,
	SDL_SCANCODE_9, SDL_SCANCODE_0,
	SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE, SDL_SCANCODE_BACKSPACE,
	SDL_SCANCODE_TAB, SDL_SCANCODE_SPACE, SDL_SCANCODE_MINUS,
	SDL_SCANCODE_EQUALS, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET,
	SDL_SCANCODE_BACKSLASH, SDL_SCANCODE_NONUSHASH, SDL_SCANCODE_SEMICOLON,
	SDL_SCANCODE_APOSTROPHE, SDL_SCANCODE_GRAVE, SDL_SCANCODE_COMMA,
	SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH, SDL_SCANCODE_CAPSLOCK,
	SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4,
	SDL_SCANCODE_F5, SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8,
	SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12,
	SDL_SCANCODE_PRINTSCREEN, SDL_SCANCODE_SCROLLLOCK, SDL_SCANCODE_PAUSE,
	SDL_SCANCODE_INSERT, SDL_SCANCODE_HOME, SDL_SCANCODE_PAGEUP,
	SDL_SCANCODE_DELETE, SDL_SCANCODE_END, SDL_SCANCODE_PAGEDOWN,
	SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_DOWN, SDL_SCANCODE_UP,
	SDL_SCANCODE_NUMLOCKCLEAR, SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_MULTIPLY,
	SDL_SCANCODE_KP_MINUS, SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_KP_ENTER,
	SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_4,
	SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8,
	SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_PERIOD,
	SDL_SCANCODE_NONUSBACKSLASH, SDL_SCANCODE_APPLICATION, SDL_SCANCODE_KP_EQUALS,
	SDL_SCANCODE_F13, SDL_SCANCODE_F14, SDL_SCANCODE_HELP, SDL_SCANCODE_UNDO,
	SDL_SCANCODE_CLEAR, SDL_SCANCODE_RETURN2, SDL_SCANCODE_KP_COMMA,
	SDL_SCANCODE_KP_EQUALSAS400, SDL_SCANCODE_KP_LEFTPAREN, SDL_SCANCODE_KP_RIGHTPAREN,
	SDL_SCANCODE_KP_LEFTBRACE, SDL_SCANCODE_KP_RIGHTBRACE, SDL_SCANCODE_KP_TAB,
	SDL_SCANCODE_KP_BACKSPACE, SDL_SCANCODE_KP_COLON, SDL_SCANCODE_KP_HASH,
	SDL_SCANCODE_KP_SPACE, SDL_SCANCODE_KP_CLEAR,
	SDL_SCANCODE_LCTRL, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_LALT,
	SDL_SCANCODE_RCTRL, SDL_SCANCODE_RSHIFT,
	SDL_NUM_SCANCODES
} SDL_Scancode;

typedef Sint32 SDL_Keycode;

#define SDLK_UNKNOWN 0
#define SDLK_BACKSPACE 8
#define SDLK_TAB 9
#define SDLK_RETURN 13
#define SDLK_ESCAPE 27
#define SDLK_SPACE 32
#define SDLK_EXCLAIM 33
#define SDLK_QUOTEDBL 34
#define SDLK_HASH 35
#define SDLK_DOLLAR 36
#define SDLK_AMPERSAND 38
#define SDLK_QUOTE 39
#define SDLK_LEFTPAREN 40
#define SDLK_RIGHTPAREN 41
#define SDLK_ASTERISK 42
#define SDLK_PLUS 43
#define SDLK_COMMA 44
#define SDLK_MINUS 45
#define SDLK_PERIOD 46
#define SDLK_SLASH 47
#define SDLK_0 48
#define SDLK_1 49
#define SDLK_2 50
#define SDLK_3 51
#define SDLK_4 52
#define SDLK_5 53
#define SDLK_6 54
#define SDLK_7 55
#define SDLK_8 56
#define SDLK_9 57
#define SDLK_COLON 58
#define SDLK_SEMICOLON 59
#define SDLK_LESS 60
#define SDLK_EQUALS 61
#define SDLK_GREATER 62
#define SDLK_QUESTION 63
#define SDLK_AT 64
#define SDLK_a 97
#define SDLK_b 98
#define SDLK_c 99
#define SDLK_d 100
#define SDLK_e 101
#define SDLK_f 102
#define SDLK_g 103
#define SDLK_h 104
#define SDLK_i 105
#define SDLK_j 106
#define SDLK_k 107
#define SDLK_l 108
#define SDLK_m 109
#define SDLK_n 110
#define SDLK_o 111
#define SDLK_p 112
#define SDLK_q 113
#define SDLK_r 114
#define SDLK_s 115
#define SDLK_t 116
#define SDLK_u 117
#define SDLK_v 118
#define SDLK_w 119
#define SDLK_x 120
#define SDLK_y 121
#define SDLK_z 122
#define SDLK_LEFTBRACKET 91
#define SDLK_BACKSLASH 92
#define SDLK_RIGHTBRACKET 93
#define SDLK_CARET 94
#define SDLK_UNDERSCORE 95
#define SDLK_BACKQUOTE 96
#define SDLK_DELETE 127
#define SDLK_CAPSLOCK 1073741881
#define SDLK_F1 1073741882
#define SDLK_F2 1073741883
#define SDLK_F3 1073741884
#define SDLK_F4 1073741885
#define SDLK_F5 1073741886
#define SDLK_F6 1073741887
#define SDLK_F7 1073741888
#define SDLK_F8 1073741889
#define SDLK_F9 1073741890
#define SDLK_F10 1073741891
#define SDLK_F11 1073741892
#define SDLK_F12 1073741893
#define SDLK_F13 1073741894
#define SDLK_PRINTSCREEN 1073741895
#define SDLK_SCROLLLOCK 1073741896
#define SDLK_PAUSE 1073741897
#define SDLK_INSERT 1073741898
#define SDLK_HOME 1073741899
#define SDLK_PAGEUP 1073741900
#define SDLK_END 1073741901
#define SDLK_PAGEDOWN 1073741902
#define SDLK_RIGHT 1073741903
#define SDLK_LEFT 1073741904
#define SDLK_DOWN 1073741905
#define SDLK_UP 1073741906
#define SDLK_NUMLOCKCLEAR 1073741907
#define SDLK_KP_DIVIDE 1073741908
#define SDLK_KP_MULTIPLY 1073741909
#define SDLK_KP_MINUS 1073741910
#define SDLK_KP_PLUS 1073741911
#define SDLK_KP_ENTER 1073741912
#define SDLK_KP_1 1073741913
#define SDLK_KP_2 1073741914
#define SDLK_KP_3 1073741915
#define SDLK_KP_4 1073741916
#define SDLK_KP_5 1073741917
#define SDLK_KP_6 1073741918
#define SDLK_KP_7 1073741919
#define SDLK_KP_8 1073741920
#define SDLK_KP_9 1073741921
#define SDLK_KP_0 1073741922
#define SDLK_KP_PERIOD 1073741923
#define SDLK_KP_EQUALS 1073741924
#define SDLK_KP_LEFTPAREN 1073741925
#define SDLK_KP_RIGHTPAREN 1073741926
#define SDLK_LCTRL 1073742048
#define SDLK_LSHIFT 1073742049
#define SDLK_LALT 1073742050
#define SDLK_LGUI 1073742051
#define SDLK_RCTRL 1073742052
#define SDLK_RSHIFT 1073742053
#define SDLK_RALT 1073742054
#define SDLK_RGUI 1073742055
#define SDLK_MODE 1073742081
#define SDLK_HELP 1073741941
#define SDLK_UNDO 1073741942
#define SDLK_CLEAR 1073741980

typedef enum {
	SDL_FIRSTEVENT = 0,
	SDL_QUIT = 0x100,
	SDL_WINDOWEVENT = 0x200,
	SDL_KEYDOWN = 0x300,
	SDL_KEYUP = 0x301,
	SDL_TEXTINPUT = 0x303,
	SDL_KEYMAPCHANGED = 0x304,
	SDL_MOUSEMOTION = 0x400,
	SDL_MOUSEBUTTONDOWN = 0x401,
	SDL_MOUSEBUTTONUP = 0x402,
	SDL_MOUSEWHEEL = 0x403,
	SDL_JOYAXISMOTION = 0x600,
	SDL_JOYBALLMOTION = 0x601,
	SDL_JOYHATMOTION = 0x602,
	SDL_JOYBUTTONDOWN = 0x603,
	SDL_JOYBUTTONUP = 0x604
} SDL_EventType;

typedef struct SDL_Keysym {
	SDL_Scancode scancode;
	SDL_Keycode sym;
	Uint16 mod;
	Uint32 unused;
} SDL_Keysym;

typedef struct SDL_KeyboardEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	Uint8 state;
	Uint8 repeat;
	Uint8 padding2;
	Uint8 padding3;
	SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	Uint32 which;
	Uint32 state;
	Sint32 x;
	Sint32 y;
	Sint32 xrel;
	Sint32 yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	Uint32 which;
	Uint8 button;
	Uint8 state;
	Uint8 clicks;
	Uint8 padding1;
	Sint32 x;
	Sint32 y;
} SDL_MouseButtonEvent;

typedef struct SDL_MouseWheelEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	Uint32 which;
	Sint32 x;
	Sint32 y;
	Uint32 direction;
} SDL_MouseWheelEvent;

typedef struct SDL_WindowEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	Uint8 event;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint32 data1;
	Sint32 data2;
} SDL_WindowEvent;

typedef struct SDL_TextInputEvent {
	Uint32 type;
	Uint32 timestamp;
	Uint32 windowID;
	char text[32];
} SDL_TextInputEvent;

typedef struct SDL_JoyAxisEvent {
	Uint32 type;
	Uint32 timestamp;
	Sint32 which;
	Uint8 axis;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 value;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyHatEvent {
	Uint32 type;
	Uint32 timestamp;
	Sint32 which;
	Uint8 hat;
	Uint8 value;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent {
	Uint32 type;
	Uint32 timestamp;
	Sint32 which;
	Uint8 button;
	Uint8 state;
} SDL_JoyButtonEvent;

typedef struct SDL_JoyBallEvent {
	Uint32 type;
	Uint32 timestamp;
	Sint32 which;
	Uint8 ball;
	Sint16 xrel;
	Sint16 yrel;
} SDL_JoyBallEvent;

typedef union SDL_Event {
	Uint32 type;
	SDL_KeyboardEvent key;
	SDL_MouseMotionEvent motion;
	SDL_MouseButtonEvent button;
	SDL_MouseWheelEvent wheel;
	SDL_WindowEvent window;
	SDL_TextInputEvent text;
	SDL_JoyAxisEvent jaxis;
	SDL_JoyHatEvent jhat;
	SDL_JoyButtonEvent jbutton;
	SDL_JoyBallEvent jball;
} SDL_Event;

typedef struct SDL_Rect {
	int x, y, w, h;
} SDL_Rect;

typedef struct SDL_Color {
	Uint8 r, g, b, a;
} SDL_Color;

typedef struct SDL_Palette {
	int ncolors;
	SDL_Color *colors;
	Uint32 version;
	int refcount;
} SDL_Palette;

typedef struct SDL_PixelFormat {
	Uint32 format;
	SDL_Palette *palette;
	Uint8 BitsPerPixel;
	Uint8 BytesPerPixel;
	Uint8 padding[2];
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	Uint8 Rloss;
	Uint8 Gloss;
	Uint8 Bloss;
	Uint8 Aloss;
	Uint8 Rshift;
	Uint8 Gshift;
	Uint8 Bshift;
	Uint8 Ashift;
	int refcount;
} SDL_PixelFormat;

typedef struct SDL_Surface {
	Uint32 flags;
	SDL_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;
	void *userdata;
	int locked;
	SDL_Rect clip_rect;
	int refcount;
} SDL_Surface;

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Joystick SDL_Joystick;

typedef struct SDL_RendererInfo {
	char name[128];
	Uint32 flags;
	Uint32 num_texture_formats;
	Uint32 texture_formats[16];
	int max_texture_width;
	int max_texture_height;
} SDL_RendererInfo;

typedef struct SDL_DisplayMode {
	Uint32 format;
	int w;
	int h;
	int refresh_rate;
	void *driverdata;
} SDL_DisplayMode;

typedef void (*SDL_AudioCallback)(void *userdata, Uint8 *stream, int len);

typedef struct SDL_AudioSpec {
	int freq;
	Uint16 format;
	Uint8 channels;
	Uint8 silence;
	Uint16 samples;
	Uint16 padding;
	Uint32 size;
	SDL_AudioCallback callback;
	void *userdata;
} SDL_AudioSpec;

typedef struct SDL_version {
	Uint8 major;
	Uint8 minor;
	Uint8 patch;
} SDL_version;

typedef struct SDL_SysWMinfo {
	SDL_version version;
	int subsystem;
	union {
		struct { void *display; unsigned long window; } x11;
	} info;
} SDL_SysWMinfo;

#define SDL_MAJOR_VERSION 2
#define SDL_MINOR_VERSION 0
#define SDL_PATCHLEVEL 0
#define SDL_VERSION(v)                        \
	do {                                  \
		(v)->major = SDL_MAJOR_VERSION; \
		(v)->minor = SDL_MINOR_VERSION; \
		(v)->patch = SDL_PATCHLEVEL;    \
	} while (0)

#define SDL_MUSTLOCK(S) (0)

#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SDL_BYTEORDER SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER SDL_LIL_ENDIAN
#endif

void *SDL_malloc(size_t size);
void SDL_free(void *ptr);
char *SDL_getenv(const char *name);
size_t SDL_strlen(const char *s);
size_t SDL_strlcpy(char *dst, const char *src, size_t maxlen);

Uint16 SDL_Swap16(Uint16 x);
Uint32 SDL_Swap32(Uint32 x);
Uint16 SDL_SwapBE16(Uint16 x);
Uint16 SDL_SwapLE16(Uint16 x);
Uint32 SDL_SwapLE32(Uint32 x);

int SDL_Init(Uint32 flags);
int SDL_InitSubSystem(Uint32 flags);
void SDL_Quit(void);
void SDL_QuitSubSystem(Uint32 flags);
Uint32 SDL_WasInit(Uint32 flags);

const char *SDL_GetError(void);
Uint32 SDL_GetTicks(void);
void SDL_Delay(Uint32 ms);

int SDL_PollEvent(SDL_Event *event);
void SDL_PumpEvents(void);
int SDL_PushEvent(SDL_Event *event);
int SDL_WaitEvent(SDL_Event *event);

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window *window);
void SDL_SetWindowTitle(SDL_Window *window, const char *title);
void SDL_SetWindowSize(SDL_Window *window, int w, int h);
void SDL_GetWindowSize(SDL_Window *window, int *w, int *h);
Uint32 SDL_GetWindowFlags(SDL_Window *window);
void SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon);
void SDL_MinimizeWindow(SDL_Window *window);
SDL_Surface *SDL_GetWindowSurface(SDL_Window *window);
int SDL_UpdateWindowSurfaceRects(SDL_Window *window, const SDL_Rect *rects, int numrects);
int SDL_GetWindowWMInfo(SDL_Window *window, SDL_SysWMinfo *info);
int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode);

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer *renderer);
int SDL_GetRendererInfo(SDL_Renderer *renderer, SDL_RendererInfo *info);
int SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h);
int SDL_RenderSetScale(SDL_Renderer *renderer, float scaleX, float scaleY);
int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int SDL_RenderClear(SDL_Renderer *renderer);
int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect);
void SDL_RenderPresent(SDL_Renderer *renderer);
SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h);
void SDL_DestroyTexture(SDL_Texture *texture);
int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch);

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
void SDL_FreeSurface(SDL_Surface *surface);
int SDL_LockSurface(SDL_Surface *surface);
void SDL_UnlockSurface(SDL_Surface *surface);
int SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color);
int SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);
int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);
int SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors, int firstcolor, int ncolors);
SDL_Surface *SDL_LoadBMP(const char *file);
typedef struct SDL_RWops SDL_RWops;
SDL_RWops *SDL_RWFromFile(const char *file, const char *mode);
int SDL_SaveBMP_RW(SDL_Surface *surface, SDL_RWops *dst, int freedst);
int SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority);

int SDL_ShowCursor(int toggle);
int SDL_SetRelativeMouseMode(SDL_bool enabled);
SDL_bool SDL_GetRelativeMouseMode(void);
Uint32 SDL_GetMouseState(int *x, int *y);
void SDL_WarpMouseInWindow(SDL_Window *window, int x, int y);
void SDL_WarpMouse(Uint16 x, Uint16 y);
void SDL_StartTextInput(void);
void SDL_StopTextInput(void);
void SDL_SetTextInputRect(const SDL_Rect *rect);

const Uint8 *SDL_GetKeyboardState(int *numkeys);
typedef Uint16 SDL_Keymod;
SDL_Keymod SDL_GetModState(void);
SDL_Keycode SDL_GetKeyFromName(const char *name);
const char *SDL_GetKeyName(SDL_Keycode key);
SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key);

int SDL_NumJoysticks(void);
SDL_Joystick *SDL_JoystickOpen(int device_index);
void SDL_JoystickClose(SDL_Joystick *joystick);
const char *SDL_JoystickName(SDL_Joystick *joystick);
int SDL_JoystickNumAxes(SDL_Joystick *joystick);
int SDL_JoystickNumButtons(SDL_Joystick *joystick);
Sint16 SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis);
Uint8 SDL_JoystickGetButton(SDL_Joystick *joystick, int button);
Uint8 SDL_JoystickGetHat(SDL_Joystick *joystick, int hat);

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
Uint32 SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes);
void SDL_CloseAudio(void);
void SDL_CloseAudioDevice(Uint32 dev);
void SDL_PauseAudio(int pause_on);
void SDL_PauseAudioDevice(Uint32 dev, int pause_on);
void SDL_LockAudio(void);
void SDL_UnlockAudio(void);

#ifdef __cplusplus
}
#endif

#endif
