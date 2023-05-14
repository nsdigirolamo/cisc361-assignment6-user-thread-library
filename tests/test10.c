/* 
 * Test Program #10 - Critical Sections with Yields
 */

#include <stdio.h>
#include <stdlib.h>
#include "ud_thread.h"

sem_t *mutex;
int resource = 1;

void read_function(int val) 
{
  printf("I am thread %d...\n", val);

  sem_wait(mutex);
    printf("thread %d entering 1st CS...\n", val);
    resource = 11;
    printf("in thread: resource is %d\n", resource);
    t_yield();
    resource = 22;
    printf("in thread: resource is %d\n", resource);
    printf("thread %d leaving 1st CS...\n", val);
  sem_signal(mutex);

  sem_wait(mutex);
    printf("thread %d entering 2nd CS...\n", val);
    resource = 55;
    printf("in thread: resource is %d\n", resource);
    t_yield();
    resource = 66;
    printf("in thread: resource is %d\n", resource);
    printf("thread %d leaving 2nd CS...\n", val);
  sem_signal(mutex);

  printf("thread %d: t_terminate()\n", val);
  t_terminate();
}

int main(void)
{
   t_init();
   sem_init(&mutex, 1);

   t_create(read_function, 123, 1);

   t_yield();

   sem_wait(mutex);
     printf("main thread entering CS...\n");
     resource = 33;
     printf("in main: resource is %d\n", resource);
     t_yield();
     resource = 77;
     printf("in main: resource is %d\n", resource);
     printf("main thread leaving CS...\n");
   sem_signal(mutex);

   printf("main thread gives up CPU...\n");
   t_yield();
   t_yield();

   printf("in main: sem_destroy() and t_shutdown()\n");
   sem_destroy(&mutex);
   t_shutdown();

   return 0;
}
