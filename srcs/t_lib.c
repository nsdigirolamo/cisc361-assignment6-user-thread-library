#include <signal.h>
#include "t_lib.h"

#define IS_DEBUGGING 0

tcb_t *running = NULL;

tcb_t *ready_queue_head = NULL;
tcb_t *ready_queue_tail = NULL;

void print_ready_queue () {
    printf("\tReady Queue: ");
    if (ready_queue_head) {
        tcb_t *temp = ready_queue_head;
        int head_id = ready_queue_head->thread_id;
        int loop_flag = 1;
        while (temp) {
            printf("%d -> ", temp->thread_id);
            if (temp->thread_id == head_id) { 
                loop_flag--;
                if (loop_flag < 0) { break; }
            }
            temp = temp->next;
        }
        if (loop_flag < 0) {
            printf("LOOP\n", head_id);
        } else {
            printf("NULL\n");
        }
    } else {
        printf("NULL\n");
    }
}

void print_tcb (tcb_t *control_block) {
    if (control_block) {
        printf("\tTCB 0x%08X: {\n\t\tthread_id:       %d\n\t\tthread_priority: %d\n\t\tthread_context:  0x%08X\n\t\tnext:            0x%08X\n\t}\n", control_block, control_block->thread_id, control_block->thread_priority, control_block->thread_context, control_block->next);
    } else {
        printf("\tTCB 0x%08X: {\n\t\tthread_id:       NULL\n\t\tthread_priority: NULL\n\t\tthread_context:  NULL\n\t\tnext:            NULL\n\t}\n", control_block);
    }
}

void t_yield () {

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tYield start!\n");
        printf("\tRunning: %d\n", running->thread_id);
        print_ready_queue();
    }

    if (!ready_queue_head) {
        if (IS_DEBUGGING) { printf("\tNo ready thread to yield to. Resetting context to running.\n"); }
        setcontext(running->thread_context);
    }

    ready_queue_tail->next = running;
    ready_queue_tail = ready_queue_tail->next;

    running = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    running->next = NULL;

    if (IS_DEBUGGING) {
        printf("\tSwapped successfully.\n");
        printf("\tRunning: %d\n", running->thread_id);
        print_ready_queue();
        printf("\tYield complete!\n");
        printf("\t-------------------------------------------------------\n");
    }

    swapcontext(ready_queue_tail->thread_context, running->thread_context);
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

    if (IS_DEBUGGING) { 
        printf("\tInitialized main!\n");
        print_tcb(main);
    }
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

    if (IS_DEBUGGING) { 
        printf("\tCreated new TCB!\n");
        print_tcb(control_block);
    }
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

void print_sem (sem_t *semaphore) {
    if (semaphore) {
        printf("\tSemaphore 0x%08X: {\n\t\tCount: %d\n\t\tQueue: ", semaphore, semaphore->count);
        if (semaphore->queue) {
            tcb_t *temp = semaphore->queue;
            int head_id = semaphore->queue->thread_id;
            int loop_flag = 1;
            while (temp) {
                printf("%d -> ", temp->thread_id);
                if (temp->thread_id == head_id) { 
                    loop_flag--;
                    if (loop_flag < 0) { break; }
                }
                temp = temp->next;
            }
            if (loop_flag < 0) {
                printf("LOOP\n\t}\n");
            } else {
                printf("NULL\n\t}\n");
            }
        } else {
            printf("NULL\n\t}\n");
        }
    }
}

int sem_init(sem_t **sp, unsigned int count) {
    
    *sp = (sem_t *) malloc(sizeof(sem_t));
    (*sp)->count = count;
    (*sp)->queue = NULL;
    
    if (IS_DEBUGGING) {
        printf("\tInitialized the following semaphore:\n");
        print_sem(*sp);
    }

    return 0;
}

void sem_wait(sem_t *sp) {
    sighold(SIGALRM);

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tWait called on the following semaphore:\n");
        print_sem(sp);
    }

    sp->count--;

    if (IS_DEBUGGING) { printf("\tCount of semaphore is now %d.\n", sp->count); }

    if (sp->count < 0) {

        if (IS_DEBUGGING) {
            printf("\tRunning thread %d added to semaphore's queue. Now yielding...\n", running->thread_id);
            printf("\tYield start!\n");
            printf("\tRunning: %d\n", running->thread_id);
            print_ready_queue();
        }
        
        if (!ready_queue_head) {
            if (IS_DEBUGGING) { printf("\tNo ready thread to yield to. Resetting context to running.\n"); }
            setcontext(running->thread_context);
        }

        if (sp->queue) {
            tcb_t *temp = sp->queue;
            sp->queue = running;
            sp->queue->next = temp;
        } else {
            sp->queue = running;
        }

        running = ready_queue_head;
        ready_queue_head = ready_queue_head->next;
        running->next = NULL;

        if (IS_DEBUGGING) {
            printf("\tSwapped successfully.\n");
            printf("\tRunning: %d\n", running->thread_id);
            print_ready_queue();
            printf("\tYield complete!\n");
            printf("\t-------------------------------------------------------\n");
        }

        swapcontext(sp->queue->thread_context, running->thread_context);
    }

    sigrelse(SIGALRM);
}

void sem_signal(sem_t *sp) {
    sighold(SIGALRM);

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tSignal called on the following semaphore:\n");
        print_sem(sp);
    }

    sp->count++;

    if (IS_DEBUGGING) { printf("\tCount increased to %d\n", sp->count); }

    if (sp->queue) {

        if (IS_DEBUGGING) { 
            printf("\tThis semaphore has a queue!\n", sp->queue); 
            print_ready_queue();
        }

        tcb_t *tail_prev = NULL;
        tcb_t *tail = sp->queue;
        while (tail->next) {
            tail_prev = tail;
            tail = tail->next;
        }
        if (tail_prev) {
            tail_prev->next = NULL;
        } else {
            sp->queue = NULL;
        }

        if (ready_queue_head) {
            ready_queue_tail->next = tail;
            ready_queue_tail = ready_queue_tail->next;
        } else {
            ready_queue_head = tail;
            ready_queue_tail = tail;
        }

        if (IS_DEBUGGING) {
            printf("\tMoved %d from the semaphore queue to the ready queue!\n", ready_queue_tail->thread_id);
            print_ready_queue();
            printf("\t-------------------------------------------------------\n");
        }
    }

    sigrelse(SIGALRM);
}

void sem_destroy(sem_t **sp) {

    while ((*sp)->queue) {
        tcb_t *temp = (*sp)->queue->next;
        free((*sp)->queue->thread_context->uc_stack.ss_sp);
        free((*sp)->queue->thread_context);
        free((*sp)->queue);
        (*sp)->queue = temp;
    }

    free((*sp));
}

int mbox_create(mbox **mb) {
    return 0;
}

void mbox_destroy(mbox **mb) {

}

void mbox_deposit(mbox *mb, char *msg, int len) {

}

void mbox_withdraw(mbox *mb, char *msg, int *len) {

}

void send(int tid, char *msg, int len) {

}

void receive(int *tid, char *msg, int *len) {

}