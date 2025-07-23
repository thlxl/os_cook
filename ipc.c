#include "string.h"
#include "mqueue.h"
#include "mutex.h"

/*锁住队列，主要就是防止中断操作队列*/
#define LockQueue( mQueue )								\
	ENTER_CRITICAL();									\
	{														\
		if( ( mQueue )->cRxLock == -1 )			\
		{													\
			( mQueue )->cRxLock = 0;	\
		}													\
		if( ( mQueue )->cTxLock == -1 )			\
		{													\
			( mQueue )->cTxLock = 0;	\
		}													\
	}								\
	EXIT_CRITICAL()




/*函数声明*/
static void copyDataToQueue(Queue_t * const mQueue, const void *ItemToQueue, const u32_t Position);
static void copyDataFromQueue(Queue_t * const mQueue, void * const Buffer);
static u32_t IsQueueFull(const Queue_t *pxQueue);
static void UnlockQueue( Queue_t * const pxQueue);
static u32_t IsQueueEmpty(const Queue_t *pxQueue);

/*
* 重置消息队列，
* 如果isNewQueue为TRUE时,表示首次创建这个消息队列，
* 如果为FALSE时，表示不是首次创建此队列
*/
u32_t mQueueReset( mQueueHandle_t mQueue, u32_t isNewQueue)
{
	Queue_t * const pxQueue = (Queue_t *)mQueue;

	ENTER_CRITICAL();
	{
		pxQueue->pcTail = pxQueue->pcHead + ( pxQueue->uxLength * pxQueue->uxItemSize );
		pxQueue->uxMessagesWaiting = ( u32_t ) 0U;
		pxQueue->pcWriteTo = pxQueue->pcHead;
		pxQueue->pcReadFrom = pxQueue->pcHead + ( ( pxQueue->uxLength - ( u32_t ) 1U ) * pxQueue->uxItemSize );
		pxQueue->cRxLock = -1;
		pxQueue->cTxLock = -1;

		if(isNewQueue == FALSE)
		{
			/* 如果不是第一次重置消息队列，就得把阻塞在发送队列的任务唤醒，因为重置为空了可以写队列了*/
			if(pxQueue->xTasksWaitingToSend != NULL )
			{
				if( os_taskRemoveFromEventList(pxQueue->xTasksWaitingToSend) != FALSE )
				{
					TRIGGER();
				}
			}
		}
		else
		{
			/* 如果是创建的新队列就初始化，出对和入队阻塞链表 */
			pxQueue->xTasksWaitingToSend = NULL;
			pxQueue->xTasksWaitingToReceive = NULL;
		}
	}
	EXIT_CRITICAL();

	return PASS;
}


/*
* 创建消息队列
* mQueueLength表示创建的队列长度
* ItemSize表示队列中每个消息的大小
*/
mQueueHandle_t mQueueCreate( const u32_t mQueueLength, const u32_t ItemSize)
{
	messageQueue_t *new_mQueue;
	size_t mQueueBytes;
	u8_t *mQueueAddress;

    /* 需要分配的队列空间 */
    mQueueBytes = ( size_t ) ( mQueueLength * ItemSize );

    /* 分配内存 */
	new_mQueue = (messageQueue_t *) os_malloc(sizeof(messageQueue_t) + mQueueBytes);

	if( new_mQueue != NULL )
	{
		/* 指向队列存储区域 */
		mQueueAddress = ((u8_t*)new_mQueue ) + sizeof(messageQueue_t);

        /* 初始化队列 */
        new_mQueue->pcHead = mQueueAddress;
        new_mQueue->uxLength = mQueueLength;
        new_mQueue->uxItemSize = ItemSize;

        mQueueReset( new_mQueue, TRUE);
	}

	return new_mQueue;
}



