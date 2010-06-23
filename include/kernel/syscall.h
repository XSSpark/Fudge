#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_ROUTINES_SIZE 3

extern int syscall_write(char *text);
extern void syscall_handler(int num);
extern void syscall_init();

#endif

