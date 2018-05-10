#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "registers.h"

#define VGA_COLORMAP_LIMIT              256

struct vga_character
{

    char character;
    char color;

};

static struct base_driver driver;
static struct console_interface consoleinterface;
static struct video_interface videointerface;
static struct {unsigned char color; unsigned int offset;} cursor;
static struct vga_character *taddress;
static void *gaddress;

static void clear(unsigned int offset)
{

    unsigned int total = videointerface.settings.w * videointerface.settings.h;
    unsigned int i;

    for (i = offset; i < total; i++)
    {

        taddress[i].character = ' ';
        taddress[i].color = cursor.color;

    }

}

static unsigned int consoleinterface_ctrlread(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &consoleinterface.settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int consoleinterface_odatawrite(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int total = videointerface.settings.w * videointerface.settings.h;
    unsigned int i;

    if (videointerface.settings.w != 80)
        return count;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            cursor.offset--;

        if (c == '\t')
            cursor.offset = (cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            cursor.offset -= (cursor.offset % videointerface.settings.w);

        if (c == '\n')
            cursor.offset += videointerface.settings.w - (cursor.offset % videointerface.settings.w);

        if (c >= ' ')
        {

            taddress[cursor.offset].character = c;
            taddress[cursor.offset].color = cursor.color;
            cursor.offset++;

        }

        if (cursor.offset >= total)
        {

            memory_copy(taddress, taddress + videointerface.settings.w, videointerface.settings.w * (videointerface.settings.h - 1) * sizeof (struct vga_character));
            clear(videointerface.settings.w * (videointerface.settings.h - 1));

            cursor.offset -= videointerface.settings.w;

        }

    }

    outcrt1(VGA_CRTINDEX_CRT0E, cursor.offset >> 8);
    outcrt1(VGA_CRTINDEX_CRT0F, cursor.offset);

    return count;

}

static unsigned int videointerface_ctrlread(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &videointerface.settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int videointerface_ctrlwrite(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_videosettings *settings = buffer;

    if (settings->w == 80)
    {

        if (videointerface.settings.w == 320)
            vga_restore();

        ctrl_setvideosettings(&videointerface.settings, 80, 25, 2);
        vga_settext();

    }

    else
    {

        if (videointerface.settings.w == 80)
            vga_save();

        ctrl_setvideosettings(&videointerface.settings, 320, 200, 1);
        vga_setgraphic();

    }

    video_notifymode(&videointerface, videointerface.settings.w, videointerface.settings.h, videointerface.settings.bpp);

    return count;

}

static unsigned int videointerface_dataread(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int s = videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp;

    return memory_read(buffer, count, gaddress, s, offset);

}

static unsigned int videointerface_datawrite(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned int s = videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp;

    return memory_write(gaddress, s, buffer, count, offset);

}

static unsigned int videointerface_colormapread(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * 3; i += 3)
    {

        io_outb(VGA_REGISTER_DACRINDEX, i / 3);
        c[i + 0] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 1] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 2] = io_inb(VGA_REGISTER_DACDATA);

    }

    return i - offset;

}

static unsigned int videointerface_colormapwrite(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * 3; i += 3)
    {

        io_outb(VGA_REGISTER_DACWINDEX, i / 3);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return i - offset;

}

static void driver_init(void)
{

    taddress = (struct vga_character *)0x000B8000;
    gaddress = (void *)0x000A0000;
    cursor.color = 0x0F;

    console_initinterface(&consoleinterface);
    video_initinterface(&videointerface);
    ctrl_setconsolesettings(&consoleinterface.settings, 1);
    ctrl_setvideosettings(&videointerface.settings, 80, 25, 2);
    clear(0);

    consoleinterface.ctrl.operations.read = consoleinterface_ctrlread;
    consoleinterface.odata.operations.write = consoleinterface_odatawrite;
    videointerface.ctrl.operations.read = videointerface_ctrlread;
    videointerface.ctrl.operations.write = videointerface_ctrlwrite;
    videointerface.data.operations.read = videointerface_dataread;
    videointerface.data.operations.write = videointerface_datawrite;
    videointerface.colormap.operations.read = videointerface_colormapread;
    videointerface.colormap.operations.write = videointerface_colormapwrite;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface, id);
    video_registerinterface(&videointerface, id);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "vga", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

