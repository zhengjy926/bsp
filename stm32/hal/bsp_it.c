/**
 ******************************************************************************
 * @file    bsp_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_it.h"
#include "bsp_conf.h"

#if defined(SOC_SERIES_STM32F1)
    #include "stm32f1xx.h"
#elif defined(SOC_SERIES_STM32F4)
    #include "stm32f4xx.h"
#elif defined(SOC_SERIES_STM32G4)
    #include "stm32g4xx.h"
#else
#error "Please select first the soc series used in your application!"    
#endif

#if USING_RTOS
    #include "FreeRTOS.h"
    #include "task.h"
#endif // USING_RTOS

#define  LOG_TAG             "bsp_it"
#define  LOG_LVL             3
#include "log.h"

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;     // stacked LR
    uint32_t pc;     // stacked PC
    uint32_t xpsr;   // stacked xPSR

    uint32_t exc_lr; // EXC_RETURN (进入 HardFault 时的 LR)
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t mmfar;
    uint32_t bfar;
} fault_snapshot_t;

volatile fault_snapshot_t g_fault;

void HardFault_C(uint32_t *sp, uint32_t exc_lr)
{
    // 1) 保存堆栈现场（入栈顺序固定）
    g_fault.r0   = sp[0];
    g_fault.r1   = sp[1];
    g_fault.r2   = sp[2];
    g_fault.r3   = sp[3];
    g_fault.r12  = sp[4];
    g_fault.lr   = sp[5];
    g_fault.pc   = sp[6];
    g_fault.xpsr = sp[7];
    g_fault.exc_lr = (uint32_t)exc_lr;

    // 2) 保存 SCB 故障寄存器
    g_fault.cfsr  = SCB->CFSR;
    g_fault.hfsr  = SCB->HFSR;
    g_fault.dfsr  = SCB->DFSR;
    g_fault.afsr  = SCB->AFSR;
    g_fault.mmfar = SCB->MMFAR;
    g_fault.bfar  = SCB->BFAR;

    // 3) 这里你可以用 RTT/UART 打印 g_fault，或写入备份 RAM/Flash 做崩溃日志
    //    调试阶段：直接在这里打断点，然后看 g_fault 的值
    LOG_E("=== HardFault Detected ===");
    LOG_E("PC (Addr): 0x%08X", g_fault.pc);   // 崩溃指令地址
    LOG_E("LR (Call): 0x%08X", g_fault.lr);   // 调用者地址
    LOG_E("CFSR     : 0x%08X", g_fault.cfsr); // 核心故障状态寄存器 (最重要)
    LOG_E("HFSR     : 0x%08X", g_fault.hfsr);
    LOG_E("BFAR     : 0x%08X", g_fault.bfar); // 如果是总线错误，这里是访问的错误地址
    LOG_E("MMFAR    : 0x%08X", g_fault.mmfar);// 如果是存储错误，这里是访问的错误地址
    
    LOG_E("=== Stack Dump ===");
    // 打印 SP 指针后的 16 个字 (64字节)
    // 这里面很可能包含“父函数”的返回地址 (0x08xxxxxx)
    for (int i = 0; i < 16; i++) {
        LOG_E("SP + %02d : 0x%08X", i*4, sp[i]);
    }

    while (1) {
        __asm volatile("nop");
    }
}

/* External variables --------------------------------------------------------*/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4                        \n" // EXC_RETURN bit2: 0->MSP, 1->PSP
        "ite eq                            \n"
        "mrseq r0, msp                     \n"
        "mrsne r0, psp                     \n"
        "mov   r1, lr                      \n" // r1 = EXC_RETURN
        "b     HardFault_C                 \n"
    );
    while (1)
    {
    };
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
    
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
    
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
    
}

/**
 * @brief This function handles System tick timer.
 */
#if !USING_RTOS
void SysTick_Handler(void)
{
    HAL_IncTick();
}
#endif // USING_RTOS
