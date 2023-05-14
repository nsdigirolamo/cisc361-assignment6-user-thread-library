/*
 * Test Program #8 - Asynchronous Send/Receive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ud_thread.h"

void producer(int val) 
{
  char msg[] = "hello world...";
  int i = 0;

  for (i = 0; i < 4; i++) {
    msg[12] = (char) i+0x30;
    send(2, msg, strlen(msg));
    printf("thread %d [%s]...\n", val, msg);
  }

  printf("producer thread %d terminates...\n", val);
  t_terminate();
}

void consumer(int val) 
{
  int i, len, who = 0;
  char *msg;

  msg = (char *) malloc(1024);

  for (i = 0; i < 8; i++) {
    who = 0;
    receive(&who, msg, &len);
    if (who != 0)
      printf("I got message [%s] from %d...\n", msg, who);
  }
   
  free(msg);

  printf("consumer thread %d terminate...\n", val);
  t_terminate();
}

int main(void) 
{
  t_init();

  t_create(producer, 1, 1);
  t_create(producer, 3, 1);
  t_create(consumer, 2, 1);

  t_yield();

  printf("thread library shutting down...\n");
  t_shutdown();

  return 0;
}
