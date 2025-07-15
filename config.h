#ifndef CONFIG_H
#define CONFIG_H

#define configCPU_CLOCK_HZ          72000000UL // CPU时钟频率为72MHz,更改这里之后必须也要更改bsp的systemInit函数中的PLL配置
#define configSYSTICK_CLOCK_HZ     configCPU_CLOCK_HZ // Systick时钟频率等于CPU时钟频率
#define configTICK_RATE_HZ         1000 // 系统节拍频率为1000Hz,即1ms一个节拍

//#define configASSERT(x) if((x)==0)  printf("Error:%s,%d\r\n", __FILE__, __LINE__)
#define configALIGNMENT			    8// 对齐方式，8字节对齐
#define configALIGNMENT_MASK        (configALIGNMENT - 1) // 对齐掩码, 000000000000000000000000000000000000111
#define configTOTAL_HEAP_SIZE       ((size_t)5 * 1024) // 堆大小为 5KB
#define configMAX_TASK_NAME_LEN     20 // 最大任务名称长度
#define configMAX_PRIORITIES		5   //rtOS的优先级数，0-4，值越大优先级越高



#define configPRIO_BITS       		4       // stm32f10只使用4位优先级(0-15),高四位，低四位未使用
                                            //cortex-m3优先级寄存器为8位，理论可以支持256个优先级

#define configMAX_INTERRUPT_PRIORITY_NUM 		    (15 << (8 - configPRIO_BITS))// 最大中断优先级数值，15为最低优先级
#define configMAX_SYSCALL_INTERRUPT_PRIORITY_NUM 	(5 << (8 - configPRIO_BITS))//系统调用的最大中断优先级数值

#endif /* CONFIG_H */