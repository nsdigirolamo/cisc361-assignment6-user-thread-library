/*
 * types used by thread library
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>

struct tcb {
	int thread_id;
    int thread_priority;
	ucontext_t *thread_context;
	struct mailBox *mb;
	struct tcb *next;
};

typedef struct tcb tcb_t;

struct tcb_linked_list_node {
	tcb_t *tcb;
	struct tcb_linked_list_node *next;
}

typedef struct tcb_linked_list_node tcb_lln_t;

struct semaphore {
	int count;
	tcb_t *queue;
};

typedef struct semaphore sem_t;

struct messageNode {
	char *msg;
	int len;
	int sender;
	int receiver;
	struct messageNode	*next;
};

typedef struct messageNode mnode_t;

struct mailBox {
	mnode_t *mnode;
	sem_t   *sem;
};

typedef struct mailBox mbox;
