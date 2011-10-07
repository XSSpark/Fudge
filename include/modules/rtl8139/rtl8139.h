#ifndef MODULES_RTL8139_H
#define MODULES_RTL8139_H

#define RTL8139_REGISTER_MAC0 0x00
#define RTL8139_REGISTER_MAC1 0x01
#define RTL8139_REGISTER_MAC2 0x02
#define RTL8139_REGISTER_MAC3 0x03
#define RTL8139_REGISTER_MAC4 0x04
#define RTL8139_REGISTER_MAC5 0x05
#define RTL8139_REGISTER_MAR0 0x08
#define RTL8139_REGISTER_MAR1 0x09
#define RTL8139_REGISTER_MAR2 0x0A
#define RTL8139_REGISTER_MAR3 0x0B
#define RTL8139_REGISTER_MAR4 0x0C
#define RTL8139_REGISTER_MAR5 0x0D
#define RTL8139_REGISTER_MAR6 0x0E
#define RTL8139_REGISTER_MAR7 0x0F
#define RTL8139_REGISTER_RBSTART 0x30
#define RTL8139_REGISTER_CMD 0x37
#define RTL8139_REGISTER_IMR 0x3C
#define RTL8139_REGISTER_ISR 0x3E
#define RTL8139_REGISTER_CONFIG0 0x51
#define RTL8139_REGISTER_CONFIG1 0x52

struct rtl8139_driver
{

    struct modules_driver base;
    unsigned short io; 
    char rx[8192+16];

};

extern void rtl8139_init();

#endif

