#include "task.h"

/*任务就绪列表*/
tcb_t* os_readyTasksLists[configMAX_PRIORITIES] = { NULL };

/*任务延时列表*/
static tcb_t* os_delayTaskList1 = NULL;
static tcb_t* os_delayTaskList2 = NULL;
static tcb_t** volatile os_ptrDelayTaskList = &os_delayTaskList1; // 指向当前延时任务列表
static tcb_t** volatile os_ptrOverflowDelayTaskList = &os_delayTaskList2; // 指向溢出延时任务列表


/*任务挂起列表*/
static tcb_t* os_suspendedTaskList = NULL;

/*任务在调度器挂起的时候，本该就绪的链表*/
static tcb_t* os_pendingReadyList = NULL;

/*当前任务控制块*/
tcb_t* volatile os_currentTCB= NULL;

/*系统tick数*/
static volatile clock_t os_tickCount 	= (clock_t) 0U;

/*下一任务解除阻塞时间*/
static volatile clock_t os_nextTaskUnblockTime	= (clock_t) 0U;

/*计时器溢出次数*/
static volatile u32_t os_numOfOverflows = (u32_t)0;

/*记录调度器挂起的这段时间经历的系统tick数*/
static volatile clock_t os_pendedTicks = (clock_t) 0U;

/*当前任务数量*/
static volatile u32_t os_currentNumberOfTasks = (u32_t )0U;

/*调度器标志*/
static u32_t os_schedulerRunning = (u32_t)FALSE;

/*调度器挂起标志*/
static volatile u32_t os_schedulerSuspended = (u32_t)FALSE;

static volatile u32_t os_yieldPending = FALSE;

/*就绪任务优先级位图*/
static volatile u32_t os_priorityBitmap = 0U;

/*空闲任务句柄*/
static TaskHandle_t IdleTaskHandle = NULL;

/*找到当前最高优先级的就绪任务*/
#define SWITCH_HIGHEST_PRIORITY_TASK()														    \
{																								    \
    u32_t topPriority;																		    \
                                                                                                    \
    /*寻找最高优先级*/								                            \
    GET_HIGHEST_PRIORITY( topPriority, os_priorityBitmap );								    \
    /*获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB*/                                       \
    os_currentTCB = (os_readyTasksLists[ topPriority ]);		    \
}

/*切换延时列表*/
#define SWITCH_DELAYED_LISTS()\
{\
    tcb_t **Temp;\
    Temp = os_ptrDelayTaskList;\
    os_ptrDelayTaskList = os_ptrOverflowDelayTaskList;\
    os_ptrOverflowDelayTaskList = Temp;\
    os_numOfOverflows++;\
    os_resetNextTaskUnblockTime();\
}


/*函数声明*/
static void os_initiNewTask( TaskFunction_t task_entry,                     /*任务入口*/
                                const char * const task_name,                     /*任务名称*/
                                  const size_t stack_size,                   /*任务栈大小*/
                                  void * const task_argv,                     /*任务形参*/
                                  u32_t  task_prior,                        /*任务优先级*/
                                  TaskHandle_t *const task_handle,             /*任务句柄*/
                                  tcb_t *task_tcb);                             /*任务控制块指针*/

static void os_addNewTaskToReadyList(tcb_t *task_tcb);


