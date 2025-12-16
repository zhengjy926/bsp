/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_flash.c
  * @author      : ZJY
  * @version     : V1.1
  * @date        : 2024-11-03
  * @brief       : STM32内部Flash驱动实现（支持F1/F4/G4系列）
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.1 : 优化并统一F1/F4/G4系列支持，修复MTD接口适配
  *         V1.0 : 初始版本
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_flash.h"
#include "bsp_conf.h"
#include "mtd.h"
#include "errno-base.h"
#include <string.h>

#define  LOG_TAG             "bsp_flash"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

#ifndef STM32_FLASH_START_ADDR
    #error "Please define the STM32_FLASH_START_ADDR!"
#endif

#ifndef STM32_FLASH_END_ADDR
    #error "Please define the STM32_FLASH_END_ADDR!"
#endif

#ifndef STM32_FLASH_ERASE_SIZE
    #error "Please define the STM32_FLASH_ERASE_SIZE!"
#endif

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/


/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32G4)
/**
  * @brief  获取给定地址所在的页号
  * @param  Addr Flash地址
  * @retval 页号
  */
static uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
    return page;
}

/**
  * @brief  获取给定地址所在的Bank
  * @param  Addr Flash地址
  * @retval Bank编号
  */
static uint32_t GetBank(uint32_t Addr)
{
#if defined(FLASH_BANK_2)
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
        return FLASH_BANK_1;
    } else {
        return FLASH_BANK_2;
    }
#else
    return FLASH_BANK_1;
#endif
}

#else  /* STM32F4 系列 */

/**
  * @brief  获取给定地址所在的扇区号（STM32F4）
  * @param  Address Flash地址
  * @retval 扇区号
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

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

/**
  * @brief  擦除MTD设备
  * @param  mtd MTD设备信息
  * @param  instr 擦除信息
  * @retval 0=成功, 负数=错误码
  */
static int bsp_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    FLASH_EraseInitTypeDef erase_config;
    uint32_t PageError = 0;
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t offs = (uint32_t)instr->addr;
    uint32_t len = (uint32_t)instr->len;
    uint32_t start_addr = (uint32_t)(STM32_FLASH_START_ADDR + offs);
    uint32_t end_addr = (uint32_t)(STM32_FLASH_START_ADDR + offs + len - 1);

    /* 参数校验 */
    if (start_addr < STM32_FLASH_START_ADDR || end_addr > STM32_FLASH_END_ADDR) {
        LOG_E("Flash erase address out of range: 0x%08X - 0x%08X", start_addr, end_addr);
        return -EINVAL;
    }

    /* 解锁Flash */
    if (HAL_FLASH_Unlock() != HAL_OK) {
        LOG_E("Flash unlock failed");
        return -EIO;
    }

    /* 根据芯片系列选择擦除方式 */
#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32G4)
    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    
#if defined(SOC_SERIES_STM32F1)
    erase_config.PageAddress = start_addr;
    erase_config.Banks = GetBank(start_addr);
    erase_config.NbPages = (len + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
#else  /* SOC_SERIES_STM32G4 */
    erase_config.Page = GetPage(start_addr);
    erase_config.Banks = GetBank(start_addr);
    erase_config.NbPages = (len + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
#endif

#else  /* SOC_SERIES_STM32F4 */
    erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_config.Sector = GetSector(start_addr);
    erase_config.NbSectors = GetSector(end_addr) - GetSector(start_addr) + 1;
    erase_config.VoltageRange = VOLTAGE_RANGE_3;
#endif

    /* 执行擦除 */
    status = HAL_FLASHEx_Erase(&erase_config, &PageError);
    
    /* 加锁Flash */
    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        LOG_E("Flash erase failed, status=%d, PageError=%u", status, PageError);
        return -EIO;
    }

    return 0;
}

/**
  * @brief  从MTD设备读取数据
  * @param  mtd MTD设备信息
  * @param  from 起始地址（相对地址）
  * @param  len 读取长度
  * @param  retlen 实际读取长度
  * @param  buf 数据缓冲区
  * @retval 0=成功, 负数=错误码
  */
static int bsp_flash_read(struct mtd_info *mtd, mtd_addr_t from, size_t len, 
                          size_t *retlen, uint8_t *buf)
{
    uint32_t start_addr = (uint32_t)(from + STM32_FLASH_START_ADDR);
    
    /* 参数校验 */
    if (start_addr < STM32_FLASH_START_ADDR || 
        start_addr + len - 1 > STM32_FLASH_END_ADDR) {
        LOG_E("Flash read address out of range: 0x%08X, len=%zu", start_addr, len);
        *retlen = 0;
        return -EINVAL;
    }

    /* Flash读取不需要解锁，直接内存拷贝 */
    memcpy(buf, (const void*)start_addr, len);
    
    *retlen = len;
    return 0;
}