/*
*往队列发送消息，当队列满时，根据TicksToWait决定是否阻塞等待
*TicksToWait为0则不等待直接返回FALSE表示发送失败
*TicksToWait > 0时表示等待时间，如果等待时间内完成发送则返回TRUE否则返回FALSE
*/
u32_t mQueueSend(mQueueHandle_t mQueue, const void * const ItemToQueue, clock_t TicksToWait, const u32_t CopyPosition )
{
	u32_t entryTimeSet = FALSE;
	TimeOut_t TimeOut;
	Queue_t * const pmQueue = (Queue_t*) mQueue;

	for( ;; )
	{
		ENTER_CRITICAL();
		{
			/*判断队列是否未满或者采用覆写的方式，就直接写*/
			if( (pmQueue->uxMessagesWaiting < pmQueue->uxLength) || (CopyPosition == OVERWRITE))
			{
                /*拷贝消息到队列*/
				copyDataToQueue(pmQueue, ItemToQueue, CopyPosition);

				/* 如果有任务正在等待队列消息，就立即取消阻塞此任务 */
				if(pmQueue->xTasksWaitingToReceive != NULL)
				{
					if(os_taskRemoveFromEventList(pmQueue->xTasksWaitingToReceive) != FALSE )
					{
						/* 解除阻塞的任务具有更高优先级，所以进行一次任务切换 */
						TRIGGER();
					}
				}
				EXIT_CRITICAL();
				return PASS;
			}
			else
			{
				if(TicksToWait == (clock_t)0)
				{
					/*若队列已满，且阻塞时间设为零，就立即退出*/
					EXIT_CRITICAL();
					return FALSE;
				}
				else if(entryTimeSet == FALSE)
				{
					/* 若阻塞时间不为0，则初始化超时结构体，并将超时标志位entryTimeSet 设为TRUE，表明超时结构体已经初始化了*/
					os_taskSetTimeOutState(&TimeOut);
					entryTimeSet = TRUE;
				}
			}
		}
		EXIT_CRITICAL();
        /*只有队列已满且阻塞时间不为零时，才需要将任务挂起，才会执行下面代码*/

		os_suspendAllTask();
        /*锁住队列*/
		LockQueue(pmQueue);

		/* 更新超时状态，并判断时间是否已到 */
		if( os_taskCheckForTimeOut(&TimeOut, &TicksToWait) == FALSE )
		{
            /*时间未到*/

			if(IsQueueFull(pmQueue) != FALSE )
			{
                /*队列已满，挂起发送任务*/
				os_taskPlaceOnEventList(pmQueue->xTasksWaitingToSend, TicksToWait);

				UnlockQueue(pmQueue);

				if(os_resumeAllTask() == FALSE)
				{
					TRIGGER();
				}
			}
			else
			{
				/*时间未到，但是队列还是满的，所以再等一会*/
				UnlockQueue(pmQueue);
				os_resumeAllTask();
			}
		}
		else
		{
			/*时间到了，直接放弃*/
			UnlockQueue(pmQueue);
			os_resumeAllTask();

			return FALSE;
		}
	}
}



