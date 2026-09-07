#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define MAXSEM 64

struct semaphore {
  struct spinlock lock;
  int used;
  int value;
};

static struct semaphore sems[MAXSEM];
static struct spinlock sem_table_lock;


// Initialize the semaphore table.
void
seminit(void)
{
  initlock(&sem_table_lock, "sem_table");

  for(int i = 0; i < MAXSEM; i++){
    initlock(&sems[i].lock, "semaphore");
    sems[i].used = 0;
    sems[i].value = 0;
  }
}


// Create a semaphore with the given initial value.
// Returns the semaphore ID, or -1 on failure.
int
semcreate(int value)
{
  if(value < 0)
    return -1;

  acquire(&sem_table_lock);

  for(int i = 0; i < MAXSEM; i++){
    if(sems[i].used == 0){

      sems[i].used = 1;
      sems[i].value = value;

      release(&sem_table_lock);

      return i;
    }
  }

  release(&sem_table_lock);

  return -1;
}


// Wait (P/down) operation.
//
// If the semaphore value is zero, the process sleeps until
// another process performs sempost().
int
semwait(int id)
{
  if(id < 0 || id >= MAXSEM)
    return -1;

  struct semaphore *s = &sems[id];

  acquire(&s->lock);

  if(s->used == 0){
    release(&s->lock);
    return -1;
  }

  while(s->value == 0){

    /*
     * Register this process as waiting on this semaphore.
     * sleep_prepare() must happen while holding the
     * semaphore lock so that a wakeup cannot be lost.
     */
    sleep_prepare(s);

    release(&s->lock);

    /*
     * Sleep until sempost() wakes us.
     */
    sleep();

    /*
     * After waking up, acquire the semaphore lock again
     * and check the value.
     */
    acquire(&s->lock);
  }

  s->value--;

  release(&s->lock);

  return 0;
}


// Signal (V/up) operation.
//
// Increment the semaphore and wake waiting processes.
int
sempost(int id)
{
  if(id < 0 || id >= MAXSEM)
    return -1;

  struct semaphore *s = &sems[id];

  acquire(&s->lock);

  if(s->used == 0){
    release(&s->lock);
    return -1;
  }

  s->value++;

  /*
   * Wake processes waiting on this semaphore.
   */
  wakeup(s);

  release(&s->lock);

  return 0;
}
