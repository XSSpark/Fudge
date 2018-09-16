#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;

static unsigned int root_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    count = task_read(state->task, buffer, count);

    if (!count)
        kernel_blocktask(state->task);

    return count;

}

static unsigned int root_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_header *header = buffer;

    if (count < sizeof (struct event_header))
        return 0;

    if (header->length != count)
        return 0;

    if (!header->source)
        header->source = state->task->id;

    if (header->destination)
        return kernel_unicastevent(&self->states, header);
    else
        return kernel_multicastevent(&self->states, header);

}

static unsigned int root_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_MAILBOX, "event");

    root.operations.read = root_read;
    root.operations.write = root_write;
    root.operations.seek = root_seek;

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

