#define ARCH_KERNELSTACKADDRESS         0x00880000
#define ARCH_KERNELSTACKSIZE            0x00008000
#define ARCH_MAILBOXADDRESS             0x00900000
#define ARCH_MAILBOXSIZE                0x00001000

void arch_syscall(void *stack);
void arch_irq(void *stack);
void arch_fiq(void);
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_setup(void);
