#include <fudge/module.h>
#include <base/base.h>
#include <log/log.h>
#include <net/net.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "rtl8139.h"

#define RTL8139_PCI_VENDOR              0x10EC
#define RTL8139_PCI_DEVICE              0x8139

enum rtl8139_register
{

    RTL8139_REGISTER_IDR0               = 0x00,
    RTL8139_REGISTER_IDR1               = 0x01,
    RTL8139_REGISTER_IDR2               = 0x02,
    RTL8139_REGISTER_IDR3               = 0x03,
    RTL8139_REGISTER_IDR4               = 0x04,
    RTL8139_REGISTER_IDR5               = 0x05,
    RTL8139_REGISTER_MAR0               = 0x08,
    RTL8139_REGISTER_MAR1               = 0x09,
    RTL8139_REGISTER_MAR2               = 0x0A,
    RTL8139_REGISTER_MAR3               = 0x0B,
    RTL8139_REGISTER_MAR4               = 0x0C,
    RTL8139_REGISTER_MAR5               = 0x0D,
    RTL8139_REGISTER_MAR6               = 0x0E,
    RTL8139_REGISTER_MAR7               = 0x0F,
    RTL8139_REGISTER_TSD0               = 0x10,
    RTL8139_REGISTER_TSD1               = 0x14,
    RTL8139_REGISTER_TSD2               = 0x18,
    RTL8139_REGISTER_TSD3               = 0x1C,
    RTL8139_REGISTER_TSAD0              = 0x20,
    RTL8139_REGISTER_TSAD1              = 0x24,
    RTL8139_REGISTER_TSAD2              = 0x28,
    RTL8139_REGISTER_TSAD3              = 0x2C,
    RTL8139_REGISTER_RBSTART            = 0x30,
    RTL8139_REGISTER_ERBCR              = 0x34,
    RTL8139_REGISTER_ERSR               = 0x36,
    RTL8139_REGISTER_CR                 = 0x37,
    RTL8139_REGISTER_CAPR               = 0x38,
    RTL8139_REGISTER_CBR                = 0x3A,
    RTL8139_REGISTER_IMR                = 0x3C,
    RTL8139_REGISTER_ISR                = 0x3E,
    RTL8139_REGISTER_TCR                = 0x40,
    RTL8139_REGISTER_RCR                = 0x44,
    RTL8139_REGISTER_TCTR               = 0x48,
    RTL8139_REGISTER_MPC                = 0x4C,
    RTL8139_REGISTER_9346CR             = 0x50,
    RTL8139_REGISTER_CONFIG0            = 0x51,
    RTL8139_REGISTER_CONFIG1            = 0x52,
    RTL8139_REGISTER_TIMERINT           = 0x54,
    RTL8139_REGISTER_MSR                = 0x58,
    RTL8139_REGISTER_CONFIG3            = 0x59,
    RTL8139_REGISTER_CONFIG4            = 0x5A,
    RTL8139_REGISTER_MULINT             = 0x5C,
    RTL8139_REGISTER_RERID              = 0x5E,
    RTL8139_REGISTER_TSAD               = 0x60,
    RTL8139_REGISTER_BMCR               = 0x62,
    RTL8139_REGISTER_BMSR               = 0x64,
    RTL8139_REGISTER_ANAR               = 0x66,
    RTL8139_REGISTER_ANLPAR             = 0x68,
    RTL8139_REGISTER_ANER               = 0x6A,
    RTL8139_REGISTER_DIS                = 0x6C

};

enum rtl8139_isr
{

    RTL8139_ISR_ROK                     = (1 << 0),
    RTL8139_ISR_TOK                     = (1 << 2)

};

enum rtl8139_headerflag
{

    RTL8139_HEADERFLAG_ROK              = (1 << 0),
    RTL8139_HEADERFLAG_FAE              = (1 << 1),
    RTL8139_HEADERFLAG_CRC              = (1 << 2),
    RTL8139_HEADERFLAG_LONG             = (1 << 3),
    RTL8139_HEADERFLAG_RUNT             = (1 << 4),
    RTL8139_HEADERFLAG_ISE              = (1 << 5),
    RTL8139_HEADERFLAG_BAR              = (1 << 13),
    RTL8139_HEADERFLAG_PAM              = (1 << 14),
    RTL8139_HEADERFLAG_MAR              = (1 << 15)

};

struct rtl8139_header
{

    unsigned short flags;
    unsigned short length;

};

static void poweron(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CONFIG1, 0x00);

}

static void reset(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CR, 0x10);

    while (io_inb(self->io + RTL8139_REGISTER_CR) & 0x10);

}

static void enable(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CR, 0x0C);

}

static void setup_interrupts(struct rtl8139_driver *self, unsigned short flags)
{

    io_outw(self->io + RTL8139_REGISTER_IMR, flags);

}

static void setup_receiver(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_REGISTER_RBSTART, (unsigned int)self->rx);
    io_outd(self->io + RTL8139_REGISTER_RCR, 0xBF);

    log_write_string("RBSTART: 0x");
    log_write_number((unsigned int)self->rx, 16);
    log_write_string("\n");

}

