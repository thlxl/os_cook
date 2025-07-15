/*
* This file is part of the OS_Cook project.
* 配置stm32f10x的系统时钟为外部晶振8MHz，PLL倍频9倍，系统时钟72MHz
* HSE：外部高速时钟（晶振）
* RCC：复位和时钟控制器
* HCLK：高速总线时钟
* PCLK1：APB1总线时钟
* PCLK2：APB2总线时钟
* AHB：高级总线
* APB1：低速外设总线1
* APB2：低速外设总线2
* PLL：相位锁定环，用于倍频
* FLASH：闪存控制器
*/
#include "stm32f10_registers.h"

#define HSE_STARTUP_TIMEOUT    ((unsigned int)0x0500)   /*!< HSE启动超时时间 */

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

static void SetSysClock(void)
{
  volatile unsigned int StartUpCounter = 0, HSEStatus = 0;
    
  /* 使能外部高速晶振 (HSE) */    
  RCC->CR |= ((unsigned int)RCC_CR_HSEON);
 
  /* 等待 HSE 就绪，带超时退出机制 */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;     /*检测HSE，是否就绪*/
    StartUpCounter++;                       /*超时计数器加1*/
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (unsigned int)0x01; /* HSE 就绪 */
  }
  else
  {
    HSEStatus = (unsigned int)0x00; /* HSE 启动失败 */
  }  

  if (HSEStatus == (unsigned int)0x01)
  {
    /* 开启 Flash 预取功能 */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /* 置 Flash 延时为 2 等待周期 */
    FLASH->ACR &= (unsigned int)((unsigned int)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (unsigned int)FLASH_ACR_LATENCY_2;

 
    /* 设置 AHB 总线时钟（HCLK） = SYSCLK，不分频 */
    RCC->CFGR |= (unsigned int)RCC_CFGR_HPRE_DIV1;

    /* 设置 APB2总线时钟（PCLK2） = HCLK，不分频 */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE2_DIV1;

    /* 设置 APB1总线时钟（PCLK1） = HCLK / 2，最大不能超过 36MHz */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE1_DIV2;


    /* 配置 PLL：PLLCLK = HSE * 9 = 8MHz * 9 = 72MHz 
    *  清除原来的 PLL 源与倍频因子
    *  设置为：以 HSE 为 PLL 输入源，倍频因子为 x9*/
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                        RCC_CFGR_PLLMULL));
    RCC->CFGR |= (unsigned int)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);


    /* 使能 PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* 等待 PLL 就绪 */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    /* 选择 PLL 作为系统时钟源 */
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_SW));
    RCC->CFGR |= (unsigned int)RCC_CFGR_SW_PLL;

    /* 等待 PLL 被选为系统时钟源 */
    while ((RCC->CFGR & (unsigned int)RCC_CFGR_SWS) != (unsigned int)0x08)
    {
    }
  }
  else
  { 
    /* 如果 HSE 启动失败，可在此处理异常 */
  }
}

void SystemInit (void)
{
  /* 打开内部高速时钟 HSI（默认打开） */
  RCC->CR |= (unsigned int)0x00000001;

  /* 重置时钟配置寄存器 CFGR 到默认值（只保留保留位） */
  RCC->CFGR &= (unsigned int)0xF8FF0000;

  /* 重置 HSEON, CSSON 和 PLLON 位 */
  RCC->CR &= (unsigned int)0xFEF6FFFF;

  /* 清除 HSEBYP（绕过 HSE） */
  RCC->CR &= (unsigned int)0xFFFBFFFF;

  /* 清除 PLLSRC/PLLXTPRE/PLLMUL/USBPRE */
  RCC->CFGR &= (unsigned int)0xFF80FFFF;

  /* 禁用 RCC 所有中断并清除 pending 标志位  */
  RCC->CIR = 0x009F0000;

  /* 配置系统时钟频率、HCLK、PCLK2 和 PCLK1 预分频器 */
  /* 配置 Flash 延时周期并启用预取缓冲区 */
  SetSysClock();

#ifdef VECT_TAB_SRAM
  //SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /*向量表偏移重定位到 SRAM */
#else
  //SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* 向量表偏移重定位到内部 FLASH */
#endif

}
