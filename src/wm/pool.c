#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"

#define MAX_WIDGETS                     1024
#define MAX_FONTS                       32
#define FONTDATA_SIZE                   0x8000

struct entry
{

    struct widget widget;
    union
    {

        struct widget_button button;
        struct widget_choice choice;
        struct widget_layout layout;
        struct widget_fill fill;
        struct widget_image image;
        struct widget_listbox listbox;
        struct widget_select select;
        struct widget_text text;
        struct widget_textbox textbox;
        struct widget_textbutton textbutton;
        struct widget_window window;

    } payload;

};

static struct list widgetlist;
static struct list bumplist;
static struct list freelist;
static struct entry entries[MAX_WIDGETS];
static struct list_item items[MAX_WIDGETS];
static struct text_font fonts[MAX_FONTS];
static unsigned char fontnormal[FONTDATA_SIZE];
static unsigned char fontbold[FONTDATA_SIZE];

struct list_item *pool_prev(struct list_item *current)
{

    return (current) ? current->prev : widgetlist.tail;

}

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, struct widget *parent)
{

    if (!strpool_getcstringlength(parent->id))
        return 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;
        struct widget *widget = &entry->widget;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(strpool_getstring(widget->in), strpool_getstring(parent->id)))
                return current;

        }

    }

    return 0;

}

struct list_item *pool_nextsource(struct list_item *current, unsigned int source)
{

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (entry->widget.source == source)
            return current;

    }

    return 0;

}

struct widget *pool_getwidgetbyid(unsigned int source, char *id)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (entry->widget.source != source)
            continue;

        if (cstring_match(strpool_getstring(entry->widget.id), id))
            return &entry->widget;

    }

    return 0;

}

static struct list_item *finditem(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (&entry->widget == widget)
            return current;

    }

    return 0;

}

static void bump(struct list_item *item)
{

    struct entry *entry = item->data;
    struct list_item *current = item->prev;

    list_move(&bumplist, &widgetlist, item);

    while ((current = pool_nextin(current, &entry->widget)))
    {

        struct list_item *prev = current->prev;

        bump(current);

        current = prev;

    }

}

void pool_bump(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
        bump(item);

    while (bumplist.head)
        list_move(&widgetlist, &bumplist, bumplist.head);

}

struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in)
{

    struct list_item *item = list_pickhead(&freelist);

    if (item)
    {

        struct entry *entry = item->data;

        widget_init(&entry->widget, source, type, id, in, &entry->payload);
        list_move(&widgetlist, &freelist, item);

        return &entry->widget;

    }

    return 0;

}

void pool_destroy(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
    {

        widget_unsetattributes(widget);
        list_move(&freelist, &widgetlist, item);

    }

}

void pool_pcxload(struct pool_pcxresource *pcxresource, char *source)
{

    if (!pcxresource->cached)
    {

        unsigned int target = fs_auth(source);

        if (target)
        {

            unsigned int id = fs_walk(1, target, 0, source);

            if (id)
            {

                struct pcx_header header;
                struct record record;
                unsigned char magic;

                fs_stat(1, target, id, &record);
                fs_read_all(1, target, id, &header, sizeof (struct pcx_header), 0);
                fs_read_full(1, target, id, pcxresource->data, 0x10000, 128);

                pcxresource->width = header.xend - header.xstart + 1;
                pcxresource->height = header.yend - header.ystart + 1;

                fs_read_all(1, target, id, &magic, 1, record.size - 768 - 1);

                if (magic == PCX_COLORMAP_MAGIC)
                    fs_read_all(1, target, id, pcxresource->colormap, 768, record.size - 768);

                pcxresource->cached = 1;

            }

        }

    }

}

void pool_pcxreadline(struct pool_pcxresource *pcxresource, int line, int y, unsigned char *buffer)
{

    if (pcxresource->lastline == line - 1)
    {

        pcxresource->lastoffset += pcx_readline(pcxresource->data + pcxresource->lastoffset, pcxresource->width, buffer);
        pcxresource->lastline = line;

    }

    else
    {

        int h;

        pcxresource->lastoffset = 0;

        for (h = 0; h < line - y + 1; h++)
            pcxresource->lastoffset += pcx_readline(pcxresource->data + pcxresource->lastoffset, pcxresource->width, buffer);

        pcxresource->lastline = line;

    }

}

struct text_font *pool_getfont(unsigned int index)
{

    return &fonts[index];

}

void pool_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding)
{

    struct text_font *font = pool_getfont(index);

    font->data = data;
    font->bitmapdata = pcf_getbitmapdata(font->data);
    font->bitmapalign = pcf_getbitmapalign(font->data);
    font->lineheight = lineheight;
    font->padding = padding;

}

void pool_loadfont(unsigned int factor)
{

    unsigned int target = fs_auth("initrd:");

    if (target)
    {

        unsigned int id1 = 0;
        unsigned int id2 = 0;

        switch (factor)
        {

        case 0:
            id1 = fs_walk(1, target, 0, "data/font/ter-112n.pcf");
            id2 = fs_walk(1, target, 0, "data/font/ter-112b.pcf");

            break;

        case 1:
            id1 = fs_walk(1, target, 0, "data/font/ter-114n.pcf");
            id2 = fs_walk(1, target, 0, "data/font/ter-114b.pcf");

            break;

        case 2:
            id1 = fs_walk(1, target, 0, "data/font/ter-116n.pcf");
            id2 = fs_walk(1, target, 0, "data/font/ter-116b.pcf");

            break;

        default:
            id1 = fs_walk(1, target, 0, "/data/font/ter-118n.pcf");
            id2 = fs_walk(1, target, 0, "/data/font/ter-118b.pcf");

            break;

        }

        if (id1 && id2)
        {

            unsigned int lineheight = 12 + factor * 4;
            unsigned int padding = 4 + factor * 2;

            fs_read_full(1, target, id1, fontnormal, FONTDATA_SIZE, 0);
            fs_read_full(1, target, id2, fontbold, FONTDATA_SIZE, 0);
            pool_setfont(ATTR_WEIGHT_NORMAL, fontnormal, lineheight, padding);
            pool_setfont(ATTR_WEIGHT_BOLD, fontbold, lineheight, padding);

        }

    }

}

void pool_setup(void)
{

    unsigned int i;

    list_init(&widgetlist);
    list_init(&bumplist);
    list_init(&freelist);

    for (i = 0; i < MAX_WIDGETS; i++)
    {

        struct list_item *item = &items[i];

        list_inititem(item, &entries[i]);
        list_add(&freelist, item);

    }

}

