#ifndef ARMCM3_H
#define ARMCM3_H

#include "config.h"
#include "type.h"
#include <arm_acle.h>// ARM C语言扩展

 void RaiseBASEPRI(void);
 void SetBASEPRI(u32_t value);
 u32_t RaiseBASEPRIReturn(void);
 void ClearBASEPRIFromISR();

/*根据优先级设置/清除优先级位图中相应的位*/
#define RECORD_READY_PRIORITY(Priority, ReadyPriorities) (ReadyPriorities ) |= (1UL << (Priority ))
#define CLEAR_READY_PRIORITY(Priority, ReadyPriorities) (ReadyPriorities ) &= ~(1UL << (Priority ))

/* 获取就绪任务中最高优先级，支持32位优先级*/
#define GET_HIGHEST_PRIORITY(TopPriority, ReadyPriorities)  TopPriority = (31UL - ( u32_t )__clz((ReadyPriorities)))

// 任务栈初始化
u32_t *os_initTaskStack(stack_t *TopOfStack, TaskFunction_t task_entry, void *task_argv);

#define DISABLE_INTERRUPTS()				RaiseBASEPRI()
#define ENABLE_INTERRUPTS()					SetBASEPRI(0)

extern void os_enterCritical( void );
extern void os_exitCritical( void );

#define ENTER_CRITICAL()					os_enterCritical()
#define EXIT_CRITICAL()						os_exitCritical()

#define SET_INTERRUPT_MASK_FROM_ISR()		RaiseBASEPRIReturn()
#define CLEAR_INTERRUPT_MASK_FROM_ISR(x)	SetBASEPRI(x)

#define SY_FULL_READ_WRITE          (15) // 全读写同步屏障

#define NVIC_INT_CTRL_REG		(*((volatile u32_t *)0xe000ed04))    //中断控制寄存器地址
#define NVIC_PENDSVSET_BIT		(1UL << 28UL)                           //PendSV中断挂起位
#define NVIC_SYSPRI2_REG		(*((volatile u32_t *)0xe000ed20))    //系统优先级寄存器2

#define NVIC_PENDSV_PRI					(((u32_t)configMAX_INTERRUPT_PRIORITY_NUM) << 16UL)
#define NVIC_SYSTICK_PRI				(((u32_t)configMAX_INTERRUPT_PRIORITY_NUM) << 24UL)


/*systick配置寄存器*/
#define NVIC_SYSTICK_CTRL_REG			(*((volatile u32_t*)0xe000e010))
#define NVIC_SYSTICK_LOAD_REG			(*((volatile u32_t*)0xe000e014))

/*这里要确保systick的时钟与内核时钟保持一致*/
#define NVIC_SYSTICK_CLK_BIT	(1UL << 2UL)
#define NVIC_SYSTICK_INT_BIT	(1UL << 1UL)
#define NVIC_SYSTICK_ENABLE_BIT	(1UL << 0UL)

#define TRIGGER()											                    \
{																			  \
	/*设置penSV的中断挂起位*/					                                \
	NVIC_INT_CTRL_REG = NVIC_PENDSVSET_BIT;								        \
	__asm volatile("dsb");											\
	__asm volatile("isb");											\
}

u32_t os_startScheduler(void);

#endif /* ARMCM3_H */