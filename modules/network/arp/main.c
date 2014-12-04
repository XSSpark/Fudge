#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_protocol protocol;

void init()
{

    network_initprotocol(&protocol, "arp");
    network_registerprotocol(&protocol);

}

void destroy()
{

    network_unregisterprotocol(&protocol);

}

