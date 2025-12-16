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

#if defined(STM32F1)
    #include "stm32f1xx.h"
#elif defined(SOC_SERIES_STM32F4)
    #include "stm32f4xx.h"
#elif defined(STM32G4)
    #include "stm32g4xx.h"
#else
#error "Please select first the soc series used in your application!"    
#endif

#if USING_RTOS
    #include "FreeRTOS.h"
    #include "task.h"
#endif // USING_RTOS

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
//    __asm volatile(
//        "tst lr, #4                        \n" // EXC_RETURN bit2: 0->MSP, 1->PSP
//        "ite eq                            \n"
//        "mrseq r0, msp                     \n"
//        "mrsne r0, psp                     \n"
//        "mov   r1, lr                      \n" // r1 = EXC_RETURN
//        "b     HardFault_C                 \n"
//    );
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
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
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
