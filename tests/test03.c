/* 
 * Test Program #3 - Semaphore
 */

#include <stdio.h>
#include <stdlib.h>
#include "ud_thread.h"

sem_t *s;
int resource = 0;

void read_function(int val) 
{
   int i;
   int res_before, res_after;

   for (i = 0; i < 5; i++) 
   {
      printf("I am READ thread %d (%d)\n", val, i);
      sem_wait(s);        
      res_before = resource;
      printf("  [%d READ %d] resource = %d\n", val, i, resource);
      t_yield();
      printf("  ** [%d READ %d] returns from t_yield()\n", val, i);
      res_after = resource;
      if (res_before != res_after) 
      {
         fprintf(stderr, "[THREAD %d] error in the semaphore program \n", val);
         exit(-1);
      }
      sem_signal(s);
   }
   t_terminate();
}

void write_function(int val) 
{
   int i;

   for (i = 0; i < 5; i++) 
   {
      printf("I am WRITE thread %d (%d)\n", val, i);\
      sem_wait(s);
      resource = rand() % 100;      
      printf("  [%d WRITE %d] resource = %d\n", val, i, resource);
      sem_signal(s);
      t_yield();
   }
   t_terminate();
}

int main(void) 
{

   int i;

   t_init();
   sem_init(&s, 1);
   
   t_create(write_function, 1, 1);
   t_create(write_function, 2, 1);
   t_create(read_function, 11, 1);
   t_create(read_function, 22, 1);
   t_create(write_function, 3, 1);
   t_create(write_function, 4, 1);
   t_create(read_function, 33, 1);
   t_create(read_function, 44, 1);
  
   for (i = 0; i < 60; i++) 
   {
      printf("I am main thread (%d)...\n", i);
      t_yield();
   }

   sem_destroy(&s);
   t_shutdown();

   return 0;
}
