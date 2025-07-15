#include "cm3.h"
#include "systick.h"
#include <stdio.h>

#define XPSR   0x01000000  

static u32_t CriticalNesting = 0xaaaaaaaa; //临界区嵌套计数器

void Start_OS(void); // 启动调度器函数声明

static void TaskExitError(void)
{
    for( ; ; );
}

u32_t *os_initTaskStack(stack_t *TopOfStack, TaskFunction_t task_entry, void *task_argv)
{
    /*异常发生时，自动加载到CPU寄存器的内容,只要异常发生，CPU就会自动加载8各字节的栈帧,依次分别为XPSR、PC、LR、R12、R3-R0
    * 我们要再svc中断后返回到任务函数，返回过程中cpu自动把上面8字节的栈内容加载到寄存器中
    * 这里我们手动设置栈帧内容，确保任务函数能正确执行
    */
    TopOfStack--;
    *TopOfStack = XPSR; // 设置XPSR寄存器
    TopOfStack--;
    *TopOfStack = ((stack_t)task_entry) & (stack_t)0xFFFFFFFE; // 设置PC寄存器为任务入口
    TopOfStack--;
    *TopOfStack = (stack_t)TaskExitError; //设置LR寄存器，异常返回地址为TaskExitError函数，应为任务函数一般都是死循环，所以这里一般都用不到
    TopOfStack -= 5;
    *TopOfStack = (stack_t)task_argv; //AAPCS规定R0-R3传递参数

    /*异常发生时，手动加载到CPU寄存器的内容*/
    TopOfStack -= 8; // 留出空间给剩下的R4-R11寄存器

    /*返回栈顶指针，此时TopOfStack指向空闲栈*/
    return TopOfStack;
}

/*启动调度器*/
u32_t os_startScheduler(void)
{
    /*配置PendSV 和 SysTick 的中断优先级为最低*/
    NVIC_SYSPRI2_REG |= NVIC_PENDSV_PRI;
    NVIC_SYSPRI2_REG |= NVIC_SYSTICK_PRI;

    /*初始化Systick */
    os_setupTimer();

	CriticalNesting = 0;
			
	//printf("StartFirstTask\n\r");
    /*启动第一个任务，不再返回*/
    Start_OS();

    /*不应该运行到这里*/
    return 0;
}

// __asm void Start_OS(void)
// {
//     PRESERVE8

//     ldr r0, =0xE000ED08            /*VTOR: 向量表地址寄存器  */         
//     ldr r0, [ r0 ]                 /*得到向量表基地址*/
//     ldr r0, [ r0 ]                 /*得到向量表第 0 项（初始 MSP）*/

//     /* 设置MSP */
//     msr msp, r0
//     /* 全局使能中断 */
//     cpsie i
//     cpsie f
//     dsb
//     isb
//     /* 调用SVC以启动第一个任务 */
//     svc 0
//     nop
//     nop
// }

// __forceinline void SetBASEPRI( u32_t val )
// {
// 	__asm
// 	{
// 		/* 设置BASEPRI寄存器，优先级数值大于该值的中断将被屏蔽 */
// 		msr basepri, val
// 	}
// }


// __forceinline void RaiseBASEPRI( void )
// {

// u32_t val = configMAX_SYSCALL_INTERRUPT_PRIORITY_NUM;

// 	__asm
// 	{
// 		/*屏蔽低优先级中断*/
// 		msr basepri, val
// 		dsb
// 		isb
// 	}
// }


// __forceinline void ClearBASEPRIFromISR( void )
// {
// 	__asm
// 	{
// 		/* 清除中断屏蔽 */
// 		msr basepri, #0
// 	}
// }


// __forceinline u32_t RaiseBASEPRIReturn( void )
// {
//     u32_t ret, val = configMAX_SYSCALL_INTERRUPT_PRIORITY_NUM;

// 	__asm
// 	{
// 		/* 获取当前的BASEPRI值，为之后恢复上下文 */
// 		mrs ret, basepri
// 		msr basepri, val
// 		dsb
// 		isb
// 	}

// 	return ret;
// }
/* 设置BASEPRI，并返回旧值 */

/* 设置 BASEPRI */
__attribute__((always_inline)) inline void SetBASEPRI(u32_t val)
{
    __asm volatile("msr basepri, %0" : : "r"(val) :);
}

/* 屏蔽低优先级中断 */
__attribute__((always_inline)) inline void RaiseBASEPRI(void)
{
    __asm volatile(
        "msr basepri, %0 \n"
        "dsb             \n"
        "isb             \n"
        :
        : "r"(configMAX_SYSCALL_INTERRUPT_PRIORITY_NUM)
        :
    );
}

/* 清除中断屏蔽 */
__attribute__((always_inline)) inline void ClearBASEPRIFromISR(void)
{
    __asm volatile(
        "msr basepri, %0"
        :
        : "r"(0)
        :
    );
}
__attribute__((always_inline)) inline u32_t RaiseBASEPRIReturn(void)
{
    u32_t ret;
    __asm volatile(
        "mrs %0, basepri          \n"
        "msr basepri, %1          \n"
        "dsb                      \n"
        "isb                      \n"
        : "=r"(ret)
        : "r"(configMAX_SYSCALL_INTERRUPT_PRIORITY_NUM)
        :
    );
    return ret;
}


void os_enterCritical( void )
{
	DISABLE_INTERRUPTS();
	CriticalNesting++;

	/*此函数必须在任务上下文中调用，不能在中断中调用*/
	if( CriticalNesting == 1 )
	{
		//configASSERT((CriticalNesting)== 1);
	}
}

void os_exitCritical( void )
{
	//configASSERT(CriticalNesting);
	CriticalNesting--;
    
	if(CriticalNesting == 0)
	{
		ENABLE_INTERRUPTS();
	}
}