/*
*从队列接收消息，当队列空时，根据TicksToWait决定是否阻塞等待
*TicksToWait为0则不等待直接返回FALSE表示发送失败
*TicksToWait > 0时表示等待时间，如果等待时间内完成读取则返回TRUE否则返回FALSE
*/
u32_t mQueueReceive(mQueueHandle_t mQueue, void * const Buffer, clock_t TicksToWait, const u32_t JustPeeking)
{
	u32_t EntryTimeSet = FALSE;
	TimeOut_t TimeOut;
	u8_t *OriginalReadPosition;
	Queue_t * const pmQueue = ( Queue_t * ) mQueue;

	for( ;; )
	{
		ENTER_CRITICAL();
		{
			const u32_t MessagesWaiting = pmQueue->uxMessagesWaiting;

            /*如果队列有消息*/
			if(MessagesWaiting > 0)
			{
				/* 保存读取位置 */
				OriginalReadPosition = pmQueue->pcReadFrom;

				copyDataFromQueue(pmQueue, Buffer);

                /*需要取出队列消息*/
				if(JustPeeking == FALSE )
				{

					/*真实的读取数据，而不是窥视 */
					pmQueue->uxMessagesWaiting = MessagesWaiting - 1;

                    /*如果有任务正在等待发送消息，就立即取消阻塞此任务*/
					if(pmQueue->xTasksWaitingToSend != NULL)
					{
                        /*把阻塞在该消息队列的发送列表的任务移除列表*/
						if(os_taskRemoveFromEventList(pmQueue->xTasksWaitingToSend) != FALSE)
						{
							TRIGGER();
						}
					}
				}
				else
				{
					/*不是真实的读取数据，而只是窥视看一下*/

                    /*因为前面调用了copyDataFromQueue，所以需要恢复读取位置*/
					pmQueue->pcReadFrom = OriginalReadPosition;

					if(pmQueue->xTasksWaitingToReceive != NULL)
					{
                        /*唤醒其他接收任务*/
						if(os_taskRemoveFromEventList(pmQueue->xTasksWaitingToReceive) != FALSE)
						{
							TRIGGER();
						}
					}
				}

				EXIT_CRITICAL();
				return PASS;
			}
			else
			{
                /*消息队列为空*/
				if(TicksToWait == 0)
				{
					/*超时时间为零表示不等待，直接返回失败 */
					EXIT_CRITICAL();
					return FALSE;
				}
				else if(EntryTimeSet == FALSE)
				{
					/* 如果队列为空，且没初始化超时结构体，就初始化超时结构体 */
					os_taskSetTimeOutState( &TimeOut );
					EntryTimeSet = TRUE;
				}
			}
		}
		EXIT_CRITICAL();

		os_suspendAllTask();

        /*锁住消息队列，不能让其他任务来操作消息队列的事件列表*/
		LockQueue(pmQueue);

		/*更新超时结构体 */
		if(os_taskCheckForTimeOut(&TimeOut, &TicksToWait) == FALSE )
		{
            /*未超时*/
			if(IsQueueEmpty(pmQueue) != FALSE )
			{
                /*队列为空，挂起接收任务*/
				os_taskPlaceOnEventList(pmQueue->xTasksWaitingToReceive, TicksToWait);

                /*解锁队列*/
				UnlockQueue(pmQueue);
				if(os_resumeAllTask() == FALSE )
				{
					TRIGGER();
				}
			}
			else
			{
                /*队列不空再等等，但是需要解锁队列以及恢复任务调度，好让其他任务有机会来读取或者发送消息*/
				UnlockQueue(pmQueue);
				os_resumeAllTask();
			}
		}
		else
		{
            /*超时时间到，读取消息失败*/
			UnlockQueue(pmQueue);
			os_resumeAllTask();

			if(IsQueueEmpty(pmQueue) != FALSE)
			{
				return FALSE;
			}
		}
	}
}




/*
*删除消息队列
*/
void mQueueDelete(mQueueHandle_t mQueue)
{
	Queue_t * const pmQueue = (Queue_t *) mQueue;

    /*释放内存*/
	os_free(pmQueue);
}



/*
*写入队列消息，将数据从缓冲区复制到队列
*/
static void copyDataToQueue( Queue_t * const mQueue, const void *ItemToQueue, const u32_t Position )
{
    /* 此函数需要在临界段调用 */

	u32_t uxMessagesWaiting;

    /* 获取当前队列中消息的数量 */
	uxMessagesWaiting = mQueue->uxMessagesWaiting;

	if( Position == SEND_TO_BACK )
	{
        /* 将数据写入队列 ,用BACK方式*/
		memcpy((void *)mQueue->pcWriteTo, ItemToQueue, (size_t)mQueue->uxItemSize ); 
		mQueue->pcWriteTo += mQueue->uxItemSize;

        /*如果到达队列尾部，就绕回头部*/
		if( mQueue->pcWriteTo >= mQueue->pcTail ) 
		{
			mQueue->pcWriteTo = mQueue->pcHead;
		}
	}
	else
	{
        /*把数据写入队列 ,用FRONT方式*/
		memcpy((void *) mQueue->pcReadFrom, ItemToQueue, (size_t) mQueue->uxItemSize ); 
		mQueue->pcReadFrom -= mQueue->uxItemSize;
        /*如果到达队列头部，就绕回尾部*/
		if( mQueue->pcReadFrom < mQueue->pcHead )
		{
			mQueue->pcReadFrom = ( mQueue->pcTail - mQueue->uxItemSize );
		}
	}

	mQueue->uxMessagesWaiting = uxMessagesWaiting + 1;
}



