#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_BUFFER 32

struct shared {
  int buffer[MAX_BUFFER];
  int in;
  int out;
  int size;
};

/*
 * Simple user-space delay.
 * xv6 user programs do not have the kernel sleep() function.
 */
void
delay(int ticks)
{
  int start = uptime();

  while(uptime() - start < ticks)
    ;
}

int
main(int argc, char *argv[])
{
  int size = 5;

  // Allow the buffer size to be specified from the command line.
  if(argc > 1)
    size = atoi(argv[1]);

  if(size <= 0 || size > MAX_BUFFER){
    printf("Buffer size must be between 1 and %d\n", MAX_BUFFER);
    exit(1);
  }

  /*
   * Obtain a shared page.
   * After fork(), parent and child will access the same page.
   */
  struct shared *s = (struct shared *)shm_get();

  if(s == 0){
    printf("Shared memory allocation failed\n");
    exit(1);
  }

  s->in = 0;
  s->out = 0;
  s->size = size;

  /*
   * Three synchronization objects:
   *
   * empty = number of empty buffer slots
   * full  = number of occupied buffer slots
   * mutex = protects the circular buffer itself
   */
  int empty = sem_create(size);
  int full  = sem_create(0);
  int mutex = sem_create(1);

  if(empty < 0 || full < 0 || mutex < 0){
    printf("Semaphore creation failed\n");
    exit(1);
  }

  int pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  /*
   * CHILD = CONSUMER
   */
  if(pid == 0){

    /*
     * Initially wait a little so that the consumer
     * demonstrates blocking when the buffer is empty.
     */
    printf("Consumer: waiting for first item\n");

    for(int i = 0; i < 20; i++){

      /*
       * If full == 0, the consumer blocks here.
       */
      sem_wait(full);

      /*
       * Only one process can modify the buffer at a time.
       */
      sem_wait(mutex);

      int item = s->buffer[s->out];

      s->out = (s->out + 1) % s->size;

      printf("Consumer: consumed %d\n", item);

      sem_post(mutex);

      /*
       * One more empty slot is now available.
       */
      sem_post(empty);

      /*
       * Make consumer slower than producer.
       * This helps demonstrate the bounded-buffer behaviour.
       */
      delay(8);
    }

    printf("Consumer: completed 20 items\n");

    exit(0);
  }

  /*
   * PARENT = PRODUCER
   */

  /*
   * Start after a small delay so the consumer gets a chance
   * to block on an empty buffer.
   */
  delay(15);

  for(int item = 1; item <= 20; item++){

    printf("Producer: trying to produce %d\n", item);

    /*
     * If the buffer is full, the producer blocks here.
     */
    sem_wait(empty);

    sem_wait(mutex);

    s->buffer[s->in] = item;

    s->in = (s->in + 1) % s->size;

    printf("Producer: produced %d\n", item);

    sem_post(mutex);

    /*
     * One more item is now available for the consumer.
     */
    sem_post(full);

    /*
     * Producer is deliberately faster than consumer.
     * With a buffer of size 5, it will eventually block
     * when the buffer becomes full.
     */
    delay(2);
  }

  wait(0);

  printf("Producer: completed 20 items\n");
  printf("Producer-Consumer completed successfully\n");

  exit(0);
}
