#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct shared {
  int shared_data;
  int read_count;
};

/*
 * User-space delay.
 * The user version of xv6 does not provide sleep(),
 * so uptime() is used to create a small delay.
 */
void
delay(int ticks)
{
  int start = uptime();

  while(uptime() - start < ticks)
    ;
}


/*
 * Reader process.
 *
 * Multiple readers are allowed to read shared_data
 * simultaneously.
 */
void
reader(struct shared *s, int rmutex,
       int resource, int serviceQueue, int id)
{
  for(int k = 0; k < 5; k++){

    /*
     * Enter the service queue first.
     * This prevents readers from continuously
     * overtaking waiting writers.
     */
    sem_wait(serviceQueue);

    sem_wait(rmutex);

    s->read_count++;

    /*
     * The first reader obtains the resource lock.
     * Other readers can enter simultaneously.
     */
    if(s->read_count == 1)
      sem_wait(resource);

    sem_post(serviceQueue);
    sem_post(rmutex);

    printf("[tick %d] Reader %d PID %d: reading %d, active readers=%d\n",
           uptime(),
           id,
           getpid(),
           s->shared_data,
           s->read_count);

    /*
     * Keep the reader active for a short time so
     * concurrent readers can be observed.
     */
    delay(5);

    sem_wait(rmutex);

    s->read_count--;

    printf("[tick %d] Reader %d PID %d: finished, active readers=%d\n",
           uptime(),
           id,
           getpid(),
           s->read_count);

    /*
     * The last reader releases the resource.
     */
    if(s->read_count == 0)
      sem_post(resource);

    sem_post(rmutex);

    /*
     * Remainder section.
     */
    delay(5);
  }
}


/*
 * Writer process.
 *
 * Only one writer can access shared_data at a time,
 * and no reader can access it while a writer is writing.
 */
void
writer(struct shared *s, int resource,
       int serviceQueue, int id)
{
  for(int k = 0; k < 5; k++){

    /*
     * Join the service queue before requesting
     * exclusive access to the resource.
     */
    sem_wait(serviceQueue);

    sem_wait(resource);

    /*
     * Allow the next process in the service queue
     * to proceed after this writer has obtained resource.
     */
    sem_post(serviceQueue);

    /*
     * =========================
     * WRITER CRITICAL SECTION
     * =========================
     */
    s->shared_data++;

    printf("[tick %d] WRITER %d PID %d: writing value=%d\n",
           uptime(),
           id,
           getpid(),
           s->shared_data);

    /*
     * Keep the writer in the critical section
     * long enough to demonstrate exclusivity.
     */
    delay(5);

    printf("[tick %d] WRITER %d PID %d: finished writing\n",
           uptime(),
           id,
           getpid());

    /*
     * Release the shared resource.
     */
    sem_post(resource);

    /*
     * Remainder section.
     */
    delay(8);
  }
}


int
main(void)
{
  /*
   * Obtain the shared page.
   * Parent and all children will access the same
   * physical page after fork().
   */
  struct shared *s = (struct shared *)shm_get();

  if(s == 0){
    printf("Shared memory allocation failed\n");
    exit(1);
  }

  s->shared_data = 0;
  s->read_count = 0;

  /*
   * rmutex:
   * Protects read_count.
   *
   * resource:
   * Provides exclusive access to shared_data.
   *
   * serviceQueue:
   * Controls entry order and prevents continuous
   * reader barging.
   */
  int rmutex = sem_create(1);
  int resource = sem_create(1);
  int serviceQueue = sem_create(1);

  if(rmutex < 0 || resource < 0 || serviceQueue < 0){
    printf("Semaphore creation failed\n");
    exit(1);
  }

  /*
   * Create 3 reader processes.
   */
  for(int i = 0; i < 3; i++){

    int pid = fork();

    if(pid < 0){
      printf("fork failed while creating reader\n");
      exit(1);
    }

    if(pid == 0){
      reader(s, rmutex, resource, serviceQueue, i);
      exit(0);
    }
  }

  /*
   * Create 2 writer processes.
   */
  for(int i = 0; i < 2; i++){

    int pid = fork();

    if(pid < 0){
      printf("fork failed while creating writer\n");
      exit(1);
    }

    if(pid == 0){
      writer(s, resource, serviceQueue, i);
      exit(0);
    }
  }

  /*
   * Wait for all 5 child processes.
   */
  for(int i = 0; i < 5; i++)
    wait(0);

  printf("Final shared_data = %d\n", s->shared_data);

  printf("Readers-Writers completed successfully\n");

  exit(0);
}
