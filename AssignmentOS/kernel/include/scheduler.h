#ifndef _SCHEDULER_H
#define _SCHEDULER_H
#include <stdint.h>
#include <stddef.h>
#include "constants.h"
#include "syscalls.h"
#include "kernel.h"
#include "tty.h"

enum Event{NONBLOCKED, KEYBOARD, WAIT_CHILD, WAIT_SEMAPHORE, WAIT_IRQ6};
enum TaskStatus{RUNNING, READY, BLOCKED, ZOMBIE};
typedef void (*signal_handler_t)();

struct TSS_t
{
    uint32_t prev:  16; uint32_t:16;
    uint32_t esp0;
    uint32_t ss0:   16; uint32_t:16;
    uint32_t esp1;
    uint32_t ss1:   16; uint32_t:16;
    uint32_t esp2;
    uint32_t ss2:   16; uint32_t:16;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es:    16; uint32_t:16;
    uint32_t cs:    16; uint32_t:16;
    uint32_t ss:    16; uint32_t:16;
    uint32_t ds:    16; uint32_t:16;
    uint32_t fs:    16; uint32_t:16;
    uint32_t gs:    16; uint32_t:16;
    uint32_t ldtss: 16; uint32_t:16;
    uint32_t:16;        uint32_t iomap_addr: 16;
}__attribute__((packed));

struct Task_t
{
    struct TSS_t tss;
    struct TTY_t *tty;
    uint16_t pid, ppid;  /* PID is also the task segment selector divided by 8 */
    enum TaskStatus status;
    enum Event block_event;
    int exit_code;
    uint32_t *page_dir;
    struct Task_t *next;
    _sem_t sem_list[SEM_PER_PROCESS];
    size_t sem_list_size;
    signal_handler_t sigint_handler;
    uint8_t io_buf[512];
    int io_buf_idx;
    bool io_buf_valid;
    char working_dir[65];
};

struct TaskQueue_t
{
    struct Task_t *head, *tail;
    volatile size_t size;
};

extern struct Task_t *CURRENT_TASK, *FOREGROUND_TASK, *task_list[MAX_PROCESS_NUM];

void init_scheduler();
void start_scheduler();
void add_to_task_queue(struct TaskQueue_t *q, struct Task_t *t);
void force_switch();
void scheduler();
void pwait();
void wait_event(enum Event e);
void wait_child(uint16_t cpid);
void wake_up_event(enum Event e);
void wake_up_pid(uint16_t pid);
int sys_exit(int code, int _1, int _2);
uint16_t _sys_fork();
int sys_wait(int pid, int _1, int _2);
int sys_execve(int _filename, int _argv, int _envp);
int sys_getpid(int _1, int _2, int _3);
int sys_getppid(int _1, int _2, int _3);

void init_task(struct Task_t *t, uint16_t cs, uint16_t ds, uint16_t ss);
void add_task(struct Task_t *t);

void sigint_default();

#define TASK_SWITCH() farjmp(0, 56)
#endif
