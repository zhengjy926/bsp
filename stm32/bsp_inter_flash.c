/**
 ******************************************************************************
 * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
 * @file        : bsp_inter_flash.c
 * @author      : ZJY
 * @version     : V1.2
 * @date        : 2026-05-19
 * @brief       : STM32内部Flash驱动实现（支持F1/F4/G4系列）
 * @attention   : None
 ******************************************************************************
 * @history     :
 *         V1.0 : 初始版本
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_inter_flash.h"
#include "main.h"
#include "bsp_conf.h"
#include "errno-base.h"
#include <string.h>

#define  LOG_TAG             "bsp_inter_flash"
#define  LOG_LVL            ELOG_LVL_INFO
#include "elog.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

#ifndef FLASH_USER_START_ADDR
    #error "Please define the FLASH_USER_START_ADDR!"
#endif

#ifndef FLASH_USER_END_ADDR
    #error "Please define the FLASH_USER_END_ADDR!"
#endif

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t flash_size_kb;
static uint32_t flash_max_addr;
static uint32_t bank_size_bytes;
static uint32_t page_size_bytes;
static bool     dbank_is_enbale;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void BSP_InterFlash_Init(void)
{
    uint32_t total_size_bytes;
    
    flash_size_kb = *(volatile uint16_t *)(FLASH_SIZE_DATA_REGISTER);
    log_d("Flash size = %dKb", flash_size_kb);
    
    total_size_bytes = (uint32_t)flash_size_kb * 1024U;
    
    flash_max_addr = FLASH_BASE + flash_size_kb * 1024U - 1U;
    log_d("Flash max addr = 0x%08X", flash_max_addr);
    
#if defined(FLASH_BANK_2)
    if (FLASH->OPTR & FLASH_OPTR_DBANK) {
        dbank_is_enbale = true;
        bank_size_bytes = total_size_bytes / 2U;
#if defined(STM32G4)
        page_size_bytes = 0x800;    /* 2 KB */
#endif
    } else {
        dbank_is_enbale = false;
        bank_size_bytes = total_size_bytes;
#if defined(STM32G4)
        page_size_bytes = 0x1000U; /* 4 KB */
#endif
    }
#endif
    
    if (FLASH_USER_START_ADDR > flash_max_addr || 
        FLASH_USER_END_ADDR > flash_max_addr) {
        log_w("User start addr or end addr out of range:0x%08X,0x%08X", FLASH_USER_START_ADDR, FLASH_USER_END_ADDR);
    }
    log_d("Flash bank size = %d", FLASH_BANK_SIZE);
    log_d("Flash page number = %d", FLASH_PAGE_NB);
}

#if defined(STM32F1) || defined(STM32G4)
/**
  * @brief  获取给定地址所在的页号
  * @param  Addr Flash地址
  * @retval 页号
  */
static uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;

    // 判断是否激活了双 Bank 模式
    if (dbank_is_enbale) {
        if (Addr < (FLASH_BASE + bank_size_bytes)) {
            // 落在 Bank 1，直接计算相对页码
            page = (Addr - FLASH_BASE) / 2048;
        } else {
            // 落在 Bank 2，需要减去 Bank 1 的偏移量，计算在 Bank 2 内部的相对页码
            page = (Addr - (FLASH_BASE + bank_size_bytes)) / 2048;
        }
    } else {
        // 【单 Bank 模式】：单页大小固定为 4 KB (4096 字节)
        page = (Addr - FLASH_BASE) / 4096;
    }

    return page;
}

/**
  * @brief  获取给定地址所在的Bank
  * @param  Addr Flash地址
  * @retval Bank编号
  */
static uint32_t GetBank(uint32_t Addr)
{
    // 判断是否激活了双 Bank 模式
    if (dbank_is_enbale)
    {
        // 比较地址是否超过了 Bank 1 的范围
        if (Addr < (FLASH_BASE + bank_size_bytes)) {
            return FLASH_BANK_1;
        } else {
            return FLASH_BANK_2;
        }
    } else {
        // 单 Bank 模式下，所有有效物理地址均属于 Bank 1
        return FLASH_BANK_1;
    }
}

#else  /* STM32F4 系列 */
/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_SECTOR_11;
  }
  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_SECTOR_12;
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_SECTOR_13;
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_SECTOR_14;
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_SECTOR_15;
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_SECTOR_16;
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_SECTOR_17;
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_SECTOR_18;
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_SECTOR_19;
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_SECTOR_20;
  }
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_SECTOR_21;
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_SECTOR_22;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
  {
    sector = FLASH_SECTOR_23;
  }  
  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
static uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;
  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) ||\
     (Sector == FLASH_SECTOR_3) || (Sector == FLASH_SECTOR_12) || (Sector == FLASH_SECTOR_13) ||\
     (Sector == FLASH_SECTOR_14) || (Sector == FLASH_SECTOR_15))
  {
    sectorsize = 16 * 1024;
  }
  else if((Sector == FLASH_SECTOR_4) || (Sector == FLASH_SECTOR_16))
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }  
  return sectorsize;
}
#endif


