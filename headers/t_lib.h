/*
 * types used by thread library
 */
 #include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>

struct tcb {
	int         thread_id;
    int         thread_priority;
	ucontext_t *thread_context;
	struct tcb *next;
};

typedef struct tcb tcb_t;

struct semaphore {
	int count;
	tcb_t *queue;
};

typedef struct semaphore sem_t;
