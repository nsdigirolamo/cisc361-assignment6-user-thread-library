/*
 * Test Program #5 - Send/Receive
 */

#include <stdio.h>
#include <string.h>
#include "ud_thread.h"

void sender(int thr_id) 
{
   int i, j;
   char *test_message = "Test Message";

   printf("Message is: %s, length: %d\n", test_message, (int) strlen(test_message));

   for (i = j = 0; i < 3; i++, j++) {

      printf("[Pitch] - This is thread %d [%d]...\n", thr_id, j);
      send(2, test_message, strlen(test_message));
   }

   printf("Thread %d is done...\n", thr_id);
   t_terminate();
}

void catcher(int thr_id) 
{
   int i, j;
   int snd_id;
   int len;
   char buffer[1024];

   snd_id = 1;
   for (i = j = 0; i < 3; i++, j++) {
      printf("[Catch] - This is thread %d [%d]...\n", thr_id, j);
      receive(&snd_id, buffer, &len);

      if(len) {
         printf("Catcher got [%s] from thread %d\n", buffer, snd_id);
      }
   }

   printf("Thread %d is done...\n", thr_id);
   t_terminate();
}

int main(void) {

   int i;

   t_init();

   t_create(sender, 1, 1); 
   t_create(catcher, 2, 1); 
   t_yield();

   t_shutdown();

   return 0;
}

