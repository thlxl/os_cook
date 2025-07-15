#include "stm32f10_registers.h"
#include "task.h"

/*led port init*/
void GPIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;  

    GPIOA->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8);  
    GPIOA->CRH |= GPIO_CRH_MODE8_1;
}

/*reverse led*/
void GPIO_Toggle(void)
{
    GPIOA->ODR ^= GPIO_ODR_ODR8; 
}

TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;

/*??1*/
void Task1_Entry( void *p_arg)
{

  //BinarySem_Handle = xSemaphoreCreateBinary();
  for( ; ; )
  {
      GPIO_Toggle();
			os_delay(500);
   }

}

/*??2*/
void Task2_Entry( void *p_arg)
{

  for( ; ; )
  {
		os_delay(10);
  }
}

int main(void){
		GPIO_Init();
	
		if(os_create_task( Task1_Entry, "Task1", 512, NULL, 2, Task1_Handle) != PASS)
		{
				//printf("Task 1 failed to Create\n\r");
		}
		if(os_create_task( Task2_Entry, "Task2", 512, NULL, 2, Task2_Handle) != PASS)
		{
				//printf("Task 1 failed to Create\n\r");
		}
		//	xTaskCreate( Task3_Entry, "Task3", 1024, NULL, 2, Task3_Handle);

		os_taskStartScheduler();

		while(1);

}
