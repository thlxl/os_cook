#ifndef TYPE_H
#define TYPE_H

#define NULL    ((void *)0)
#define TRUE        1
#define FALSE       0
#define PASS        0

typedef unsigned int  stack_t;  // 栈类型定义为无符号整数
typedef unsigned int  clock_t;  // 时钟类型定义为无符号整数

typedef unsigned int  u32_t;  // 基础类型定义为无符号整数
typedef unsigned short int  u16_t;  // 16位无符号整数类型定义
typedef unsigned char  u8_t;  // 8位无符号整数类型定义  
typedef signed char s8_t;

typedef u32_t  size_t;  // 大小类型定义为无符号整数


typedef void (*TaskFunction_t)(void *);  // 任务函数指针类型定义
typedef void * TaskHandle_t;  // 任务句柄类型定义
#endif
