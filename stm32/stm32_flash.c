/**
  ******************************************************************************
  * @file        : stm32_flash.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 202x-xx-xx
  * @brief       : xxx
  * @attention   : xxx
  ******************************************************************************
  * @history     :
  *         V1.0 : 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32_flash.h"
#include "mtd_core.h"
#include "board.h"
#include <string.h>

#define  LOG_TAG             "stm32_flash"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
#ifndef STM32_FLASH_START_ADDR
    #error "Please define the STM32_FLASH_START_ADDR!"
#endif

#ifndef STM32_FLASH_END_ADDR
    #error "Please define the STM32_FLASH_END_ADDR!"
#endif

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
#if defined(SOC_SERIES_STM32F1)
/**
 * @brief  (STM32F1) 获取给定地址所在的页。
 */
static uint32_t GetPage(uint32_t Addr)
{
    return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}
#else
/**
 * @brief  (STM32F4/G4/...) 获取给定地址所在的扇区。
 * @note   此函数是示例，需要根据具体芯片手册的Sector分布来调整。
 */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    // 以STM32F407为例，其扇区大小不均匀
    if((Address < 0x08003FFF) && (Address >= 0x08000000)) { sector = FLASH_SECTOR_0; }
    else if((Address < 0x08007FFF) && (Address >= 0x08004000)) { sector = FLASH_SECTOR_1; }
    else if((Address < 0x0800BFFF) && (Address >= 0x08008000)) { sector = FLASH_SECTOR_2; }
    else if((Address < 0x0800FFFF) && (Address >= 0x0800C000)) { sector = FLASH_SECTOR_3; }
    else if((Address < 0x0801FFFF) && (Address >= 0x08010000)) { sector = FLASH_SECTOR_4; }
    else if((Address < 0x0803FFFF) && (Address >= 0x08020000)) { sector = FLASH_SECTOR_5; }
    else if((Address < 0x0805FFFF) && (Address >= 0x08040000)) { sector = FLASH_SECTOR_6; }
    else if((Address < 0x0807FFFF) && (Address >= 0x08060000)) { sector = FLASH_SECTOR_7; }
    else if((Address < 0x0809FFFF) && (Address >= 0x08080000)) { sector = FLASH_SECTOR_8; }
    else if((Address < 0x080BFFFF) && (Address >= 0x080A0000)) { sector = FLASH_SECTOR_9; }
    else if((Address < 0x080DFFFF) && (Address >= 0x080C0000)) { sector = FLASH_SECTOR_10; }
    else { sector = FLASH_SECTOR_11; }
    
    return sector;
}
#endif

int stm32_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    FLASH_EraseInitTypeDef erase_config;
    uint32_t PageError = 0;
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t offs = (uint32_t)instr->addr;
    uint32_t len = (uint32_t)instr->len;
    uint32_t start_addr = (uint32_t)(STM32_FLASH_START_ADDR + offs);
    uint32_t end_addr = (uint32_t)(STM32_FLASH_START_ADDR + offs + len - 1);

    /* 解锁Flash */
    HAL_FLASH_Unlock();

    /* 根据芯片系列选择擦除方式 */
#if defined(SOC_SERIES_STM32F1)
    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.PageAddress = start_addr;
    erase_config.NbPages = (end_addr - start_addr) / FLASH_PAGE_SIZE + 1;
#else
    erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_config.Sector = GetSector(start_addr);
    erase_config.NbSectors = GetSector(end_addr) - GetSector(start_addr) + 1;
    erase_config.VoltageRange = VOLTAGE_RANGE_3;
#endif

    /* 执行擦除 */
    status = HAL_FLASHEx_Erase(&erase_config, &PageError);
    
    /* 加锁Flash */
    HAL_FLASH_Lock();
    return status;
}

int stm32_flash_read(struct mtd_info *mtd, uint64_t from, size_t len, size_t *retlen, uint8_t *buf)
{
    uint32_t start_addr = (uint32_t)(from + STM32_FLASH_START_ADDR);
    
    memcpy(buf, (const void*)start_addr, len);
    
    *retlen = len;
    return len;
}

int stm32_flash_write(struct mtd_info *mtd, uint64_t to, size_t len, size_t *retlen, const uint8_t *buf)
{
    int ret = 0;
    uint32_t start_addr = to + STM32_FLASH_START_ADDR;
    
    
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return -1;
    }
    
    for (uint32_t i = 0; i < len; i += 4)
    {
        uint32_t data_to_write = 0xFFFFFFFF;
        
        // 将多个字节的数据合并到一个变量中
        memcpy(&data_to_write, buf + i, (len - i) >= 4 ? 4 : (len - i));
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start_addr + i, data_to_write) != HAL_OK) {
            LOG_E("HAL_FLASH_Program failed at addr=0x%08x", start_addr + i);
            ret = -1;
            break; // 如果发生错误，则停止写入
        }
    }
    *retlen = len;
    
    HAL_FLASH_Lock();
    
    return ret;
}

struct mtd_info stm32_flash_info = {
    .name = "stm32_flash",
    .flags = MTD_WRITEABLE,
    .size = (uint64_t)(STM32_FLASH_END_ADDR - STM32_FLASH_START_ADDR),
#if defined(SOC_SERIES_STM32F1)
    .erasesize = FLASH_PAGE_SIZE,
#else
    .erasesize = STM32_FLASH_ERASE_SIZE,
#endif
    .writesize = 1,
    .writebufsize = 4,
    ._read = stm32_flash_read,
    ._write = stm32_flash_write,
    ._erase = stm32_flash_erase,
};
/* Private functions ---------------------------------------------------------*/


