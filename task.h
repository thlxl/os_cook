#ifndef TASK_H
#define TASK_H

#include "type.h"
#include "config.h"
#include "mem.h"
#include "cortex-m3/include/cm3.h"
#include <stdio.h>

typedef struct tsk
{
    volatile stack_t       *task_sp;                                /*栈顶*/
    struct tsk    *next;                              /*下一任务链表节点*/
    struct tsk    *last;                              /*上一任务链表节点*/
    struct tsk    *next_event;                         /*下一事件链表节点*/
    struct tsk    *last_event;                         /*上一事件链表节点*/
    stack_t                 *stack_start;                       /*任务栈起始地址*/
    char                    task_name[configMAX_TASK_NAME_LEN];                     /*任务名称，字符串形式*/
    clock_t                 ticksToDelay;                         /*延时计数器*/
    u32_t             task_prior;                          /*任务优先级*/
    struct tsk    **taskListHead;                  /*任务链表头*/
    struct tsk    **eventListHead;                  /*事件链表头*/
    #if ( configUSE_MUTEXES == 1 )
		u32_t		uxBasePriority;		/*< 优先级继承机制使用 */
		u32_t		uxMutexesHeld;
	#endif
}tcb_t;

typedef enum which_list
{
    eTASK = 0,          /*任务就绪链表*/
    eEVENT              /*事件链表*/
} which_list_t;

typedef struct TIME_OUT
{
	u32_t overflowCount;
	clock_t timeOnEntering;
} TimeOut_t;

/*判断链表是否为空*/
#define LIST_IS_EMPTY(tskList)    ((tskList) == (NULL) ? TRUE : FALSE)


u32_t os_create_task(TaskFunction_t task_entry,                  /*任务入口*/
							const char * const task_name,              /*任务名称*/
							const size_t stack_size,            /*任务栈大小*/
							void * const task_argv,              /*任务形参*/
							u32_t task_prior,                 /*任务优先级*/
							TaskHandle_t * const task_handle );    /*任务句柄*/

void os_taskStartScheduler(void);

void os_delay(const clock_t ticksToDelay);

void os_suspendAllTask( void );

u32_t os_resumeAllTask( void );

u32_t os_taskRemoveFromEventList(const tcb_t * const eventList);

void os_taskSetTimeOutState(TimeOut_t * const TimeOut);

u32_t os_taskCheckForTimeOut(TimeOut_t * const TimeOut, clock_t * const TicksToWait);

void os_setTrigger(void);

void os_taskPlaceOnEventList(tcb_t *eventList, const clock_t TicksToWait);

#endif