/*
*读取队列消息，将数据从队列复制到缓冲区
*/
static void copyDataFromQueue( Queue_t * const mQueue, void * const Buffer )
{
	if( mQueue->uxItemSize != (u32_t)0)
	{
		mQueue->pcReadFrom += mQueue->uxItemSize;
		if( mQueue->pcReadFrom >= mQueue->pcTail ) 
		{
			mQueue->pcReadFrom = mQueue->pcHead;
		}

		memcpy((void *) Buffer, (void *) mQueue->pcReadFrom, (size_t) mQueue->uxItemSize);
	}
}



/*
*判断队列是否满，为满就返回TRUE
*/
static u32_t IsQueueFull( const Queue_t *mQueue )
{
	u32_t ret;

	ENTER_CRITICAL();
	{
		if( mQueue->uxMessagesWaiting == mQueue->uxLength )
		{
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}
	}
	EXIT_CRITICAL();

	return ret;
}


/*
*判断队列是否空，为空就返回TRUE
*/
static u32_t IsQueueEmpty( const Queue_t *mQueue )
{
	u32_t ret;

	ENTER_CRITICAL();
	{
		if( mQueue->uxMessagesWaiting == 0)
		{
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}
	}
	EXIT_CRITICAL();

	return ret;
}


/*
*解锁队列
*处理在队列锁住的情况下中断往队列中添加或者取出消息时
*需要更新的阻塞事件
*/
static void UnlockQueue(Queue_t * const mQueue )
{
	/* 此函数必须再调度器暂停时才能调用 */
    /* 处理队列上锁时，添加和删除队列消息是来自中断 */

	ENTER_CRITICAL();
	{
        /*处理队列上锁时，添加的队列的消息*/
		s8_t cTxLock = mQueue->cTxLock;

		/* 查看数据是否在队列锁定时被添加到队列中。*/
		while(cTxLock > 0 )
		{
			/* 从事件列表中删除的任务将被添加到待处理就绪列表中，因为调度程序仍处于暂停状态。 */
			if(mQueue->xTasksWaitingToReceive != FALSE )
			{
				if(os_taskRemoveFromEventList(mQueue->xTasksWaitingToReceive) != FALSE )
				{
					/*任务等待具有更高的优先级，因此请记录需要上下文切换。*/
					os_setTrigger();
				}
			}
			else
			{
				break;
			}
			
			--cTxLock;
		}

		mQueue->cTxLock = -1;
	}
	EXIT_CRITICAL();

	ENTER_CRITICAL();
	{
        /*处理队列上锁时，取出的队列的消息*/
		s8_t cRxLock = mQueue->cRxLock;

		while(cRxLock > 0)
		{
			if(mQueue->xTasksWaitingToSend == FALSE )
			{
				if(os_taskRemoveFromEventList(mQueue->xTasksWaitingToSend) != FALSE)
				{
					os_setTrigger();
				}
				--cRxLock;
			}
			else
			{
				break;
			}
		}

		mQueue->cRxLock = -1;
	}
	EXIT_CRITICAL();
}



