#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

void seminit(void);
int semcreate(int value);
int semwait(int id);
int sempost(int id);

#endif
