#ifndef MUTEX_H__
#define MUTEX_H__

#include "type.h"
#include "task.h"

typedef struct os_mutex
{
    u8_t           priority;                      /*持有互斥量的任务优先级*/
    u8_t           hold;                          /*为0时表示，互斥量没有被持有，大于0时表示被递归持有的次数*/

    tcb_t*     owner;                         /*持有互斥量的任务*/
    tcb_t*     taken_list;                    /* 阻塞在互斥量上的任务列表*/
}mutex_t;


typedef void * mutexHandle_t;

u32_t mutexLock(mutexHandle_t mutex, clock_t TicksToWait);
u32_t mutexUnlock(mutexHandle_t mutex);
mutexHandle_t mutexCreat(void);
void mutexDelete(mutexHandle_t mutex);


/*互斥量尝试上锁，如果互斥量被其他任务持有，则立即返回，不会阻塞当前任务*/
#define mutexTryLock(mutex) mutexLock(mutex, 0)

#endif//MUTEX_H__
