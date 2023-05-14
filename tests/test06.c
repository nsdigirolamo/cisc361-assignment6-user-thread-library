/* 
 * Test Program #6 - Mailbox
 */

#include <stdio.h>
#include <stdlib.h>
#include "ud_thread.h"

mbox *mb;
char *msg[2] = {"hello world...", "bye, bye"};

void producer(int id) 
{
  int i;
  char mymsg[30];

  for (i = 0; i < 2; i++) {
    sprintf(mymsg, "%s - tid %d", msg[i], id);
    printf("Producer (%d): [%s] [length=%d]\n", id, mymsg, (int) strlen(mymsg));
    mbox_deposit(mb, mymsg, strlen(mymsg));
  }

  t_terminate();
}

void consumer(int id) 
{
  int i;
  int len;
  char mesg[1024];

  for (i = 0; i < 4; i++) {
    mbox_withdraw(mb, mesg, &len);
    printf("Message from mailbox: [%s]\n", mesg);
  }

  t_terminate();
}

int main(void) {

   t_init();

   mbox_create(&mb);
   t_create(producer, 1, 1);
   t_create(producer, 2, 1);
   t_create(consumer, 3, 1);  
   t_yield();
   mbox_destroy(&mb);

   t_shutdown();
   printf("Done with mailbox test...\n");

   return 0;
}
