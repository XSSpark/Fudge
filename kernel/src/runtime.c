#include <memory.h>
#include <runtime.h>

unsigned int runtime_set_task_event(struct runtime_task *task, unsigned int index, unsigned int callback)
{

    if (index >= RUNTIME_TASK_EVENT_SLOTS)
        return 0;

    task->events[index].callback = callback;

    return 1;

}

unsigned int runtime_unset_task_event(struct runtime_task *task, unsigned int index)
{

    if (index >= RUNTIME_TASK_EVENT_SLOTS)
        return 0;

    task->events[index].callback = 0;

    return 1;

}

static struct runtime_task *notify_interrupt(struct runtime_task *self, unsigned int index)
{

    return self;

}

struct runtime_descriptor *runtime_get_task_descriptor(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_DESCRIPTOR_SLOTS)
        return 0;

    return &task->descriptors[index];

}

struct runtime_mount *runtime_get_task_mount(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_MOUNT_SLOTS)
        return 0;

    return &task->mounts[index];

}

struct runtime_mount *runtime_find_task_mount(struct runtime_task *self, unsigned int count, char *path)
{

    unsigned int i;
    unsigned int max = 0;
    struct runtime_mount *current = 0;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (!self->mounts[i].count)
            continue;

        if (self->mounts[i].count < max)
            continue;

        if (memory_match(self->mounts[i].path, path, self->mounts[i].count))
        {

            current = &self->mounts[i];
            max = self->mounts[i].count;

        }

    }

    return current;

}

void runtime_init_descriptor(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_interface *interface)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->id = id;
    descriptor->interface = interface;

}

void runtime_init_mount(struct runtime_mount *mount, struct vfs_interface *parent, unsigned int parentid, struct vfs_interface *child, unsigned int childid, unsigned int count, char *path)
{

    memory_clear(mount, sizeof (struct runtime_mount));

    mount->parent = parent;
    mount->parentid = parentid;
    mount->child = child;
    mount->childid = childid;

    /* remove later on */
    mount->count = count;
    memory_write(mount->path, 64, path, mount->count, 0);

}

void runtime_init_registers(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb, unsigned int status)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->sb = sb;
    registers->status = status;

}

void runtime_init_task(struct runtime_task *task)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->notify_interrupt = notify_interrupt;

}