int32_t BSP_InterFlash_EraseSector(uint32_t start_addr, uint32_t end_addr)
{
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0U;
    HAL_StatusTypeDef status;
    uint32_t FirstPage = 0, NbOfPages = 0;
    
    if (start_addr > flash_max_addr || end_addr > flash_max_addr) {
        return -ERR_INVAL;
    }

    if ((start_addr > end_addr) || (start_addr < FLASH_USER_START_ADDR) ||
        (end_addr > FLASH_USER_END_ADDR)) {
        log_e("Flash erase range invalid: 0x%08X - 0x%08X", start_addr, end_addr);
        return -ERR_INVAL;
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        log_e("Flash unlock failed");
        return -ERR_IO;
    }

#if defined(STM32F1) || defined(STM32G4)
    {
        /* Get the 1st page to erase */
        FirstPage = GetPage(start_addr);

        /* Get the number of pages to erase from 1st page */
        NbOfPages = GetPage(end_addr) - FirstPage + 1;

        erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_config.Banks     = GetBank(start_addr);
#if defined(STM32F1)
        erase_config.PageAddress = start_addr;
#else
        erase_config.Page      = FirstPage;
#endif
        erase_config.NbPages   = NbOfPages;
    }

#elif defined(STM32F4)
    erase_config.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_config.VoltageRange = VOLTAGE_RANGE_3;
    erase_config.Sector       = GetSector(start_addr);
    erase_config.NbSectors    = GetSector(end_addr) - erase_config.Sector + 1U;
#else
    #error "No support MCU series!"
#endif

    status = HAL_FLASHEx_Erase(&erase_config, &page_error);
    if (status != HAL_OK) {
        log_e("Flash erase failed, status=%d, PageError=%u", (int32_t)status, page_error);
        return -ERR_IO;
    }

    HAL_FLASH_Lock();
    
    return 0;
}

int32_t BSP_InterFlash_Read(uint32_t addr, void *buf, uint32_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return -ERR_INVAL;
    }

    if ((addr < FLASH_USER_START_ADDR) || ((addr + len - 1U) > FLASH_USER_END_ADDR)) {
        log_e("Flash read address out of range: 0x%08X, len=%u", addr, len);
        return -ERR_INVAL;
    }

    /* STM32内部Flash为内存映射，可直接读取 */
    (void)memcpy(buf, (const void *)addr, len);

    return 0;
}

int32_t BSP_InterFlash_Write(uint32_t addr, const void *buf, uint32_t len)
{
    int32_t  ret = 0;
    uint32_t i   = 0U;
    HAL_StatusTypeDef status = HAL_OK;

    if ((buf == NULL) || (len == 0U)) {
        return -ERR_INVAL;
    }

    if ((addr < FLASH_USER_START_ADDR) || ((addr + len - 1U) > FLASH_USER_END_ADDR)) {
        log_e("Flash write address out of range: 0x%08X, len=%u", addr, len);
        return -ERR_INVAL;
    }
    
     /* 写入地址必须满足一定的字节对齐要求  */
#if defined(STM32F1)
    if (addr % 2 != 0) {
        return -ERR_INVAL;
    }
#elif defined(STM32F4)
    /* 确保供电 */
    if (addr % 4 != 0) {
        return -ERR_INVAL;
    }
#elif defined(STM32G4)
    if (addr % 8 != 0) {
        return -ERR_INVAL;
    }
#else
    #error "Unsupported STM32 series for Flash programming"
#endif
    
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        log_e("Flash unlock failed = %d", status);
        return -ERR_IO;
    }

#if defined(STM32F1)
    for (i = 0U; i < len; i += 2U) {
        uint16_t data  = 0xFFFFU;
        uint32_t chunk = ((len - i) >= 2U) ? 2U : (len - i);

        (void)memcpy(&data, buf + i, (size_t)chunk);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, data);
        if (status != HAL_OK) {
            log_e("Flash program failed at 0x%08X, status = %d, errno code = %d", addr + i, status, HAL_FLASH_GetError());
            ret = -ERR_IO;
            break;
        }
    }

#elif defined(STM32F4)
    for (i = 0U; i < len; i += 4U) {
        uint32_t data  = 0xFFFFFFFFU;
        uint32_t chunk = ((len - i) >= 4U) ? 4U : (len - i);

        (void)memcpy(&data, buf + i, (size_t)chunk);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, data);
        if (status != HAL_OK) {
            log_e("Flash program failed at 0x%08X, status = %d, errno code = %d", addr + i, status, HAL_FLASH_GetError());
            ret = -ERR_IO;
            break;
        }
    }

#elif defined(STM32G4)
    for (i = 0U; i < len; i += 8U) {
        uint64_t data  = 0xFFFFFFFFFFFFFFFFULL;
        uint32_t chunk = ((len - i) >= 8U) ? 8U : (len - i);

        (void)memcpy(&data, buf + i, (size_t)chunk);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, data);
        if (status != HAL_OK) {
            log_e("Flash program failed at 0x%08X, status = %d, errno code = %d", addr + i, status, HAL_FLASH_GetError());
            ret = -ERR_IO;
            break;
        }
    }
#else
    #error "Unsupported STM32 series for Flash programming"
#endif

    HAL_FLASH_Lock();

    return ret;
}

/* Private functions ---------------------------------------------------------*/
