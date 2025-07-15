        AREA    |.text|, CODE, READONLY
        THUMB
        REQUIRE8
        PRESERVE8
        IMPORT  os_currentTCB
        IMPORT  os_switchContext
        EXPORT  Start_OS
        EXPORT  SVC_Handler
		EXPORT	PendSV_Handler
			
MAX_SYSCALL_INTERRUPT_PRIORITY_NUM	EQU 0x50
			
Start_OS PROC
        ldr r0, =0xE000ED08             ;VTOR: 向量表地址寄存器           
        ldr r0, [ r0 ]                  ;得到向量表基地址
        ldr r0, [ r0 ]                  ;得到向量表第 0 项（初始 MSP）

        ;设置MSP
        msr msp, r0
        ;全局使能中断
        cpsie i
        cpsie f
        dsb
        isb
        ;调用SVC以启动第一个任务
        svc 0
        nop
        nop
        ENDP

SVC_Handler PROC
        ;加载第一个任务
        ldr r3, =os_currentTCB           ;r3 = &os_currentTCB
        ldr r1, [r3]                     ;获取当前TCB指针
        ldr r0, [r1]                     ;当前任务的栈顶

        ldmia r0!, {r4-r11}            ;把r4到r11的寄存器从PSP栈中弹出,并更新r0指针
        msr psp, r0                     ;更新PSP寄存器,后面异常返回时需要根据psp来获取返回地址
        isb

        mov r0, #0
        msr basepri, r0                 ;清除BASEPRI寄存器，允许所有中断

        orr lr, lr, #0x0d            ;cortex-m3/m4中进入异常时，lr会被更新为EXC_RETURN的值
                                ;然后根据EXC_RETURN的值来确定返回到哪个模式，这里表示返回到线程模式，使用线程栈psp
        bx  lr                     ;这条指令不是传统意义上的pc=lr进行跳转，而是cpu从栈顶弹出之前说的8个寄存器的值
                                        ;取出其中的pc值，把它赋值到pc寄存器中实现程序的跳转
        ENDP


PendSV_Handler PROC
        ;获取旧任务的栈顶
        mrs r0, psp                  ;进入异常前cpu已经自动保存了r0的值，这里可以放心使用
        isb
        ldr r3, =os_currentTCB           ;r3 = &os_currentTCB
        ldr r2, [r3]                     ;获取旧任务TCB指针  

        stmdb r0!, {r4-r11}            ;把r4到r11的寄存器压入PSP栈中，保存旧任务的上下文
        str r0, [r2]                     ;保存旧任务的PSP值到TCB中，因为tcb结构体的第一个值就是栈顶指针
        stmdb sp!, {r3, lr}         ;把r3和lr寄存器压入主栈中,r3为os_currentTCB变量的地址，后面需要os_currentTCB指向新任务

        mov r0, #MAX_SYSCALL_INTERRUPT_PRIORITY_NUM
        msr basepri, r0                 ;设置BASEPRI寄存器，禁止低优先级中断
        dsb
        isb

        bl os_switchContext         ;调用任务切换函数

        mov r0, #0
        msr basepri, r0                 ;清除BASEPRI寄存器，允许所有中断

        ldmia sp!, {r3, lr}         ;从主栈中弹出r3和lr寄存器
        ldr r1, [r3]                     ;获取新的TCB指针
        ldr r0, [r1]                     ;获取新的任务栈顶

        ldmia r0!, {r4-r11}            ;把r4到r11的寄存器从PSP栈中弹出,并更新r0指针
        msr psp, r0                     ;更新PSP寄存器,后面
        isb

        bx lr                     ;上面说了异常返回时lr里面不是程序返回需要的地址，真正的返回地址在栈里面
        nop
        ENDP
		END

