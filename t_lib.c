#include "t_lib.h"

tcb_t *running = NULL;

tcb_t *ready_queue_head = NULL;
tcb_t *ready_queue_tail = NULL;

void print_tcb (tcb_t *control_block) {
    if (control_block) {
        printf("\tTCB 0x%08X: {\n\t\tthread_id:       %d\n\t\tthread_priority: %d\n\t\tthread_context:  0x%08X\n\t\tnext:            0x%08X\n\t}\n", control_block, control_block->thread_id, control_block->thread_priority, control_block->thread_context, control_block->next);
    } else {
        printf("\tTCB 0x%08X: {\n\t\tthread_id:       NULL\n\t\tthread_priority: NULL\n\t\tthread_context:  NULL\n\t\tnext:            NULL\n\t}\n", control_block);
    }
}

void t_yield () {

    /*
    printf("\tYielding...\n");
    printf("\tRunning:\n");
    print_tcb(running);
    printf("\tReady Queue Head:\n");
    print_tcb(ready_queue_head);
    */

    ready_queue_tail->next = running;
    ready_queue_tail = ready_queue_tail->next;

    if (!ready_queue_head) {
        // printf("\tNo ready thread to yield to. Resetting context to running.\n");
        setcontext(running->thread_context);
    }

    running = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    running->next = NULL;

    /*
    printf("\tSwapped successfully.\n");
    printf("\tRunning:\n");
    print_tcb(running);
    printf("\tReady Queue Head:\n");
    print_tcb(ready_queue_head);
    printf("\tReady Queue Tail:\n");
    print_tcb(ready_queue_tail);
    */

    swapcontext(ready_queue_tail->thread_context, running->thread_context);

    // printf("\tYield complete!\n");
}

void t_init () {

    ucontext_t *tmp;
    tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
    getcontext(tmp);    /* let tmp be the context of main() */

    tcb_t *main = malloc(sizeof(tcb_t));
    main->thread_id = 0;
    main->thread_priority = 1;
    main->thread_context = tmp;
    main->next = NULL;

    running = main;

    // printf("\tInitialized main!\n");
    // print_tcb(main);
}

int t_create (void (*fct)(int), int id, int pri) {

    size_t sz = 0x10000;

    ucontext_t *uc;
    uc = (ucontext_t *) malloc(sizeof(ucontext_t));

    getcontext(uc);

    uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
    uc->uc_stack.ss_size = sz;
    uc->uc_stack.ss_flags = 0;
    uc->uc_link = running->thread_context; 
    makecontext(uc, (void (*)(void)) fct, 1, id);

    tcb_t *control_block = malloc(sizeof(tcb_t));
    control_block->thread_id = id;
    control_block->thread_priority = pri;
    control_block->thread_context = uc;
    control_block->next = NULL;

    if (!ready_queue_head) {
        ready_queue_head = control_block;
        ready_queue_tail = ready_queue_head;
    } else {
        ready_queue_tail->next = control_block;
        ready_queue_tail = ready_queue_tail->next;
    }

    // printf("\tCreated new TCB!\n");
    // print_tcb(control_block);
}

void t_terminate () {

    tcb_t *temp = running;
    running = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    running->next = NULL;

    free(temp->thread_context->uc_stack.ss_sp);
    free(temp->thread_context);
    free(temp);

    setcontext(running->thread_context);
}

void t_shutdown () {

    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    free(running);

    while (ready_queue_head) {
        tcb_t *temp = ready_queue_head->next;
        free(ready_queue_head->thread_context->uc_stack.ss_sp);
        free(ready_queue_head->thread_context);
        free(ready_queue_head);
        ready_queue_head = temp;
    }
}
