#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "mqueue.h"

typedef messageQueue_t  semHandle_t;

#define sem_init(maxCount, initValue)      mQueueCreateSemaphore(maxCount, initValue)


#define sem_destroy(semaphore)          mQueueDelete(semaphore)
#define sem_post(semaphore)             mQueueSend(semaphore, NULL, 0, SEND_TO_BACK)
#define sem_wait(semaphore, TimeOfWait)  mQueueReceive(semaphore, NULL, TimeOfWait, FALSE)



#endif//SEMAPHORE_H
