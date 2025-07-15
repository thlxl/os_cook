#include "systick.h"

//extern void SysTick_Handler(void);
extern u32_t os_taskIncrementTick(void);

void os_setupTimer(void)
{
    /*设置重装载寄存器的值*/
    NVIC_SYSTICK_LOAD_REG = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;

    /*设置系统定时器的时钟等于内核时钟
      使能Systick 定时器中断
      使能Systick 定时器*/
    NVIC_SYSTICK_CTRL_REG = (NVIC_SYSTICK_CLK_BIT |
                                  NVIC_SYSTICK_INT_BIT |
                                  NVIC_SYSTICK_ENABLE_BIT);

}

void SysTick_Handler(void)
{
    /*关中断*/
    RaiseBASEPRI();

   {
       /*更新系统时基*/
       if(os_taskIncrementTick() != FALSE)
       {
            NVIC_INT_CTRL_REG = NVIC_PENDSVSET_BIT;//挂起PendSV中断
       }

   }
   /*开中断*/
   ClearBASEPRIFromISR();
}

