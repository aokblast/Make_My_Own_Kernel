#ifndef __TASK_H__

#define __TASK_H__

#include "lib.h"
#include "memory.h"
#include "cpu.h"
#include "ptrace.h"

#define STACK_SIZE 32768

#define TASK_RUNNING		(1 << 0)
#define TASK_INTERRUPTIBLE	(1 << 1)
#define	TASK_UNINTERRUPTIBLE	(1 << 2)
#define	TASK_ZOMBIE		(1 << 3)	
#define	TASK_STOPPED		(1 << 4)

#define PF_KTHREAD	(1 << 0)

#define KERNEL_CS 	(0x08)
#define	KERNEL_DS 	(0x10)

#define	USER_CS		(0x28)
#define USER_DS		(0x30)

#define CLONE_FS	(1 << 0)
#define CLONE_FILES	(1 << 1)
#define CLONE_SIGNAL	(1 << 2)

extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;

extern unsigned long _stack_start;
extern void ret_from_intr();

struct task_struct{
    struct List list;
    volatile long state;
    unsigned long flags;

    struct mm_struct *mm;
    struct thread_struct *thread;

    unsigned long addr_limit;

    long pid;
    long counter;
    long signal;
    long priority;
};

struct mm_struct{
    pml4t_t *pgd;

    unsigned long startCode, endCode;
    unsigned long startData, endData;
    unsigned long startRodata, endRodata;
    unsigned long startBrk, endBrk;
    unsigned long startStack;
};

struct thread_struct{
    unsigned long rsp0;
    
    unsigned long rip;
    unsigned long rsp;

    unsigned long fs;
    unsigned long gs;

    unsigned long cr2;
    unsigned long trapNr;
    unsigned long errorCode;
};

struct tss_struct{
    unsigned int reserv0;
    unsigned long rsp0;
    unsigned long rsp1;
    unsigned long rsp2;
    unsigned long reserv1;
    unsigned long ist0;
    unsigned long ist1;
    unsigned long ist2;
    unsigned long ist3;
    unsigned long ist4;
    unsigned long ist5;
    unsigned long ist6;
    unsigned long ist7;
    unsigned long reserv2;
    unsigned short reserv3;
    unsigned short ioMap;
}__attribute__((packed));

union task_union{
    struct task_struct task;
    unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
}__attribute__((aligned (8)));

#define INIT_TASK(tsk) { \
    .state = TASK_UNINTERRUPTIBLE, \
    .flags = PF_KTHREAD, \
    .mm = &init_mm, \
    .thread = &init_thread, \
    .pid = 0, \
    .counter = 1, \
    .signal = 0, \
    .priority = 0 \
}

struct mm_struct init_mm = {0};
struct thread_struct init_thread;

union task_union init_task_union __attribute__((__section__ (".data.init_task"))) = {INIT_TASK(init_task_union.task)};

struct task_struct *init_task[NR_CPUS] = {&(init_task_union.task), 0};

struct thread_struct init_thread = {
    .rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
    .rsp = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
    .fs = KERNEL_DS,
    .gs = KERNEL_DS,
    .cr2 = 0,
    .trapNr = 0,
    .errorCode = 0
};

#define INIT_TSS { \
    .reserv0 = 0, \
    .rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
    .rsp1 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
    .rsp2 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
    .reserv1 = 0, \
    .ist1 = 0xffff800000007c00, \
    .ist2 = 0xffff800000007c00, \
    .ist3 = 0xffff800000007c00, \
    .ist4 = 0xffff800000007c00, \
    .ist5 = 0xffff800000007c00, \
    .ist6 = 0xffff800000007c00, \
    .ist7 = 0xffff800000007c00, \
    .reserv2 = 0, \
    .reserv3 = 0, \
    .ioMap = 0 \
}


struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};


static inline struct task_struct *get_current(){
    struct task_struct *res;
    __asm__ __volatile("andq %%rsp, %0 \n\t": "=r"(res) : "0"(~32767UL));
    return res;
}

#define current get_current()

#define GET_CURRENT \
    "movq %rsp, %rbx \n\t" \
    "andq $-32768, %rbx"

#define switch_to(prev, next) \
do{ \
    __asm__ __volatile__("pushq %%rbp \n\t" \
                         "pushq %%rax \n\t" \
                         "movq %%rsp, %0\n\t" \
                         "movq %2, %%rsp \n\t" \
                         "leaq 1f(%%rip), %%rax \n\t" \
                         "movq %%rax, %1 \n\t" \
                         "pushq %3 \n\t" \
                         "jmp __switch_to \n\t" \
                         "1: \n\t" \
                         "popq %%rax \n\t" \
                         "popq %%rbp \n\t" \
                         :"=m"(prev->thread->rsp), "=m"(prev->thread->rip) \
                         :"m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next) \
                         :"memory" \
    );  \
}while(0)

unsigned long do_fork(struct pt_regs * regs, unsigned long cloneFlags, unsigned long stackStart, unsigned long stackSize);
void task_init();

#endif
