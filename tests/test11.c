/* 
 * Test Program #11 - Rendezvous: blocking Send/Receive
 */

#include <stdio.h>
#include <stdlib.h>
#include "ud_thread.h"

void producer(int val) {

   char *msg = "hello world...\0";

   printf("P: Before block_send()...\n");
   block_send(2, msg, strlen(msg));
   printf("P: After block_send()...\n");

   t_yield();

   t_terminate();
}

void consumer(int val) {

   int len, who = 1;
   char msg[1024];

   printf("C: Before block_receive()...\n");
   block_receive(&who, msg, &len);
   printf("C: After block_receive()...\n");

   if (len != 0) {
     printf("C %d got message [%s] from P %d...\n", val, msg, who);
   }

   t_terminate();
}

int main(void) {

   t_init();

   t_create(producer, 1, 1);
   t_create(consumer, 2, 1);

   t_yield();
   t_yield();
   t_yield();

   t_shutdown();

   return 0;
}