static void setup_transmitter(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_REGISTER_TSAD0, (unsigned int)self->tx0);
    io_outd(self->io + RTL8139_REGISTER_TSAD1, (unsigned int)self->tx1);
    io_outd(self->io + RTL8139_REGISTER_TSAD2, (unsigned int)self->tx2);
    io_outd(self->io + RTL8139_REGISTER_TSAD3, (unsigned int)self->tx3);

}

static void handle_irq(struct base_device *device)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)device->driver;
    unsigned short status = io_inw(driver->io + RTL8139_REGISTER_ISR);

    log_write_string("Interrupt status: 0x");
    log_write_number(status, 16);
    log_write_string("\n");

    if (status & RTL8139_ISR_ROK)
    {

        unsigned char cr = io_inb(driver->io + RTL8139_REGISTER_CR);
        unsigned short cbr = io_inw(driver->io + RTL8139_REGISTER_CBR);
        unsigned short capr = io_inw(driver->io + RTL8139_REGISTER_CAPR);
        struct rtl8139_header *header = (struct rtl8139_header *)(driver->rx);

        log_write_string("CR: 0x");
        log_write_number(cr, 16);
        log_write_string(" ");
        log_write_string("CBR: 0x");
        log_write_number(cbr, 16);
        log_write_string(" ");
        log_write_string("CAPR: 0x");
        log_write_number(capr, 16);
        log_write_string(" ");
        log_write_string("HF: 0x");
        log_write_number(header->flags, 16);
        log_write_string(" ");
        log_write_string("HL: 0x");
        log_write_number(header->length, 16);
        log_write_string("\n");

        io_outw(driver->io + RTL8139_REGISTER_CAPR, 0);
        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_ROK);

    }

    if (status & RTL8139_ISR_TOK)
        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_TOK);

}

static void start(struct base_driver *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;

    poweron(driver);
    reset(driver);
    setup_interrupts(driver, 0xD07F);
    setup_receiver(driver);
    setup_transmitter(driver);
    enable(driver);

    driver->inet.mac[0] = io_inb(driver->io + RTL8139_REGISTER_IDR0);
    driver->inet.mac[1] = io_inb(driver->io + RTL8139_REGISTER_IDR1);
    driver->inet.mac[2] = io_inb(driver->io + RTL8139_REGISTER_IDR2);
    driver->inet.mac[3] = io_inb(driver->io + RTL8139_REGISTER_IDR3);
    driver->inet.mac[4] = io_inb(driver->io + RTL8139_REGISTER_IDR4);
    driver->inet.mac[5] = io_inb(driver->io + RTL8139_REGISTER_IDR5);

}

static void attach(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;
    struct rtl8139_driver *driver = (struct rtl8139_driver *)device->driver;
    unsigned int bar0 = pci_device_ind(pciDevice, PCI_CONFIG_BAR0);
    unsigned int bar1 = pci_device_ind(pciDevice, PCI_CONFIG_BAR1);

    driver->io = bar0 & ~1;
    driver->mmio = bar1 & ~1;

    log_write_string("BAR0: 0x");
    log_write_number(bar0, 16);
    log_write_string("\n");
    log_write_string("BAR1: 0x");
    log_write_number(bar1, 16);
    log_write_string("\n");

    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inw(pciDevice, PCI_CONFIG_VENDOR) == RTL8139_PCI_VENDOR && pci_device_inw(pciDevice, PCI_CONFIG_DEVICE) == RTL8139_PCI_DEVICE;

}

static unsigned int receive(struct net_interface *self, unsigned int count, void *buffer)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;
    struct rtl8139_header *header;

    driver->rxp = io_inw(driver->io + RTL8139_REGISTER_CAPR) + 0x10;

    header = (struct rtl8139_header *)(driver->rx + driver->rxp);

    log_write_string("RXP: 0x");
    log_write_number(driver->rxp, 16);
    log_write_string(" ");
    log_write_string("HEADER F: 0x");
    log_write_number(header->flags, 16);
    log_write_string(" ");
    log_write_string("HEADER L: 0x");
    log_write_number(header->length, 16);
    log_write_string("\n");

    driver->rxp += (header->length + 4 + 3) & ~3;

    io_outw(driver->io + RTL8139_REGISTER_CAPR, driver->rxp - 0x10);

    return memory_read(buffer, count, driver->rx, 64, 0);

}

static unsigned int send(struct net_interface *self, unsigned int count, void *buffer)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;
    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (driver->txp)
    {

        case 0:

            memory_write(driver->tx0, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_REGISTER_TSD0, status);

            break;

        case 1:

            memory_write(driver->tx1, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_REGISTER_TSD1, status);

            break;

        case 2:

            memory_write(driver->tx2, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_REGISTER_TSD2, status);

            break;

        case 3:

            memory_write(driver->tx3, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_REGISTER_TSD3, status);

            break;

    }

    driver->txp++;
    driver->txp %= 4;

    return count;

}

void rtl8139_init_driver(struct rtl8139_driver *driver)
{

    memory_clear(driver, sizeof (struct rtl8139_driver));
    base_init_driver(&driver->base, "rtl8139", start, check, attach);
    net_init_interface(&driver->inet, &driver->base, receive, send);

}

