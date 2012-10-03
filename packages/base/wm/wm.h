#ifndef WM
#define WM

#define SCREEN_WIDTH                1024
#define SCREEN_HEIGHT               768
#define SCREEN_BPP                  32
#define SCREEN_BSIZE                4

#define SCREEN_BACKGROUND           0x002C1319
#define FRAME_BACKGROUND            0x003C2329
#define FRAME_BORDER_DARK           0x001C0309
#define FRAME_BORDER_LIGHT          0x004C3339
#define WINDOW_BACKGROUND           0x00D6C6C0
#define PANEL_BACKGROUND            0x00E6D6D0
#define PANEL_BORDER_DARK           0x00C6B6B0
#define PANEL_BORDER_LIGHT          0x00F6E6E0

struct window
{

    unsigned int x;
    unsigned int y;
    struct gfx_surface *surface;
    void (*draw)(struct window *self);

};

#endif

