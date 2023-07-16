#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include <modules/arch/x86/acpi/acpi.h>
#include "apic.h"

#define ROUTINES                        256
#define MSR_APIC                        0x1B

static struct {unsigned int detected;} lapics[256];
static struct {unsigned int detected; unsigned int address; unsigned int intbase;} ioapics[256];
static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTPHYSICAL;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTPHYSICAL;
static void (*routines[ROUTINES])(unsigned int irq);
static unsigned int overrides[16];
static unsigned int mmio;

static volatile unsigned int readio(unsigned int base, unsigned int offset)
{

    *(volatile unsigned int *)(base) = offset;

    return *(volatile unsigned int *)(base + 0x10);

}

static void writeio(unsigned int base, unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(base) = offset;
    *(volatile unsigned int *)(base + 0x10) = value;

}

void apic_debug_ioapic(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
        unsigned int madtend = (unsigned int)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
                unsigned int ioapicid = readio(ioapic->address, 0);
                unsigned int ioapicversion = readio(ioapic->address, 1);
                unsigned int id = ((ioapicid >> 24) & 0x0F);
                unsigned int version = ioapicversion & 0xFF;
                unsigned int max = ((ioapicversion >> 16) & 0xFF) + 1;
                unsigned int j;

                DEBUG_FMT3(DEBUG_INFO, "ioapic id %u version %u max %u", &id, &version, &max);

                for (j = 0; j < max; j++)
                {

                    unsigned int value0 = readio(ioapic->address, 0x10 + j * 2);
                    unsigned int value1 = readio(ioapic->address, 0x10 + j * 2 + 1);

                    DEBUG_FMT3(DEBUG_INFO, "ioapic redtbl %u %H8u:%H8u", &j, &value1, &value0);

                }

            }

            madttable += entry->length;

        }

    }

}

void apic_debug(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
        unsigned int madtend = (unsigned int)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 0)
            {

                struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

                DEBUG_FMT3(DEBUG_INFO, "lapic processor %c id %c flags 0x%8Hu", &lapic->processor, &lapic->id, &lapic->flags);

            }

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;

                DEBUG_FMT3(DEBUG_INFO, "ioapic id %c address 0x%8Hu intbase %u", &ioapic->id, &ioapic->address, &ioapic->intbase);

            }

            if (entry->type == 2)
            {

                struct acpi_madt_ioapic_iso *override = (struct acpi_madt_ioapic_iso *)entry;

                DEBUG_FMT4(DEBUG_INFO, "ioapic override bus %c irq %c gsi %u flags %4Hh", &override->bus, &override->irq, &override->intbase, &override->flags);

            }

            if (entry->type == 4)
            {

                struct acpi_madt_lapic_nmi *nmi = (struct acpi_madt_lapic_nmi *)entry;

                DEBUG_FMT1(DEBUG_INFO, "lapic nmi processor %c", &nmi->processor);

            }

            madttable += entry->length;

        }

    }

}

void apic_setupisrs(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
        unsigned int madtend = (unsigned int)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;

                idt_setdescriptor(&idt->pointer, 0x61, apic_isr01, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x62, apic_isr02, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x63, apic_isr03, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x64, apic_isr04, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x68, apic_isr08, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x6C, apic_isr0C, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                idt_setdescriptor(&idt->pointer, 0x6E, apic_isr0E, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                writeio(ioapic->address, 0x10 + 0x01 * 2, 0x61);
                writeio(ioapic->address, 0x10 + 0x02 * 2, 0x62);
                writeio(ioapic->address, 0x10 + 0x03 * 2, 0x63);
                writeio(ioapic->address, 0x10 + 0x04 * 2, 0x64);
                writeio(ioapic->address, 0x10 + 0x08 * 2, 0x68);
                writeio(ioapic->address, 0x10 + 0x0C * 2, 0x6C);
                writeio(ioapic->address, 0x10 + 0x0E * 2, 0x6E);

            }

            if (entry->type == 2)
            {

                struct acpi_madt_ioapic_iso *override = (struct acpi_madt_ioapic_iso *)entry;

                if (override->intbase < 16)
                    overrides[override->intbase] = override->irq;

            }

            madttable += entry->length;

        }

    }

    idt_setdescriptor(&idt->pointer, 0xFE, apic_test, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0xFF, apic_spurious, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

static void detect(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
        unsigned int madtend = (unsigned int)madt + madt->base.length;

        if (madt->flags & 0x01)
        {

            /* pic_disable(); */

        }

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 0)
            {

                struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

                if ((lapic->flags & 0x01) || (lapic->flags & 0x02))
                {

                    lapics[lapic->id].detected = 1;

                }

            }

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;

                ioapics[ioapic->id].detected = 1;
                ioapics[ioapic->id].address = ioapic->address;
                ioapics[ioapic->id].intbase = ioapic->intbase;

            }

            if (entry->type == 2)
            {

                /* override io apic */

            }

            if (entry->type == 4)
            {

                /* local apic nmi */

            }

            madttable += entry->length;

        }

    }

}

