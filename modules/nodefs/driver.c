#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>

static unsigned int filesystem_read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id == 1)
    {

        void *b = buffer;
        unsigned int i;

        for (i = 0; i < filesystem->count; i++)
        {

            string_write(b, "%s\n", filesystem->nodes[i]->name);
            b += string_length(b);

        }

        return string_length(buffer);

    }

    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->read(node, count, buffer);

}

static unsigned int filesystem_write(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->write(node, count, buffer);

}

static unsigned int filesystem_find(struct vfs_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    unsigned int i;

    for (i = 0; i < filesystem->count; i++)
    {

        if (string_find(filesystem->nodes[i]->name, name))
            return i + 2;

    }

    return 0;

}

static void register_node(struct nodefs_driver *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->filesystem.nodes[i])
        {

            self->filesystem.nodes[i] = node;
            self->filesystem.count++;

            break;

        }

    }

}

static void unregister_node(struct nodefs_driver *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (self->filesystem.nodes[i] == node)
        {

            self->filesystem.nodes[i] = 0;
            self->filesystem.count--;

            break;

        }

    }

}

void nodefs_filesystem_init(struct nodefs_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    vfs_filesystem_init(&filesystem->base, 0, 0, filesystem_read, filesystem_write, filesystem_find, 0); 
    filesystem->count = 0;

    vfs_mount(&filesystem->base, "/module/");

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    memory_clear(driver, sizeof (struct nodefs_driver));

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE, "nodefs");
    nodefs_filesystem_init(&driver->filesystem);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

