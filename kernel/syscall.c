#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

static unsigned int (*routines[SYSCALL_ROUTINE_SLOTS])(struct runtime_task *task, unsigned int stack);

static unsigned int attach(struct runtime_task *task, unsigned int index, unsigned int callback)
{

    if (!callback)
        return 0;

    return event_register_routine(index, task, callback);

}

static unsigned int close(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem)
        return 0;

    if (descriptor->filesystem->close)
        descriptor->filesystem->close(descriptor->filesystem, descriptor->id);

    runtime_descriptor_init(descriptor, 0, 0, 0);

    return 1;

}

static unsigned int detach(struct runtime_task *task, unsigned int index)
{

    return event_unregister_routine(index, task);

}

static unsigned int halt(struct runtime_task *task)
{

    kernel_enable_interrupts();
    kernel_halt();

    return 1;

}

static unsigned int execute(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    unsigned int slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    struct runtime_task *ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, task, slot);
    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.paddress, ntask->memory.size);
    mmu_load_memory(ntask->id);

    unsigned int count = descriptor->filesystem->read(descriptor->filesystem, descriptor->id, 0, ntask->memory.size, (void *)ntask->memory.paddress);

    if (!count)
        return 0;

    struct elf_header *header = elf_get_header((void *)ntask->memory.paddress);

    ntask->memory.vaddress = elf_get_virtual(header);

    if (!ntask->memory.vaddress)
        return 0;

    ntask->used = 1;
    ntask->parentid = task->id;

    runtime_registers_init(&ntask->registers, header->entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);

    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.vaddress, ntask->memory.size);
    mmu_load_memory(ntask->id);

    runtime_set_running_task(ntask);

    return slot;

}

static unsigned int exit(struct runtime_task *task)
{

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    task->used = 0;

    mmu_load_memory(ptask->id);
    runtime_set_running_task(ptask);

    return ptask->id;

}

static unsigned int load(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    void *physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    struct elf_header *header = elf_get_header(physical);

    if (!header)
        return 0;

    elf_relocate(header);

    void (*init)() = (void (*)())elf_get_symbol(header, "init");

    if (!init)
        return 0;

    init();

    return 1;

}

static unsigned int open(struct runtime_task *task, unsigned int index, char *path)
{

    if (!path)
        return 0;

    unsigned int slot = (index) ? index : task->get_descriptor_slot(task);

    if (!slot)
        return 0;

    struct runtime_descriptor *descriptor = task->get_descriptor(task, slot);

    if (!descriptor)
        return 0;

    struct modules_filesystem *filesystem = modules_get_filesystem(path);

    if (!filesystem)
        return 0;

    unsigned int id = filesystem->find(filesystem, path + string_length(filesystem->path));

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, filesystem, 0);

    if (descriptor->filesystem->open)
        descriptor->filesystem->open(descriptor->filesystem, descriptor->id);

    return slot;

}

static unsigned int read(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    return descriptor->filesystem->read(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

static unsigned int reboot(struct runtime_task *task)
{

    return 1;

}

static unsigned int unload(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    void *physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    struct elf_header *header = elf_get_header(physical);

    if (!header)
        return 0;

    void (*destroy)() = (void (*)())elf_get_symbol(header, "destroy");

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

static unsigned int wait(struct runtime_task *task)
{

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    task->event = 0;

    mmu_load_memory(ptask->id);
    runtime_set_running_task(ptask);

    return ptask->id;

}

static unsigned int write(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->write)
        return 0;

    return descriptor->filesystem->write(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

static unsigned int handle_attach(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    unsigned int callback = *(unsigned int *)(stack + 8);

    return attach(task, index, callback);

}

static unsigned int handle_close(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return close(task, index);

}

static unsigned int handle_detach(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return detach(task, index);

}

static unsigned int handle_halt(struct runtime_task *task, unsigned int stack)
{

    return halt(task);

}

static unsigned int handle_execute(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return execute(task, index);

}

static unsigned int handle_exit(struct runtime_task *task, unsigned int stack)
{

    return exit(task);

}

static unsigned int handle_load(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return load(task, index);

}

static unsigned int handle_open(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *path = *(char **)(stack + 8);

    return open(task, index, path);

}

static unsigned int handle_read(struct runtime_task *task, unsigned int stack)
{

    unsigned int id = *(unsigned int *)(stack + 4);
    unsigned int offset = *(unsigned int *)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);
    char *buffer = *(char **)(stack + 16);

    return read(task, id, offset, count, buffer);

}

static unsigned int handle_reboot(struct runtime_task *task, unsigned int stack)
{

    return reboot(task);

}

static unsigned int handle_unload(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return unload(task, index);

}

static unsigned int handle_wait(struct runtime_task *task, unsigned int stack)
{

    return wait(task);

}

static unsigned int handle_write(struct runtime_task *task, unsigned int stack)
{

    unsigned int id = *(unsigned int *)(stack + 4);
    unsigned int offset = *(unsigned int *)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);
    char *buffer = *(char **)(stack + 16);

    return write(task, id, offset, count, buffer);

}

static void syscall_register_routine(unsigned char index, unsigned int (*routine)(struct runtime_task *task, unsigned int stack))
{

    routines[index] = routine;

}

unsigned int syscall_raise(unsigned int index, struct runtime_task *task)
{

    unsigned int (*routine)(struct runtime_task *task, unsigned int stack) = routines[index];

    if (!routine)
        return 0;

    return routine(task, task->registers.sp);

}

struct runtime_task *syscall_execute(char *path)
{

    unsigned int slot = runtime_get_task_slot();
    struct runtime_task *task = runtime_get_task(slot);

    runtime_task_init(task, slot);
    unsigned int id = open(task, 0, path);

    unsigned int index = execute(task, id);

    if (!index)
        return 0;

    return runtime_get_task(index);

}

void syscall_init()
{

    syscall_register_routine(SYSCALL_ROUTINE_OPEN, handle_open);
    syscall_register_routine(SYSCALL_ROUTINE_CLOSE, handle_close);
    syscall_register_routine(SYSCALL_ROUTINE_READ, handle_read);
    syscall_register_routine(SYSCALL_ROUTINE_WRITE, handle_write);
    syscall_register_routine(SYSCALL_ROUTINE_EXECUTE, handle_execute);
    syscall_register_routine(SYSCALL_ROUTINE_EXIT, handle_exit);
    syscall_register_routine(SYSCALL_ROUTINE_WAIT, handle_wait);
    syscall_register_routine(SYSCALL_ROUTINE_LOAD, handle_load);
    syscall_register_routine(SYSCALL_ROUTINE_UNLOAD, handle_unload);
    syscall_register_routine(SYSCALL_ROUTINE_HALT, handle_halt);
    syscall_register_routine(SYSCALL_ROUTINE_REBOOT, handle_reboot);
    syscall_register_routine(SYSCALL_ROUTINE_ATTACH, handle_attach);
    syscall_register_routine(SYSCALL_ROUTINE_DETACH, handle_detach);

}

