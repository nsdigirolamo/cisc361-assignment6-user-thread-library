/*
 * Evan Shone and Jake Hoffman 
 * This tests semphores with three different
 * use cases: Synchronization, Locks, Countdown
 */

#include "ud_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

sem_t *sync;
sem_t *lock;
sem_t *count;

int NUM_THREADS = 3; /* less than 10 */
int NUM_ITERATIONS = 3;
int COUNTDOWN_SIZE = 3;

void sync_wait(int id);
void sync_signal(int id);
void lock_thread(int id);
void countdown_thread(int id);
void printBreak();

int main(void) {
  /* init thread library */
  printBreak();
  printf("About to initialize library\n");
  t_init();
  printf("Library initialized\n");

  /* make semaphores */
  printBreak();
  printf("About to initialize semaphores\n");
  sem_init(&sync, 0);
  sem_init(&lock, 1);
  sem_init(&count, COUNTDOWN_SIZE);
  printf("Semaphores initialized\n");

  /* Sync Test */
  printBreak();
  printf("\tSynchronization Test\n");
  printBreak();

  /* make wait and signal threads */
  printf("Making sync_wait\tthread id:%d\n", 1);
  t_create(sync_wait, 1, 1);
  printf("Making sync_signal\tthread id:%d\n", 2);
  t_create(sync_signal, 2, 1);

  /* yield until sem is signaled */
  int yield_i = 0;
  for ( ; yield_i < NUM_ITERATIONS; yield_i++) {
    printBreak();
    printf("Thread id:main\titer:%d\tyield\n", yield_i+1);
    t_yield();
  }

  /* yield once more to signal the sem */
  printBreak();
  printf("Thread id:main\t\tyield\n");
  t_yield();
  printf("\n\tsync_wait should now be woken up\n\n");
  printf("Thread id:main\t\tyield\n");
  t_yield();

  printBreak();
  printf("\tSynchronization Test Complete\n");
  printBreak();

  /* Lock Test */
  printBreak();
  printf("\tLock Test\n");
  printBreak(); 

  int thread_i = 1, lock_i = 0;
  /* Make some threads */
  printf("About to make %d lock threads...\n", NUM_THREADS);
  for ( ; thread_i < NUM_THREADS+1; thread_i++) {
    printf("main creating thread id:%d\n", thread_i+2);
    t_create(lock_thread, thread_i+2, 1);
  }

  /* yield so all the threads run through */
  for ( ; lock_i < NUM_ITERATIONS*(NUM_THREADS+2); lock_i++) {
    printBreak();
    printf("Thread id:main\titer:%d\tyield\n", lock_i+1);
    t_yield();
  }

  printBreak();
  printf("\tLock Test Complete\n");
  printBreak();
  
  /* Countdown Test */
  printBreak();
  printf("\tCountdown Test\n");
  printBreak();

  /* Make some threads */
  printf("About to make %d count threads...\n", NUM_THREADS*2);
  for ( ; thread_i < (NUM_THREADS*3)+1; thread_i++) {
    printf("main creating thread id:%d\n", thread_i+2);
    t_create(countdown_thread, thread_i+2, 1);
  }

  printBreak();
  printf("Only %d threads will be able to make it\n", COUNTDOWN_SIZE);
  t_yield();

  printBreak();
  printf("Those threads signal & terminate\n");
  t_yield();

  printBreak();
  printf("The rest of the threads make it\n");
  t_yield();

  printBreak();
  printf("The rest of the threads signal & terminate\n");
  t_yield();

  printBreak();
  printf("\tCountdown Test Complete\n");

  /* destroy the semaphores */
  printBreak();
  printf("About to destroy semaphores\n");
  sem_destroy(&sync);
  sem_destroy(&lock);
  sem_destroy(&count);
  printf("Semaphores destroyed\n");

  t_shutdown();

  printBreak();
  printf("\tTEST COMPLETE\n");
  printBreak();
  return 0;
}

void countdown_thread(int id) {
  sem_wait(count);
  printf("Thread id:%d\tmade it!\n", id);
  t_yield();
  printf("Thread id:%d\tsignal & terminate\n", id);
  sem_signal(count);
  t_terminate();
}

void lock_thread(int id) {
  int yield_i = 0;
  printf("Thread id:%d\t\tlock\n", id);
  sem_wait(lock); 
  for ( ; yield_i < NUM_ITERATIONS; yield_i++) {
    printf("Thread id:%d\titer:%d\tyield\n", id, yield_i+1);
    t_yield();
  }
  printf("Thread id:%d\t\tunlock\n", id);
  sem_signal(lock);
  printf("Thread id:%d\titer:%d\tyield\n", id, yield_i+1);
  t_yield();
  printf("Thread id:%d\t\tterminating...\n", id);
  t_terminate();
}

void sync_wait(int id) {
  printf("Thread id:%d\t(wait)\twait\n", id);
  sem_wait(sync);
  printf("Thread id:%d\t(wait)\tafter wait\n", id);
  printf("Thread id:%d\t(wait)\tterminating...\n", id);
  t_terminate();
}

void sync_signal(int id) {
  int yield_i = 0;

  for ( ; yield_i < NUM_ITERATIONS; yield_i++) {
    printf("Thread id:%d\titer:%d\tyield\n", id, yield_i+1);
    t_yield();
  }

  printf("Thread id:%d\t(sig)\tsignal\n", id);
  sem_signal(sync);
  printf("Thread id:%d\t(sig)\tafter signal\n", id);
  printf("Thread id:%d\t(sig)\tyield\n", id);
  t_yield();
  printf("Thread id:%d\t(sig)\tafter yield\n", id);
  printf("Thread id:%d\t(sig)\tterminating...\n", id);
  t_terminate();
}

void printBreak() {
  printf("---------------------------------------------------\n");
}
