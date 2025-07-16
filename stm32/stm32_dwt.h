#ifndef __STM32_DWT_H
#define __STM32_DWT_H
/*------------------------------- Includes -----------------------------------*/
#include "sys_def.h"
/*---------------------------- Exported typedef ------------------------------*/
/*---------------------------- Exported define -------------------------------*/
/*---------------------------- Exported macro --------------------------------*/
/*---------------------------- Exported variables ----------------------------*/
/*----------------------- Exported function prototypes -----------------------*/
void stm32_dwt_init(void);
void dwt_delay_us(uint32_t time_us);
void dwt_delay_ms(uint32_t time_ms);
#endif /* End of __BSP_DWT_H -------------------------------------------------*/
