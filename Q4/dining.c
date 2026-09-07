#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NPHIL 5

/*
 * User-space delay.
 *
 * The user version of xv6 does not provide the kernel sleep()
 * function, so we use uptime() for a simple delay.
 */
void
delay(int ticks)
{
  int start = uptime();

  while(uptime() - start < ticks)
    ;
}


/*
 * Each philosopher repeatedly:
 *
 * THINKING -> HUNGRY -> EATING -> THINKING
 *
 * Deadlock avoidance strategy:
 * Resource ordering.
 *
 * Every philosopher always picks the lower-numbered fork
 * first and the higher-numbered fork second.
 *
 * This prevents circular wait and therefore prevents deadlock.
 */
void
philosopher(int id, int forks[], int cycles)
{
  int left = id;
  int right = (id + 1) % NPHIL;

  int first;
  int second;

  /*
   * Always acquire the lower-numbered fork first.
   */
  if(left < right){
    first = left;
    second = right;
  }
  else{
    first = right;
    second = left;
  }

  for(int c = 0; c < cycles; c++){

    /*
     * THINKING
     */
    printf("[tick %d] Philosopher %d: THINKING\n",
           uptime(), id);

    delay(3 + id);

    /*
     * HUNGRY
     */
    printf("[tick %d] Philosopher %d: HUNGRY\n",
           uptime(), id);

    /*
     * Acquire the first fork.
     * Since all philosophers follow the same ordering,
     * circular wait cannot occur.
     */
    sem_wait(forks[first]);

    printf("[tick %d] Philosopher %d acquired fork %d\n",
           uptime(), id, first);

    /*
     * Acquire the second fork.
     */
    sem_wait(forks[second]);

    printf("[tick %d] Philosopher %d acquired fork %d\n",
           uptime(), id, second);

    /*
     * EATING
     */
    printf("[tick %d] Philosopher %d: EATING\n",
           uptime(), id);

    delay(5);

    /*
     * Release both forks after eating.
     */
    sem_post(forks[second]);
    sem_post(forks[first]);

    printf("[tick %d] Philosopher %d: THINKING "
           "(completed cycle %d/%d)\n",
           uptime(), id, c + 1, cycles);

    /*
     * Remainder/thinking section.
     */
    delay(2);
  }

  printf("[tick %d] Philosopher %d COMPLETED ALL %d CYCLES\n",
         uptime(), id, cycles);
}


int
main(int argc, char *argv[])
{
  int cycles = 5;

  /*
   * Optional command-line argument:
   *
   * dining       -> 5 cycles
   * dining 10    -> 10 cycles
   */
  if(argc > 1)
    cycles = atoi(argv[1]);

  if(cycles <= 0){
    printf("Number of cycles must be greater than 0\n");
    exit(1);
  }

  /*
   * Create five binary semaphores.
   *
   * Each semaphore represents one fork.
   *
   * Initial value 1 means the fork is available.
   */
  int forks[NPHIL];

  for(int i = 0; i < NPHIL; i++){
    forks[i] = sem_create(1);

    if(forks[i] < 0){
      printf("Failed to create fork semaphore %d\n", i);
      exit(1);
    }
  }

  /*
   * Create five philosopher processes.
   */
  for(int i = 0; i < NPHIL; i++){

    int pid = fork();

    if(pid < 0){
      printf("fork failed while creating philosopher %d\n", i);
      exit(1);
    }

    if(pid == 0){

      /*
       * Child becomes philosopher i.
       */
      philosopher(i, forks, cycles);

      exit(0);
    }
  }

  /*
   * Parent waits for all five philosophers.
   */
  for(int i = 0; i < NPHIL; i++)
    wait(0);

  printf("\n");
  printf("All philosophers completed %d cycles.\n", cycles);
  printf("Dining Philosophers completed successfully.\n");

  exit(0);
}
