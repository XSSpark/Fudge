#define RENDER_FONTNORMAL               0
#define RENDER_FONTBOLD                 1

struct render_rowinfo
{

    unsigned int chars;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    unsigned int newline;

};

struct render_textinfo
{

    unsigned int rows;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    struct render_rowinfo last;

};

unsigned int render_getrowinfo(unsigned int index, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int render_gettextinfo(unsigned int index, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw);
void render_damage(struct blit_damage *damage, int x0, int y0, int x1, int y1);
void render_render(struct blit_display *display, struct blit_damage *damage);
void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding);
void render_setup(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
