#include <signal.h>
#include <string.h>
#include "t_lib.h"

int mbox_create(mbox **mb);
void mbox_destroy(mbox **mb);

#define IS_DEBUGGING 1

tcb_t *running = NULL;

tcb_t *ready_queue_head = NULL;
tcb_t *ready_queue_tail = NULL;

tcb_lln_t *thread_list = NULL;

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
        printf("\tTCB 0x%08X: {\n", control_block);
        printf("\t\tTID: %d\n", control_block->thread_id);
        printf("\t\tPriority: %d\n", control_block->thread_priority);
        printf("\t\tThread Context: 0x%08X\n", control_block->thread_context);
        if (control_block->next) {
            printf("\t\tNext: 0x%08X\n", control_block->next);
        } else {
            printf("\t\tNext: NULL\n");
        }
        
        printf("\t}\n");
    } else {
        printf("\tTCB 0x%08X: NULL\n", control_block);
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

    mbox *mb;
    mbox_create(&mb);

    tcb_t *main = malloc(sizeof(tcb_t));
    main->thread_id = 0;
    main->thread_priority = 1;
    main->thread_context = tmp;
    main->mb = mb;
    main->next = NULL;

    tcb_lln_t *node = malloc(sizeof(tcb_lln_t));
    node->tcb = main;
    node->next = NULL;
    thread_list = node;

    running = main;

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tInitialized main!\n");
        print_tcb(main);
        printf("\t-------------------------------------------------------\n");
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

    mbox *mb;
    mbox_create(&mb);

    tcb_t *control_block = malloc(sizeof(tcb_t));
    control_block->thread_id = id;
    control_block->thread_priority = pri;
    control_block->thread_context = uc;
    control_block->mb = mb;
    control_block->next = NULL;

    tcb_lln_t *node = malloc(sizeof(tcb_lln_t));
    node->tcb = control_block;
    node->next = thread_list;
    thread_list = node;

    if (!ready_queue_head) {
        ready_queue_head = control_block;
        ready_queue_tail = ready_queue_head;
    } else {
        ready_queue_tail->next = control_block;
        ready_queue_tail = ready_queue_tail->next;
    }

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tCreated new TCB!\n");
        print_tcb(control_block);
        printf("\t-------------------------------------------------------\n");
    }
}

void t_terminate () {

    tcb_t *temp = running;
    running = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    running->next = NULL;

    free(temp->thread_context->uc_stack.ss_sp);
    free(temp->thread_context);
    mbox_destroy(&(temp->mb));
    free(temp);

    setcontext(running->thread_context);
}

