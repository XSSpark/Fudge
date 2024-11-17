#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct service service;

static struct resource *service_foreach(struct resource *current)
{

    return resource_foreachtype(current, RESOURCE_CLOCKINTERFACE);

}

static unsigned int service_getinode(struct resource *current, unsigned int index)
{

    struct clock_interface *interface = current->data;

    return interface->inode;

}

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct clock_interface *interface = kernel_getinterface(target);

    switch (event)
    {

    case EVENT_INFO:
        return oninfo(interface, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_link(0, 0, &interface->resource, place);
    interface->oninfo = oninfo;

}

void module_init(void)
{

    service_init(&service, "clock", service_foreach, service_getinode);
    service_register(&service);

}

