#include "stm32f10_serial.h"

/* USART1 初始化 */
void USART_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;  // 使能 USART1 和 GPIOA 时钟

    // 配置 PA9 (USART1_TX) 和 PA10 (USART1_RX)
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9 | GPIO_CRH_MODE10 | GPIO_CRH_CNF10); /* 清除原有配置 */
    GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1;  // PA9 设置为复用推挽输出 (TX)
    GPIOA->CRH |= GPIO_CRH_CNF10_0;  // PA10 设置为浮空输入 (RX)

    // 配置 USART1 波特率
    //0x1d4c，波特率 9600
    USART1->BRR =  0x271;// 115200波特率

    // 配置 USART1 为 8 数据位，1 停止位，无校验
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;  // 使能 USART，配置为收发模式
}

// 数据发送
void USART_Transmit(char data)
{
    while (!(USART1->SR & USART_SR_TXE));  // 等待发送缓冲区空
    USART1->DR = data;  // 发送数据
}

// // 重定向 printf 到 USART1
// int fputc(int ch, FILE *f)
// {
//     // 等待 USART1 发送缓冲区空
//     while (!(USART1->SR & USART_SR_TXE));  // 等待发送缓冲区空

//     // 发送数据
//     USART1->DR = (ch & 0xFF);  // 发送数据

//     return ch;  // 返回发送的字符
// }
