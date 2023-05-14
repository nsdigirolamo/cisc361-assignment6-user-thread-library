#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ud_thread.h"

sem_t *s1, *s2, *s3;

void worker1(int val) {
  sem_wait(s1);
  printf("I am worker 1\n");
  sem_signal(s1);
  sem_signal(s2);
  t_yield();
}

void worker2(int val) {
  printf("I am worker 2\n");
  sem_signal(s1);
  t_yield();
}

void worker3(int val) {
  sem_wait(s3);
  sem_wait(s2);
  printf("I am worker 3\n");
  t_yield();
}

void worker4(int val) {
  sem_wait(s1);
  printf("I am worker 4\n");
  sem_signal(s1);
  sem_signal(s3);
  t_yield();
}

int main(int argc, char *argv[])
{
  pthread_t p1, p2, p3, p4;

  t_init();
  sem_init(&s1, 0);
  sem_init(&s2, 0);
  sem_init(&s3, 0);

  t_create(worker3, 3, 0);
  t_create(worker4, 4, 0);
  t_create(worker2, 2, 0);
  t_create(worker1, 1, 0);

  t_yield();
  t_yield();
  t_yield();
  t_yield();

  /*
  pthread_create(&p3, NULL, worker3, NULL);
  pthread_create(&p4, NULL, worker4, NULL);
  pthread_create(&p2, NULL, worker2, NULL);
  pthread_create(&p1, NULL, worker1, NULL); 
  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  pthread_join(p3, NULL);
  pthread_join(p4, NULL);
  */

  printf("done.......\n");
  return 0;
}