static unsigned int apic_ind(unsigned int reg)
{

    volatile unsigned int *address = (volatile unsigned int *)(mmio + reg);

    return *address;

}

static void apic_outd(unsigned int reg, unsigned int value)
{

    volatile unsigned int *address = (volatile unsigned int *)(mmio + reg);

    *address = value;

}

unsigned int apic_checklapic(unsigned int id)
{

    return lapics[id].detected;

}

unsigned int apic_checkioapic(unsigned int id)
{

    return ioapics[id].detected;

}

/* TODO: Remove changing of CR3 register. This is a workaround because existing tasks cant access apic mapped register. */
unsigned int apic_getid(void)
{

    unsigned int directory = cpu_getcr3();
    unsigned int id;

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

    id = apic_ind(APIC_REG_ID) >> 24;

    cpu_setcr3(directory);

    return id;

}

unsigned short apic_interrupt(struct cpu_general general, unsigned int index, struct cpu_interrupt interrupt)
{

    unsigned int gsi = overrides[index];

    if (routines[gsi])
        routines[gsi](gsi);

    apic_outd(APIC_REG_EOI, 0);

    return arch_resume(&general, &interrupt);

}

unsigned short apic_interruptnone(struct cpu_general general, struct cpu_interrupt interrupt)
{

    apic_outd(APIC_REG_EOI, 0);

    return arch_resume(&general, &interrupt);

}

unsigned int apic_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    routines[irq] = routine;

    return 1;

}

unsigned int apic_unsetroutine(unsigned int irq)
{

    routines[irq] = 0;

    return 1;

}

void apic_setup_ap(void)
{

    apic_outd(APIC_REG_TPR, 0x00000000);
    apic_outd(APIC_REG_LDR, 0x00000000);
    apic_outd(APIC_REG_DFR, 0xFFFFFFFF);
    apic_outd(APIC_REG_SV, (1 << 8) | 0xFF);
    apic_outd(APIC_REG_LVTCMCI, 0x00010000);
    apic_outd(APIC_REG_LVTTIMER, 0x00010000);
    apic_outd(APIC_REG_LVTPERF, 0x00010000);
    apic_outd(APIC_REG_LVTLINT0, 0x00010000);
    apic_outd(APIC_REG_LVTLINT1, 0x00010000);
    apic_outd(APIC_REG_LVTERROR, 0x00010000);

}

void apic_setup_bp(void)
{

    apic_outd(APIC_REG_TPR, 0x00000000);
    apic_outd(APIC_REG_LDR, 0x00000000);
    apic_outd(APIC_REG_DFR, 0xFFFFFFFF);
    apic_outd(APIC_REG_SV, (1 << 8) | 0xFF);
    apic_outd(APIC_REG_LVTCMCI, 0x00010000);
    apic_outd(APIC_REG_LVTTIMER, 0x00010000);
    apic_outd(APIC_REG_LVTPERF, 0x00010000);
    apic_outd(APIC_REG_LVTLINT0, 0x00000700);
    apic_outd(APIC_REG_LVTLINT1, 0x00000400);
    apic_outd(APIC_REG_LVTERROR, 0x00010000);

}

void apic_sendint(unsigned int id, unsigned int value)
{

    unsigned int directory = cpu_getcr3();

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

    apic_outd(APIC_REG_ICR1, id << 24);
    apic_outd(APIC_REG_ICR0, value);

    while (apic_ind(APIC_REG_ICR0) & APIC_REG_ICR_STATUS_PENDING);

    cpu_setcr3(directory);

}

void module_init(void)
{

    struct cpuid_data data;
    struct msr_data msrdata;
    unsigned int i;

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    for (i = 0; i < 16; i++)
        overrides[i] = i;

/*
    if ((data.ecx & CPUID_FEATURES00_2XAPIC))
    {


    }
*/

    if ((data.edx & CPUID_FEATURES01_APIC))
    {

        msr_get(MSR_APIC, &msrdata);

        mmio = (msrdata.eax & 0xFFFFF000);

        arch_mapuncached(7, 0xFEC00000, 0xFEC00000, 0x400000);
        detect();

    }

}

