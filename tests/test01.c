/*
 * Test Program T1 - Thread Creation
 */

#include <stdio.h>
#include "ud_thread.h"

void function(int thr_id) 
{
   int i, j;

   for (i = j = 0; i < 3; i++, j++) {
      printf("this is thread %d [%d]...\n", thr_id, j);
      t_yield();
   }

   printf("Thread %d is done...\n", thr_id);
   t_terminate();
}

int main(void)
{
   int i;

   t_init();
   t_create(function, 1, 1);
   printf("This is main(1)...\n");
   t_create(function, 2, 1);
   printf("This is main(2)...\n");
   t_create(function, 3, 1);

   for (i = 0; i < 4; i++) {
      printf("This is main(3)[%d]...\n", i);
      t_yield();
   }

   printf("Begin shutdown...\n");
   t_shutdown();
   printf("Done with shutdown...\n");

   return 0;
}
