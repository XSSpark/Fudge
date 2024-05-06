#define ARCH_KERNELSTACKPHYSICAL        0x00880000
#define ARCH_KERNELSTACKSIZE            0x00008000
#define ARCH_MAILBOXPHYSICAL            0x00900000
#define ARCH_MAILBOXSIZE                0x00001000
#define ARCH_TASKSTACKSIZE              0x00008000
#define ARCH_TASKSTACKVIRTUAL           0x80000000

unsigned int arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt);
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_setup(void);