/*
*创建一个信号量，并初始化信号量
*信号量底层使用的是消息队列那一套API
*/
mQueueHandle_t mQueueCreateSemaphore(const u32_t maxCount, const u32_t initValue)
{
	mQueueHandle_t mQueue;
	mQueue = mQueueCreate(maxCount, 0);

	if(mQueue != NULL)
	{
		/* 初始化信号量 */
		((Queue_t*)mQueue)->uxMessagesWaiting = initValue;
	}

	return mQueue;
}


/*
*创建一个互斥量
*/
mutexHandle_t mutexCreat(void)
{
	mutex_t* mutex;

	mutex = (mutex_t*)os_malloc(sizeof(mutex_t));
	if(mutex == NULL){
		return mutex;
	}

	mutex->owner = NULL;
	mutex->taken_list = NULL;
	mutex->priority = 0;
	mutex->hold = 0;

	return mutex;
}


/*
*删除互斥量（注：删除时，要手动保证没有任务线程阻塞到这个互斥量上）
*/
void mutexDelete(mutexHandle_t mutex)
{
	mutex_t* pmutex = (mutex_t*)mutex;

	os_free(pmutex);
}


/*
* 互斥量上锁
*/
u32_t mutexLock(mutexHandle_t mutex, clock_t TicksToWait)
{
	u32_t EntryTimeSet = FALSE;
	TimeOut_t TimeOut;
	mutex_t* const pmutex = (mutex_t*) mutex;

	tcb_t* const currentTaskTcb = (tcb_t*)os_getCurrentTaskHandle();

	if(pmutex->owner == currentTaskTcb)
	{
		pmutex->hold++;
		return PASS;
	}
	else
	{
		for(;;)
		{
			ENTER_CRITICAL();
			{
				/*如果当前互斥量没上锁*/
				if(pmutex->hold == 0)
				{
					/*记录互斥量持有者的信息*/
					pmutex->hold = 1;
					pmutex->owner = currentTaskTcb;
					pmutex->priority = currentTaskTcb->task_prior;
					currentTaskTcb->mutexHeld++;
					return PASS;
				}
				else if(TicksToWait == 0)
				{

					EXIT_CRITICAL();
					return FALSE;
				}
				else if(EntryTimeSet == FALSE)
				{
					os_taskSetTimeOutState(&TimeOut);
					EntryTimeSet = TRUE;
				}
			}
			EXIT_CRITICAL();

			os_suspendAllTask();
			if(os_taskCheckForTimeOut(&TimeOut, &TicksToWait) == FALSE)
			{
			
				if(pmutex->hold != 0)
				{
					ENTER_CRITICAL();

					os_taskPriorInherit(pmutex->owner);
					os_taskPlaceOnEventList(pmutex->taken_list, TicksToWait);

					EXIT_CRITICAL();
					if(os_resumeAllTask() == FALSE)
					{
						TRIGGER();
					}
				}
				else
				{
					os_resumeAllTask();
				}
			}
			else
			{
				os_resumeAllTask();
				return FALSE;
			}
		}
	}
}


/*
* 互斥量解锁
*/
u32_t mutexUnlock(mutexHandle_t mutex)
{
	u32_t ret;
	mutex_t* const pmutex = (mutex_t*) mutex;

	tcb_t* const currentTaskTcb = (tcb_t*)os_getCurrentTaskHandle();

	if(pmutex->owner == currentTaskTcb)
	{
		pmutex->hold--;

		if(pmutex->hold == 0)
		{
			ENTER_CRITICAL();
			{
				os_taskPriorDisinherit(pmutex->owner, pmutex->priority);
				pmutex->owner = NULL;
				pmutex->priority = 0;
				pmutex->hold = 0;
				currentTaskTcb->mutexHeld--;

				if(pmutex->taken_list != NULL)
				{
					if(os_taskRemoveFromEventList(pmutex->taken_list) != FALSE)
					{
						TRIGGER();
					}
				}

			}
			EXIT_CRITICAL();
		}

		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}

	return ret;
}


