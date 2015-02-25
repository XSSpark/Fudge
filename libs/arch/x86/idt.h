#define IDT_FLAG_TYPE32TASK             0x05
#define IDT_FLAG_TYPE16INT              0x06
#define IDT_FLAG_TYPE16TRAP             0x07
#define IDT_FLAG_TYPE32INT              0x0E
#define IDT_FLAG_TYPE32TRAP             0x0F
#define IDT_FLAG_STORAGE                0x10
#define IDT_FLAG_RING1                  0x20
#define IDT_FLAG_RING2                  0x40
#define IDT_FLAG_RING3                  0x60
#define IDT_FLAG_PRESENT                0x80
#define IDT_INDEX_DE                    0x00
#define IDT_INDEX_DB                    0x01
#define IDT_INDEX_NI                    0x02
#define IDT_INDEX_BP                    0x03
#define IDT_INDEX_OF                    0x04
#define IDT_INDEX_BR                    0x05
#define IDT_INDEX_UD                    0x06
#define IDT_INDEX_NM                    0x07
#define IDT_INDEX_DF                    0x08
#define IDT_INDEX_CO                    0x09
#define IDT_INDEX_TS                    0x0A
#define IDT_INDEX_NP                    0x0B
#define IDT_INDEX_SS                    0x0C
#define IDT_INDEX_GF                    0x0D
#define IDT_INDEX_PF                    0x0E
#define IDT_INDEX_MF                    0x10
#define IDT_INDEX_AC                    0x11
#define IDT_INDEX_MC                    0x12
#define IDT_INDEX_XM                    0x13
#define IDT_INDEX_SYSCALL               0x80

struct idt_descriptor
{

    unsigned char base0;
    unsigned char base1;
    unsigned char selector0;
    unsigned char selector1;
    unsigned char zero;
    unsigned char flags;
    unsigned char base2;
    unsigned char base3;

};

struct idt_pointer
{

    unsigned char limit0;
    unsigned char limit1;
    unsigned char base0;
    unsigned char base1;
    unsigned char base2;
    unsigned char base3;

};

void idt_setdescriptor(struct idt_pointer *pointer, unsigned char index, void (*callback)(), unsigned short selector, unsigned char flags);
void idt_initpointer(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors);
