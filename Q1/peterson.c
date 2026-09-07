#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct shared {
  volatile int flag[2];
  volatile int turn;
  volatile int counter;
};

void
work(struct shared *s, int id)
{
  int other = 1 - id;

  for(int k = 0; k < 10; k++){

    // Entry section
    s->flag[id] = 1;
    __sync_synchronize();

    s->turn = other;
    __sync_synchronize();

    while(s->flag[other] && s->turn == other){
      // Busy waiting required by Peterson's algorithm.
    }

    // Critical section
    int temp = s->counter;
    temp++;
    s->counter = temp;

    printf("Process %d ENTER CS, counter = %d\n",
           id, s->counter);

    // Small delay while inside CS makes violations easier to notice.
    for(volatile int j = 0; j < 100000; j++)
      ;

    printf("Process %d EXIT CS\n", id);

    // Exit section
    __sync_synchronize();
    s->flag[id] = 0;

    // Remainder section
    for(volatile int j = 0; j < 100000; j++)
      ;
  }
}

int
main(void)
{
  struct shared *s = (struct shared *)shm_get();

  if(s == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  s->flag[0] = 0;
  s->flag[1] = 0;
  s->turn = 0;
  s->counter = 0;

  int pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    work(s, 1);
    exit(0);
  }

  work(s, 0);

  wait(0);

  printf("Final counter = %d\n", s->counter);
  printf("Expected counter = 20\n");

  exit(0);
}