u32_t os_create_task(TaskFunction_t task_entry,                  /*任务入口*/
							const char * const task_name,              /*任务名称*/
							const size_t stack_size,            /*任务栈大小*/
							void * const task_argv,              /*任务形参*/
							u32_t task_prior,                 /*任务优先级*/
							TaskHandle_t * const task_handle )    /*任务句柄*/
{
	tcb_t *new_tcb;
	u32_t ret;

    stack_t *stack;

	/* 动态分配栈内存 */
	stack = ( stack_t * ) os_malloc( stack_size  * sizeof( stack_t ) );
    
    if( stack != NULL )
    {
        /* 动态分配TCB. */
        new_tcb = ( tcb_t * ) os_malloc( sizeof( tcb_t ) ); 

        if( new_tcb != NULL )
        {
            /* Store the stack location in the TCB. */
            new_tcb->stack_start = stack;
        }
        else
        {
            /* 如果TCB分配失败，则释放已分配的栈空间 */
            os_free(stack);
        }
    }
    else
    {
        new_tcb = NULL;
    }

    if( new_tcb != NULL )
    {
        os_initiNewTask( task_entry, task_name, stack_size, task_argv, task_prior, task_handle, new_tcb);
        os_addNewTaskToReadyList( new_tcb );
        ret = PASS;
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static void os_initiNewTask( TaskFunction_t task_entry,                     /*任务入口*/
                                  const char * const task_name,                     /*任务名称*/
                                  const size_t stack_size,                   /*任务栈大小*/
                                  void * const task_argv,                     /*任务形参*/
                                  u32_t  task_prior,                        /*任务优先级*/
                                  TaskHandle_t *const task_handle,             /*任务句柄*/
                                  tcb_t *task_tcb)                               /*任务控制块指针*/
{
    stack_t *TopOfStack;
    u32_t index;

    /*获取栈顶指针*/
    TopOfStack = task_tcb->stack_start + (stack_size - (u32_t) 1);

    /*向下做八字节对齐*/
    TopOfStack = (stack_t *) (((u32_t)TopOfStack) & (~((u32_t) 0x0007)));

    /*将任务的名字存储在TCB中*/
    for( index = (u32_t) 0; index < (u32_t)configMAX_TASK_NAME_LEN; index++)
    {
        task_tcb->task_name[index] = task_name[index];

        if(task_name[index] == 0x00)
        {
            break;
        }
    }
    /*任务的名字长度不能超过configMAX_TASK_NAME_LEN*/
    task_tcb->task_name[ configMAX_TASK_NAME_LEN - 1] = '\0';

    /*初始化TCB中的上一个和下一个节点*/
    task_tcb->next = NULL;
    task_tcb->last = NULL;
    task_tcb->next_event = NULL;
    task_tcb->last_event = NULL;

    //listSET_LIST_ITEM_VALUE( &( task_tcb->xEventListItem ), ( TickType_t ) configMAX_PRIORITIES - ( TickType_t ) task_prior ); 

    /*初始化优先级*/
    if( task_prior >= (u32_t) configMAX_PRIORITIES )
    {
        task_prior = (u32_t) configMAX_PRIORITIES - (u32_t)1U;
    }

    task_tcb->task_prior = task_prior;

    #if ( configUSE_MUTEXES == 1 )
	{
		task_tcb->uxBasePriority = task_prior;
		task_tcb->uxMutexesHeld = 0;
	}
	#endif /* configUSE_MUTEXES */

    /*初始化任务栈*/
    task_tcb->task_sp = os_initTaskStack(TopOfStack,task_entry,task_argv);

    /*让任务句柄指向任务控制块*/
    if((void*)task_handle != NULL)
    {
        *task_handle = ( TaskHandle_t ) task_tcb;
    }
}

void os_addTaskToReadyList(tcb_t *task_tcb)
{
    tcb_t *temp;

    /*检查任务控制块是否为空*/
    if (task_tcb == NULL) {
        //printf("Error: Task control block is NULL.\n");
        return;
    }

    /*检查任务优先级是否在合法范围内*/
    if (task_tcb->task_prior >= (u32_t)configMAX_PRIORITIES || task_tcb->task_prior < 0) {
        //printf("Error: Task priority out of range.\n");
        return;
    }

    /*设置任务就绪位图*/
    RECORD_READY_PRIORITY(task_tcb->task_prior, os_priorityBitmap);

    // 找到该优先级下的就绪链表头
    task_tcb->taskListHead = &os_readyTasksLists[task_tcb->task_prior];

    if (os_readyTasksLists[task_tcb->task_prior] == NULL) {
        // 空链表，直接插入
        os_readyTasksLists[task_tcb->task_prior] = task_tcb;
        task_tcb->next = NULL;
        task_tcb->last = NULL;
        return;
    }

    // 遍历到链表末尾
    temp = os_readyTasksLists[task_tcb->task_prior];
    while (temp->next != NULL) {
        temp = temp->next;
    }

    // 尾插
    temp->next = task_tcb;
    task_tcb->last = temp;
    task_tcb->next = NULL;
}


/*将节点从链表删除*/
void os_taskListRemove(tcb_t* const taskNodeToRemove, which_list_t list)
{
    if(taskNodeToRemove == NULL)
    {
        //printf("Error: Task node to remove is NULL.\n");
        return; // 如果节点为空，直接返回
    }

    if(list == eTASK)
    {
        /*从任务列表中删除*/
        if(taskNodeToRemove->last != NULL)
        {
            taskNodeToRemove->last->next = taskNodeToRemove->next;
        }
        else
        {
            /*如果是第一个节点，则更新头指针*/
            *(taskNodeToRemove->taskListHead) = taskNodeToRemove->next;
        }

        if(taskNodeToRemove->next != NULL)
        {
            taskNodeToRemove->next->last = taskNodeToRemove->last;
        }
    }
    else if(list == eEVENT)
    {
        /*从事件任务列表中删除*/
        if(taskNodeToRemove->last_event != NULL)
        {
            taskNodeToRemove->last_event->next_event = taskNodeToRemove->next_event;
        }
        else
        {
            /*如果是第一个节点，则更新头指针*/
            *(taskNodeToRemove->eventListHead) = taskNodeToRemove->next_event;
        }

        if(taskNodeToRemove->next_event != NULL)
        {
            taskNodeToRemove->next_event->last_event = taskNodeToRemove->last_event;
        }
    }
}


void os_listInsertEnd(tcb_t *taskNodeToInsert, tcb_t **listHead)
{
    tcb_t *temp;

    if (taskNodeToInsert == NULL) {
        //printf("Error: Task node to insert is NULL.\n");
        return;
    }

    if (*listHead == NULL) {
        *listHead = taskNodeToInsert;  // 空链表直接插入
        taskNodeToInsert->next = NULL;
        taskNodeToInsert->last = NULL;
        return;
    }

    temp = *listHead;  // temp 指向第一个节点
    taskNodeToInsert->taskListHead = listHead; // 设置任务链表头
    while (temp->next != NULL) {
        temp = temp->next;  // 遍历到链表末尾
    }

    temp->next = taskNodeToInsert;
    taskNodeToInsert->last = temp;
    taskNodeToInsert->next = NULL;
}

void os_listInsertByDelay(tcb_t *taskNodeToInsert, tcb_t **listHead)
{
    tcb_t *temp, *prev = NULL;

    if (taskNodeToInsert == NULL) {
        //printf("Error: Task node to insert is NULL.\n");
        return;
    }

    // 设置任务链表头指针
    taskNodeToInsert->taskListHead = listHead;

    // 如果链表为空，直接作为第一个节点
    if (*listHead == NULL) {
        *listHead = taskNodeToInsert;
        taskNodeToInsert->next = NULL;
        taskNodeToInsert->last = NULL;
        return;
    }

    temp = *listHead;

    // 找到第一个 ticksToDelay 大于新任务的插入位置
    while (temp != NULL && temp->ticksToDelay <= taskNodeToInsert->ticksToDelay) {
        prev = temp;
        temp = temp->next;
    }

    if (prev == NULL) {
        // 插到链表头
        taskNodeToInsert->next = *listHead;
        (*listHead)->last = taskNodeToInsert;
        taskNodeToInsert->last = NULL;
        *listHead = taskNodeToInsert;
    } else {
        // 插到 prev 和 temp 之间（包括尾插 temp==NULL）
        taskNodeToInsert->next = temp;
        taskNodeToInsert->last = prev;
        prev->next = taskNodeToInsert;
        if (temp != NULL) {
            temp->last = taskNodeToInsert;
        }
    }
}

static void os_addNewTaskToReadyList(tcb_t *task_tcb)
{
    /*进入临界段*/
    ENTER_CRITICAL();
    {
        /*全局任务计数器加一操作*/
        os_currentNumberOfTasks++;

        /*如果os_currentTCB为空，则将os_currentTCB 指向新创建的任务*/
        if(os_currentTCB == NULL)
        {
            os_currentTCB = task_tcb;

            // /*如果是第一次创建任务，则需要初始化任务相关的列表*/
            // if(CurrentNumberOfTasks == (u32_t) 1)
            // {
            //     prvInitialiseTaskLists();
            // }
        }
        else     /*如果os_currentTCB不为空，则将指向最高优先级任务的TCB*/
        {
            if(os_schedulerRunning == FALSE)
            {
                if(os_currentTCB->task_prior <= task_tcb->task_prior)
                {
                    os_currentTCB = task_tcb;
                }
            }
            
        }

        /*将任务添加到就绪列表*/
        os_addTaskToReadyList(task_tcb);
        //portSETUP_TCB(task_tcb);
    }
    EXIT_CRITICAL();/*退出临界段*/

    if(os_schedulerRunning != FALSE)
    {
        if(os_currentTCB->task_prior < task_tcb->task_prior)
        {
            TRIGGER()
        }
    }
}


static void os_resetNextTaskUnblockTime(void)
{
    if(LIST_IS_EMPTY(*os_ptrDelayTaskList) != FALSE)
    {
        /*当前延时列表为空，则设置os_nextTaskUnblockTime等于最大值*/
        os_nextTaskUnblockTime = 0xffffffffU; // 0xFFFFFFFF
    }
    else
    {
        /*当前列表不为空，则有任务在延时，则获取当前列表下第一个节点延时计数器的值，
          然后将该节点的排序值更新到os_nextTaskUnblockTime*/
        os_nextTaskUnblockTime = (*os_ptrDelayTaskList)->ticksToDelay;
    }
}

static void IdleTask(void* param)
{
    /*空闲任务，什么都不做*/
    for(;;)
    {

    }
}

/* 启动调度器 */
void os_taskStartScheduler(void)
{
    u32_t ret;

    ret = os_create_task(IdleTask,"IDLE",128,(void*)NULL,0,&IdleTaskHandle);

    if(ret == PASS)
    {
        DISABLE_INTERRUPTS();

        os_nextTaskUnblockTime = 0xffffffffU; // 0xFFFFFFFF
        os_schedulerRunning = TRUE;
        os_tickCount = 0U;

        //printf("StartScheduler\n\r");
        /*启动调度器*/
        if(os_startScheduler() != FALSE)
        {
                /*调度器启动成功，则不会返回，即不会来到这里*/
        }
    }
}


u32_t os_taskIncrementTick(void)
{
    tcb_t *TCB = NULL;
    clock_t ClockValue;
    u32_t SwitchRequired = (u32_t)FALSE;

    if(os_schedulerSuspended == (u32_t)FALSE)
    {    
        /*如果调度器未挂起，更新系统时基计数器xTickCount*/
        const clock_t ConstTickCount = os_tickCount + 1;
        os_tickCount = ConstTickCount;

        /*如果ConstTickCount溢出，则切换延时列表*/
        if(ConstTickCount == (clock_t) 0U)
        {
            SWITCH_DELAYED_LISTS();
        }

        /*最近的延时任务延时到期*/
        if(ConstTickCount >= os_nextTaskUnblockTime)
        {
            for( ;; )
            {
                if(LIST_IS_EMPTY(*os_ptrDelayTaskList) != FALSE)
                {
                    /*延时列表为空，设置os_nextTaskUnblockTime为可能的最大值*/
                    os_nextTaskUnblockTime = 0xffffffffU; // 0xFFFFFFFF
                    break;
                }
                else
                {
                    /*延时列表不为空*/
                    TCB = (tcb_t*)*(os_ptrDelayTaskList);
                    ClockValue = TCB->ticksToDelay;

                    /*直到将延时列表中所有延时到期的任务移除才跳出for循环*/
                    if(ConstTickCount < ClockValue)
                    {
                        os_nextTaskUnblockTime = ClockValue;
                        break;
                    }

                    /*将任务从延时列表移除，消除等待状态*/
                    os_taskListRemove(TCB, eTASK);

                    /*将解除等待的任务添加到就绪列表*/
                    os_addTaskToReadyList(TCB);

                    /*检查是否需要切换任务*/
                    if(TCB->task_prior >= os_currentTCB->task_prior)
                    {
                        SwitchRequired = TRUE;
                    }
                }
            }
        }

        /*检查当前优先级是否有任务就绪，有就切换（时间片轮转）*/
        if (os_currentTCB->last != NULL || os_currentTCB->next != NULL)
        {
            SwitchRequired = TRUE;
        }

    }
    else
    {
        ++os_pendedTicks;
    }

    if( os_yieldPending != FALSE )
    {
        SwitchRequired = TRUE;
    }
    
    return SwitchRequired;
}

void os_switchContext(void)
{
    if(os_schedulerSuspended != FALSE)
    {
        os_yieldPending = TRUE;
    }
    else
    {
        os_yieldPending = FALSE;

        SWITCH_HIGHEST_PRIORITY_TASK();

    }  
}

void os_suspendAllTask( void )
{
	++os_schedulerSuspended;
}

u32_t os_resumeAllTask( void )
{
    tcb_t *TCB = NULL;
    u32_t alreadyYielded = FALSE;

	/* 如果 uxSchedulerSuspended 为零，则此函数与先前对 vTaskSuspendAll() 的调用不匹配。 */
	//configASSERT( uxSchedulerSuspended );

	/* 有可能在调度器被挂起期间，某个中断服务程序（ISR）导致一个任务从某个事件等待列表中被移除。
    * 在这种情况下，被移除的任务会被添加到 xPendingReadyList 中。一旦调度器恢复运行，
    * 就可以安全地将这个列表中所有处于“待就绪”状态的任务移动到它们各自对应的任务就绪队列中。 */
	ENTER_CRITICAL();
	{
		--os_schedulerSuspended;

		if( os_schedulerSuspended == (u32_t) FALSE )
		{
			if( os_currentNumberOfTasks > ( u32_t ) 0U )
			{
				/* 把待就绪任务从挂起列表移动到就绪列表 */
				while( LIST_IS_EMPTY(os_pendingReadyList) == FALSE)
				{
					TCB = (tcb_t *) os_pendingReadyList;
					os_taskListRemove(TCB, eTASK);
					os_taskListRemove(TCB, eEVENT);
					os_addTaskToReadyList(TCB);

					/* 如果被移动的任务的优先级高于当前任务，则执行上下文切换。 */
					if( TCB->task_prior >= os_currentTCB->task_prior )
					{
						os_yieldPending = TRUE;
					}
				}

				if(TCB != NULL)
				{
					/*一个任务在调度器挂起期间被解除阻塞了，可能需要重新计算下一个解除阻塞时间。*/
					os_resetNextTaskUnblockTime();
				}

				/* 如果在调度器挂起期间发生了滴答，则现在应该处理它们 */
				{
					u32_t uxPendedCounts = os_pendedTicks;

					if( uxPendedCounts > (u32_t) 0U )
					{
						do
						{
							if( os_taskIncrementTick() != FALSE )
							{
								os_yieldPending = TRUE;
							}
							--uxPendedCounts;
						} while( uxPendedCounts > 0U );

						os_pendedTicks = 0;
					}
				}

				if( os_yieldPending != FALSE )
				{

					alreadyYielded = TRUE;

					TRIGGER();// 触发PendSV中断，但是现在处于临界段中，所以不会立即执行上下文切换
				}
			}
		}
	}
	EXIT_CRITICAL();

	return alreadyYielded;
}

static void os_addCurrentTaskToDelayedList(clock_t ticksToWait, const u32_t CanBlockIndefinitely)
{
    clock_t timeToWake;

    /*获取系统时基计数器的 os_tickCount 值*/
    const clock_t constTickCount = os_tickCount;

    /*将任务从就序列表中移除*/
    os_taskListRemove(os_currentTCB, eTASK);
    if(os_readyTasksLists[os_currentTCB->task_prior] == (u32_t)0)
    {
        CLEAR_READY_PRIORITY(os_currentTCB->task_prior, os_priorityBitmap);
    }

    if( (ticksToWait == 0xFFFFFFFF) && ( CanBlockIndefinitely != FALSE ) )
    {
        /*挂起任务*/
        os_listInsertEnd(os_currentTCB, &os_suspendedTaskList);
    }
    else
    {

        /*计算任务延时到期时，系统时基计数器os_tickCount的值是多少*/
        timeToWake = constTickCount + ticksToWait;

        /*将延时到期的值设置为节点的排序值*/
        os_currentTCB->ticksToDelay = timeToWake;

        /*溢出*/
        if(timeToWake < constTickCount)
        {
            os_listInsertByDelay(os_currentTCB,os_ptrOverflowDelayTaskList);
        }
        else/*没有溢出*/
        {
            os_listInsertByDelay(os_currentTCB,os_ptrDelayTaskList);

            /*更新下一个任务解锁时刻变量os_nextTaskUnblockTime的值*/
            if(timeToWake < os_nextTaskUnblockTime)
            {
                os_nextTaskUnblockTime = timeToWake;
            }
        }
    }
}


void os_delay(const clock_t ticksToDelay)
{
    u32_t alreadyYielded = FALSE;

    if( ticksToDelay > (clock_t) 0U )
    {

        os_suspendAllTask();
        {
            /*将任务任务插到延时列表*/
            os_addCurrentTaskToDelayedList(ticksToDelay, FALSE);
        }
        alreadyYielded = os_resumeAllTask();
    }

    if(alreadyYielded == FALSE)
    {
        TRIGGER();
    }
}

u32_t os_taskRemoveFromEventList(const tcb_t * const eventList)
{
    tcb_t *unblockedTCB;
    u32_t ret;

	unblockedTCB = ( tcb_t * ) eventList;
    /*从事件列表中移除任务*/
    os_taskListRemove(unblockedTCB, eEVENT);

	if( os_schedulerSuspended == (u32_t)FALSE)
	{
        /*如果调度器没有挂起，就将任务从挂起列表中移除，并添加到就绪列表中*/
		os_taskListRemove(unblockedTCB, eTASK);
		os_addTaskToReadyList(unblockedTCB);
	}
	else
	{
		/* 如果调度器挂起，就将任务添加到待就绪列表中 */
		os_listInsertEnd(unblockedTCB, &os_pendingReadyList);
	}

	if(unblockedTCB->task_prior > os_currentTCB->task_prior)
	{
		/* 如果被解除阻塞的任务的优先级高于当前任务，则执行上下文切换。 */
		ret = TRUE;
		os_yieldPending = TRUE;
	}
	else
	{
		ret = FALSE;
	}

	return ret;
}

void os_taskSetTimeOutState(TimeOut_t * const TimeOut)
{
	TimeOut->overflowCount = os_numOfOverflows;
	TimeOut->timeOnEntering = os_tickCount;
}

u32_t os_taskCheckForTimeOut(TimeOut_t * const TimeOut, clock_t * const TicksToWait)
{
    u32_t ret;

	ENTER_CRITICAL();
	{
		const u32_t ConstTickCount = os_tickCount;

		#if( INCLUDE_xTaskAbortDelay == 1 )
			if( os_currentTCB->ucDelayAborted != FALSE )
			{
				os_currentTCB->ucDelayAborted = FALSE;
				ret = TRUE;
			}
			else
		#endif

		#if ( INCLUDE_vTaskSuspend == 1 )
			if( *TicksToWait == 0xffffffffUL )
			{
				ret = FALSE;
			}
			else
		#endif

		if((os_numOfOverflows != TimeOut->overflowCount ) && ( ConstTickCount >= TimeOut->timeOnEntering ) ) 
		{
			/*当计数器溢出，且计数值大于timeOnEntering则肯定时间已到*/
			ret = TRUE;
		}
		else if(((clock_t )(ConstTickCount - TimeOut->timeOnEntering ) ) < *TicksToWait ) 
		{
			/* 此处可以不考虑溢出的情况，因为无符号加减中溢出会自动处理 */

            /* 更新超时状态 */
			*TicksToWait -= ( ConstTickCount - TimeOut->timeOnEntering );
			os_taskSetTimeOutState(TimeOut);
            /*未超时*/
			ret = FALSE;
		}
		else
		{
            /*超时*/
			ret = TRUE;
		}
	}
	EXIT_CRITICAL();

	return ret;
}

void os_setTrigger(void)
{
    /* 处理任务错过了调度的情况 */
    os_yieldPending = TRUE;
}


/*
* 把等待事件任务放到事件列表
*/
void os_taskPlaceOnEventList(tcb_t * eventList, const clock_t TicksToWait)
{

	/* 必须在禁用中断或暂停调度程序并锁定正在访问的队列的情况下调用此函数。 */

    /* 将当前任务添加到对应的事件列表 */
	os_listInsertByDelay(os_currentTCB, &eventList);

    /*把当前任务添加到延时列表*/
	os_addCurrentTaskToDelayedList(TicksToWait, TRUE);
}