/**
  * @brief  向MTD设备写入数据
  * @param  mtd MTD设备信息
  * @param  to 目标地址（相对地址）
  * @param  len 写入长度
  * @param  retlen 实际写入长度
  * @param  buf 数据缓冲区
  * @retval 0=成功, 负数=错误码
  */
static int bsp_flash_write(struct mtd_info *mtd, mtd_addr_t to, size_t len, 
                           size_t *retlen, const uint8_t *buf)
{
    int ret = 0;
    uint32_t start_addr = to + STM32_FLASH_START_ADDR;
    size_t written = 0;
    
    *retlen = 0;

    /* 参数校验 */
    if (start_addr < STM32_FLASH_START_ADDR || 
        start_addr + len - 1 > STM32_FLASH_END_ADDR) {
        LOG_E("Flash write address out of range: 0x%08X, len=%zu", start_addr, len);
        return -EINVAL;
    }

    /* 解锁Flash */
    if (HAL_FLASH_Unlock() != HAL_OK) {
        LOG_E("Flash unlock failed");
        return -EIO;
    }

    /* 根据芯片系列选择编程方式 */
#if defined(SOC_SERIES_STM32F1)
    /* F1按半字（2字节）编程 */
    for (size_t i = 0; i < len; i += 2) {
        uint16_t data_to_write = 0xFFFF;
        size_t chunk_len = (len - i) >= 2 ? 2 : (len - i);
        
        memcpy(&data_to_write, buf + i, chunk_len);
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, start_addr + i, data_to_write) != HAL_OK) {
            LOG_E("Flash program failed at addr=0x%08X", start_addr + i);
            ret = -EIO;
            break;
        }
        
        written += chunk_len;
    }

#elif defined(SOC_SERIES_STM32F4)
    /* F4按字（4字节）编程 */
    for (size_t i = 0; i < len; i += 4) {
        uint32_t data_to_write = 0xFFFFFFFF;
        size_t chunk_len = (len - i) >= 4 ? 4 : (len - i);
        
        memcpy(&data_to_write, buf + i, chunk_len);
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start_addr + i, data_to_write) != HAL_OK) {
            LOG_E("Flash program failed at addr=0x%08X", start_addr + i);
            ret = -EIO;
            break;
        }
        
        written += chunk_len;
    }

#elif defined(SOC_SERIES_STM32G4)
    /* G4按双字（8字节）编程 */
    for (size_t i = 0; i < len; i += 8) {
        uint64_t data_to_write = 0xFFFFFFFFFFFFFFFFULL;
        size_t chunk_len = (len - i) >= 8 ? 8 : (len - i);
        
        memcpy(&data_to_write, buf + i, chunk_len);
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, start_addr + i, data_to_write) != HAL_OK) {
            LOG_E("Flash program failed at addr=0x%08X", start_addr + i);
            ret = -EIO;
            break;
        }
        
        written += chunk_len;
    }
#else
    #error "Unsupported STM32 series for Flash programming"
#endif

    *retlen = written;
    
    /* 加锁Flash */
    HAL_FLASH_Lock();
    
    return ret;
}

/**
  * @brief  STM32内部Flash MTD设备描述符
  */
struct mtd_info bsp_flash_info = {
    .name = "stm32_flash",
    .type = 0,
    .flags = MTD_WRITEABLE,
    .size = (STM32_FLASH_END_ADDR - STM32_FLASH_START_ADDR + 1),
    .erasesize = STM32_FLASH_ERASE_SIZE,
#if defined(SOC_SERIES_STM32F1)
    .writesize = 2,          /* F1: 半字（2字节） */
    .writesize_shift = 1,
#elif defined(SOC_SERIES_STM32F4)
    .writesize = 4,          /* F4: 字（4字节） */
    .writesize_shift = 2,
#elif defined(SOC_SERIES_STM32G4)
    .writesize = 8,          /* G4: 双字（8字节） */
    .writesize_shift = 3,
#endif
    
    ._read = bsp_flash_read,
    ._write = bsp_flash_write,
    ._erase = bsp_flash_erase,
    
    .priv = NULL,
};

/* Private functions ---------------------------------------------------------*/
