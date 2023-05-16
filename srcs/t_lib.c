#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include "t_lib.h"

int mbox_create(mbox **mb);
void mbox_destroy(mbox **mb);

#define IS_DEBUGGING false

tcb_t *running = NULL;

tcb_t *ready_queue_head = NULL;
tcb_t *ready_queue_tail = NULL;

tcb_lln_t *thread_list = NULL;

void t_yield () {

    if (IS_DEBUGGING) {
        print_ready_queue();
    }

    if (!ready_queue_head) {
        setcontext(running->thread_context);
    }

    ready_queue_tail->next = running;
    ready_queue_tail = ready_queue_tail->next;

    running = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    running->next = NULL;

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
    node->next = NULL;
    
    tcb_lln_t *temp = thread_list;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = node;

    if (!ready_queue_head) {
        ready_queue_head = control_block;
        ready_queue_tail = ready_queue_head;
    } else {
        ready_queue_tail->next = control_block;
        ready_queue_tail = ready_queue_tail->next;
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

int sem_init(sem_t **sp, unsigned int count) {
    
    *sp = (sem_t *) malloc(sizeof(sem_t));
    (*sp)->count = count;
    (*sp)->queue = NULL;

    return 0;
}

void sem_wait(sem_t *sp) {
    sighold(SIGALRM);

    sp->count--;

    if (sp->count < 0) {
        
        if (!ready_queue_head) {
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

        swapcontext(sp->queue->thread_context, running->thread_context);
    }

    sigrelse(SIGALRM);
}

void sem_signal(sem_t *sp) {
    sighold(SIGALRM);

    sp->count++;

    if (sp->queue) {

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

void print_mnode_list(mnode_t *node) {
    printf("\t");
    while (node) {
        printf("0x%08X -> ", node);
        node = node->next;
    }
    printf("NULL\n");
}

int mbox_create(mbox **mb) {

    sem_t *semaphore;
    sem_init(&semaphore, -1);

    *mb = (mbox *) malloc(sizeof(mbox));
    (*mb)->mnode = NULL;
    (*mb)->sem = semaphore;

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
}

void mbox_withdraw(mbox *mb, char *msg, int *len) {

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
}

void send(int tid, char *msg, int len) {

    int sender_id = running->thread_id;
    int receiver_id = tid;

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

    receiving_thread->mb->mnode = message_node;

    sem_signal(receiving_thread->mb->sem);
}

void receive(int *tid, char *msg, int *len) {

    int sender_id = *tid;
    int receiver_id = running->thread_id;

    mnode_t *previous = NULL;
    mnode_t *current = running->mb->mnode;
    mnode_t *found = NULL;

    if (!current) {
        sem_wait(running->mb->sem);
    }

    if (sender_id == 0) {
        while (current->next) {
            previous = current;
            current = current->next;
        }
        found = current;
    } else {
        while (current) {
            if (current->sender == sender_id) { 
                found = current;
                break;
            }
            previous = current;
            current = current->next;
        }
    }

    if (found) {
        *tid = found->sender;
        strcpy(msg, found->msg);
        *len = found->len;

        if (previous) {
            previous->next = found->next;
        } else {
            running->mb->mnode = NULL;
        }

        running->mb->sem->count--;

        free(found->msg);
        free(found);
    } else {
        *len = 0;
    }
}

void block_send(int tid, char *msg, int length) {
    int sender_id = running->thread_id;
    int receiver_id = tid;

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
        return;
    }

    mnode_t *message_node = malloc(sizeof(mnode_t));
    message_node->msg = malloc(sizeof(char) * (length + 1));
    strcpy(message_node->msg, msg);
    message_node->len = length;
    message_node->sender = sender_id;
    message_node->receiver = receiver_id;
    
    if (receiving_thread->mb->mnode) {
        message_node->next = receiving_thread->mb->mnode;
    } else {
        message_node->next = NULL;
    }

    receiving_thread->mb->mnode = message_node;

    sem_signal(receiving_thread->mb->sem);

    mnode_t *node = receiving_thread->mb->mnode;

    t_yield();
}

void block_receive(int *tid, char *msg, int *length) {

    int sender_id = *tid;
    int receiver_id = running->thread_id;

    mnode_t *previous = NULL;
    mnode_t *current = running->mb->mnode;
    mnode_t *found = NULL;

    if (!current) {
        sem_wait(running->mb->sem);
        current = running->mb->mnode;
    }

    if (sender_id == 0) {
        while (current->next) {
            previous = current;
            current = current->next;
        }
        found = current;
    } else {
        while (current) {
            if (current->sender == sender_id) { 
                found = current;
                break;
            }
            previous = current;
            current = current->next;
        }
    }

    if (found) {
        *tid = found->sender;
        strcpy(msg, found->msg);
        *length = found->len;

        if (previous) {
            previous->next = found->next;
        } else {
            running->mb->mnode = NULL;
        }

        running->mb->sem->count--;

        free(found->msg);
        free(found);
    } else {
        *length = 0;
    }
}