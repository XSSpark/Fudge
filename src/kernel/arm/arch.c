#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "reg.h"
#include "pic.h"
#include "uart.h"
#include "kmi.h"
#include "timer.h"
#include "lcd.h"
#include "arch.h"

#define ARM4_XRQ_RESET   0x00
#define ARM4_XRQ_UNDEF   0x01
#define ARM4_XRQ_SWINT   0x02
#define ARM4_XRQ_ABRTP   0x03
#define ARM4_XRQ_ABRTD   0x04
#define ARM4_XRQ_RESV1   0x05
#define ARM4_XRQ_IRQ     0x06
#define ARM4_XRQ_FIQ     0x07

extern unsigned int arch_x_swi;
extern unsigned int arch_x_irq;
extern unsigned int arch_x_fiq;

static unsigned int spawn(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int id;} *args = stack;

    if (args->ichannel && args->id)
    {

        unsigned int ntask = kernel_createtask();

        if (ntask)
        {

            return kernel_loadtask(ntask, ARCH_TASKSTACKVIRTUAL - 0x10, args->ichannel, args->id);

        }

    }

    DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    return 0;

}

void arch_swi(void)
{

    uart_puts("SWI\n");

}

void arch_irq(void)
{

    unsigned int status = reg_read32(0x14000000);

    uart_puts("IRQ\n");

    if (status == PIC_IRQ_UART0)
    {

        reg_write32(0x16000044, 1);
        uart_puts("  UART0\n");

    }

    if (status == PIC_IRQ_KEYBOARD)
    {

        unsigned int value = reg_read32(0x18000008);

        uart_puts("  KEYBOARD\n");

        if (value == 0x1C)
            uart_puts("  Enter was pressed\n");

    }

    if (status == PIC_IRQ_TIMER0)
    {

        reg_write32(0x1300000C, 1);
        uart_puts("  TIMER0\n");

    }

}

void arch_fiq(void)
{

    uart_puts("FIQ\n");

}

static void xrqinstall(unsigned int index, void *addr)
{

    unsigned int value = 0xEA000000 | (((unsigned int)addr - (8 + (4 * index))) >> 2);

    buffer_copy((void *)(index * 4), &value, 4);

}

void arch_setup2(void)
{

    unsigned int ntask = kernel_createtask();

    if (ntask)
    {

        kernel_loadtask(ntask, ARCH_TASKSTACKVIRTUAL - 0x10, 0, 0);
        kernel_place(0, ntask, EVENT_MAIN, 0, 0);

    }

    else
    {

        DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    }

    /*
    arch_leave();
    */

}

void arch_setup(void)
{

    xrqinstall(ARM4_XRQ_RESET, &arch_x_swi);
    xrqinstall(ARM4_XRQ_UNDEF, &arch_x_swi);
    xrqinstall(ARM4_XRQ_SWINT, &arch_x_swi);
    xrqinstall(ARM4_XRQ_ABRTP, &arch_x_swi);
    xrqinstall(ARM4_XRQ_ABRTD, &arch_x_swi);
    xrqinstall(ARM4_XRQ_RESV1, &arch_x_swi);
    xrqinstall(ARM4_XRQ_IRQ, &arch_x_irq);
    xrqinstall(ARM4_XRQ_FIQ, &arch_x_fiq);
    resource_setup();
    pic_setup();
    uart_setup();
    timer_setup();
    kmi_setup();
    lcd_setup();
    kernel_setup(ARCH_KERNELSTACKPHYSICAL, ARCH_KERNELSTACKSIZE, ARCH_MAILBOXPHYSICAL, ARCH_MAILBOXSIZE);
    abi_setup();
    abi_setcallback(0x0C, spawn);
    arch_setup2();

    for (;;);

}