void t_shutdown () {

    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    mbox_destroy(&(running->mb));
    free(running);

    while (ready_queue_head) {
        tcb_t *temp = ready_queue_head->next;
        free(ready_queue_head->thread_context->uc_stack.ss_sp);
        free(ready_queue_head->thread_context);
        mbox_destroy(&(ready_queue_head->mb));
        free(ready_queue_head);
        ready_queue_head = temp;
    }

    while (thread_list) {
        tcb_lln_t *temp = thread_list->next;
        free(thread_list);
        thread_list = temp;
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
        printf("\t-------------------------------------------------------\n");
        printf("\tInitialized the following semaphore:\n");
        print_sem(*sp);
        printf("\t-------------------------------------------------------\n");
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

    if (IS_DEBUGGING) { 
        printf("\tCount of semaphore is now %d.\n", sp->count);
        printf("\t-------------------------------------------------------\n");
    }

    if (sp->count < 0) {

        if (IS_DEBUGGING) {
            printf("\tRunning thread %d added to semaphore's queue. Now yielding...\n", running->thread_id);
            printf("\t-------------------------------------------------------\n");
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

    if (IS_DEBUGGING) { 
        printf("\tCount increased to %d\n", sp->count);
        printf("\t-------------------------------------------------------\n");
    }

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

void print_mbox(mbox *mb) {
    printf("\tMailbox 0x%08X: {\n", mb);
    if (mb->mnode) {
        printf("\t\tMessage Node: 0x%08X: {\n", mb->mnode);
        printf("\t\t\tMessage: \"%s\"\n", mb->mnode->msg);
        printf("\t\t\tLength: %d\n", mb->mnode->len);
        printf("\t\t\tSender: %d\n", mb->mnode->sender);
        printf("\t\t\tReceiver: %d\n", mb->mnode->receiver);
        printf("\t\t\tNext: 0x%08X\n", mb->mnode->next);
        printf("\t\t}\n");
    } else {
        printf("\t\tMessage Node: NULL\n");
    }
    printf("\t\tSemaphore: 0x%08X\n", mb->sem);
    printf("\t}\n");
}

int mbox_create(mbox **mb) {

    sem_t *semaphore;
    sem_init(&semaphore, -1);

    *mb = (mbox *) malloc(sizeof(mbox));
    (*mb)->mnode = NULL;
    (*mb)->sem = semaphore;

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tInitialized the following mailbox:\n");
        print_mbox(*mb);
        printf("\t-------------------------------------------------------\n");
    }

    return 0;
}

void mbox_destroy(mbox **mb) {

    mnode_t *message_node = (*mb)->mnode;

    while (message_node) {
        mnode_t *temp = message_node->next;
        free(message_node->msg);
        free(message_node);
        message_node = temp;
    }

    sem_destroy(&((*mb)->sem));

    free((*mb));
}

void mbox_deposit(mbox *mb, char *msg, int len) {

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tA message was deposited into a mailbox!\n");
        printf("\tMessage: \"%s\"\n", msg);
        printf("\tLength: %d\n", len);
        printf("\tBefore deposit ----------------------------------------\n");
        print_mbox(mb);
    }

    char *message = malloc(sizeof(char) * (len + 1));
    strcpy(message, msg);

    mnode_t *message_node = malloc(sizeof(mnode_t));
    message_node->msg = message;
    message_node->len = len;
    message_node->sender = -1;
    message_node->receiver = -1;

    if (mb->mnode) {
        message_node->next = mb->mnode;
    } else {
        message_node->next = NULL;
    }

    mb->mnode = message_node;

    if (IS_DEBUGGING) {
        printf("\tAfter deposit -----------------------------------------\n");
        print_mbox(mb);
        printf("\t-------------------------------------------------------\n");
    }
}

void mbox_withdraw(mbox *mb, char *msg, int *len) {

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tA message was withdrawn from a mailbox!\n");
        printf("\tBefore withdrawl --------------------------------------\n");
        print_mbox(mb);
    }

    if (!mb->mnode) {
        *len = 0;
        return;
    }

    mnode_t *previous = NULL;
    mnode_t *message_node = mb->mnode;

    while (message_node->next) {
        previous = message_node;
        message_node = message_node->next;
    }

    strcpy(msg, message_node->msg);
    *len = message_node->len;

    if (previous) {
        previous->next = NULL;
    } else {
        mb->mnode = NULL;
    }

    free(message_node->msg);
    free(message_node);
    
    if (IS_DEBUGGING) {
        printf("\tAfter withdrawl ---------------------------------------\n");
        print_mbox(mb);
        printf("\t-------------------------------------------------------\n");
    }
}

void send(int tid, char *msg, int len) {

    int sender_id = running->thread_id;
    int receiver_id = tid;

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tA message was sent!\n");
        printf("\tSender TID: %d\n", sender_id);
        printf("\tReceiver TID: %d\n", receiver_id);
        printf("\tMessage: \"%s\"\n", msg);
        printf("\tLength: %d\n", len);
    }

    tcb_lln_t *receiving_thread_node = thread_list;
    tcb_t *receiving_thread = NULL;

    while (receiving_thread_node) {
        if (receiving_thread_node->tcb->thread_id == receiver_id) {
            receiving_thread = receiving_thread_node->tcb;
            break; 
        } else {
            receiving_thread_node = receiving_thread_node->next;
        }
    }

    if (!receiving_thread) {
        if (IS_DEBUGGING) {
            printf("\tCould not find any receiver with ID %d. Returning...", receiver_id);
            printf("\t-------------------------------------------------------\n");
        }
        return;
    }

    mnode_t *message_node = malloc(sizeof(mnode_t));
    message_node->msg = malloc(sizeof(char) * (len + 1));
    strcpy(message_node->msg, msg);
    message_node->len = len;
    message_node->sender = sender_id;
    message_node->receiver = receiver_id;
    
    if (receiving_thread->mb->mnode) {
        message_node->next = receiving_thread->mb->mnode;
    } else {
        message_node->next = NULL;
    }

    if (IS_DEBUGGING) {
        printf("\tCreated Message Node ----------------------------------\n");
        printf("\tMessage Node: 0x%08X: {\n", message_node);
        printf("\t\tMessage: \"%s\"\n", message_node->msg);
        printf("\t\tLength: %d\n", message_node->len);
        printf("\t\tSender: %d\n", message_node->sender);
        printf("\t\tReceiver: %d\n", message_node->receiver);
        printf("\t\tNext: 0x%08X\n", message_node->next);
        printf("\t}\n");
        printf("\t-------------------------------------------------------\n");
    }

    receiving_thread->mb->mnode = message_node;

    sem_signal(receiving_thread->mb->sem);
}

void receive(int *tid, char *msg, int *len) {

    int sender_id = *tid;
    int receiver_id = running->thread_id;

    if (IS_DEBUGGING) {
        printf("\t-------------------------------------------------------\n");
        printf("\tTrying to receive a message!\n");
        printf("\tSender TID: %d\n", sender_id);
        printf("\tReceiver TID: %d\n", receiver_id);
        printf("\tReceiver ----------------------------------------------\n");
        print_tcb(running);
        printf("\tReceiver Mailbox --------------------------------------\n");
        print_mbox(running->mb);
    }

    mnode_t *previous = NULL;
    mnode_t *message_node = running->mb->mnode;

    if (!message_node) {
        sem_wait(running->mb->sem);
    }

    printf("Sequence: ")
    while (message_node->next) {
        previous = message_node;
        message_node = message_node->next;
        printf("0x%08X -> ", previous);
    }
    printf("NULL");

    if (IS_DEBUGGING) {
        printf("\tEarliest message node -----------------------------------\n");
        printf("\tMessage Node: 0x%08X: {\n", message_node);
        printf("\t\tMessage: \"%s\"\n", message_node->msg);
        printf("\t\tLength: %d\n", message_node->len);
        printf("\t\tSender: %d\n", message_node->sender);
        printf("\t\tReceiver: %d\n", message_node->receiver);
        printf("\t\tNext: 0x%08X\n", message_node->next);
        printf("\t}\n");
        printf("\t-------------------------------------------------------\n");
    }

    strcpy(msg, message_node->msg);
    *len = message_node->len;

    if (previous) {
        previous->next = NULL;
    } else {
        running->mb->mnode = NULL;
    }

    running->mb->sem->count--;

    free(message_node->msg);
    free(message_node);
}