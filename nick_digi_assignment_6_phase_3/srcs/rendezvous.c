#include <stdio.h>
#include <unistd.h>
#include "ud_thread.h"

// If done correctly, each child should print their "before" message
// before either prints their "after" message. Test by adding sleep(1)
// calls in various locations.

sem_t *s1, *s2;

void child_1(int val) {
    printf("child 1: before\n");
    sem_signal(s1);
    sem_wait(s2);
    printf("child 1: after\n");
}

void child_2(int val) {
    printf("child 2: before\n");
    sem_signal(s2);
    sem_wait(s1);
    printf("child 2: after\n");
}

int main(int argc, char *argv[]) {

    t_init();
    printf("parent: begin\n");
    sem_init(&s1, 0);
    sem_init(&s2, 0);
    t_create(child_1, 1, 0);
    t_create(child_2, 2, 0);
    t_yield();
    t_yield();
    t_yield();
    printf("parent: end\n");
    return 0;

    /*
    pthread_t p1, p2;
    printf("parent: begin\n");
    sem_init(&s1, 0, 0);
    sem_init(&s2, 0, 0);
    pthread_create(&p1, NULL, child_1, NULL);
    pthread_create(&p2, NULL, child_2, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("parent: end\n");
    return 0;
    */
}

