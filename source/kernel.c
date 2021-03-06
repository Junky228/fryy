#include "kernel.h"

void sh();
void init();
static void set_timer(void (*scheduler)());
static void scheduler();

res_t * res;

void main()
{
    asm "mov ax, cs";
    asm "mov ss, ax";
    asm "mov sp, #0";
    task_init(init, 0x1000);
    res = res_init(2);
    set_timer(task_schedule);
    task_set(task_get());
    //fsdriver();
    while(1);
}

/* root task */
void init()
{
    task_init(shell, 0x1000);
    task_deinit(task_get());
}

void set_timer(void (*scheduler)())
{
    scheduler = scheduler;
    asm "push ds";
    asm "mov ax, #0";
    asm "mov ds, ax";
    asm "mov ax, 4[bp]";
    asm "mov word [0x08 * 4], ax";
    asm "mov word [0x08 * 4 + 2], cs";
    asm "pop ds";
}
