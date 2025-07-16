/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxxx.c
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       : 
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32_flash.h"
#include <string.h>
#include "minmax.h"
#include "mymath.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef STM32F429xx
static struct mtd_erase_region_info stm32_erase_regions[] = {
    { .offset = 0x00000000, .erasesize = 0x4000, .numblocks = 4 },  // 16KB x4
    { .offset = 0x00010000, .erasesize = 0x10000, .numblocks = 1 }, // 64KB x1
    { .offset = 0x00020000, .erasesize = 0x20000, .numblocks = 7 }, // 128KB x7
};
#endif
/* Exported variables  -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
static int get_erase_idx(struct mtd_info *mtd, uint32_t addr)
{
    int index = 0;
    struct mtd_erase_region_info *region = NULL;
    uint32_t region_start = 0;
    uint32_t region_end = 0;

    for (int i = 0; i < mtd->numeraseregions; i++) {
        region = &mtd->eraseregions[i];
        region_start = region->offset;
        region_end = region_start + region->erasesize * region->numblocks;
        if (addr < region_end) {
            index += (addr - region_start) / region->erasesize;
            return index;
        }
        index += region->numblocks;
    }
    // 超出范围，返回最后一个扇区索引
    return index - 1;
}

int stm32_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    uint32_t start_addr = instr->addr;
    uint32_t end_addr = start_addr + instr->len - 1;
    int ret = 0;
    uint32_t sector_error;
    uint32_t start_index;
    uint32_t max_to_erase;
    FLASH_EraseInitTypeDef erase_config;

    /* 检查并解锁 Flash */
    if (FLASH->CR & FLASH_CR_LOCK) {
        if (HAL_FLASH_Unlock() != HAL_OK)
            return -1;
    }

    /* 配置擦除参数（仅设置一次） */
#ifdef STM32F429xx
    erase_config.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;  // 根据实际电压配置
#elif STM32G474xx
        /* Clear OPTVERR bit set on virgin samples */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    
    erase_config.TypeErase    = FLASH_TYPEERASE_PAGES;
    erase_config.Banks        = FLASH_BANK_1;
#endif
    

    /* 根据 start_addr 和 end_addr 确定擦除索引和擦除块个数 */
    if (!mtd->eraseregions) {
        start_index = start_addr / mtd->erasesize;
        max_to_erase = (end_addr / mtd->erasesize) - start_index + 1;
    } else {
        start_index = get_erase_idx(mtd, start_addr);
        max_to_erase = get_erase_idx(mtd, end_addr) - start_index + 1;
    }

#ifdef STM32F429xx
    /* 配置擦除参数（sector 索引 + 批量数量） */
    erase_config.Sector = start_index + STM32_FLASH_START_INDEX;  // 假设 sector 从 12 开始
    erase_config.NbSectors = max_to_erase;
#elif STM32G474xx
    erase_config.Page    = start_index + STM32_FLASH_START_INDEX;
    erase_config.NbPages = max_to_erase;
#endif

    /* 禁用中断，执行擦除 */
    __set_PRIMASK(1);
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_config, &sector_error);
    __set_PRIMASK(0);
    
    HAL_FLASH_Lock();  // 重新锁定 Flash
    return ret;
}

int stm32_flash_read(struct mtd_info *mtd, uint32_t from, size_t len, size_t *retlen, uint8_t *buf)
{
    const uint8_t *flash_addr;

    *retlen = 0;

    /* 计算Flash物理内存映射地址 */
    flash_addr = (const uint8_t *)(mtd->offset + from);

    /* 执行内存拷贝 */
    memcpy(buf, flash_addr, len);
    *retlen = len;

    return 0;
}

int stm32_flash_write(struct mtd_info *mtd, uint32_t to, size_t len,
                      size_t *retlen, const uint8_t *buf)
{
    uint32_t current_addr = mtd->offset + to; ///< 逻辑地址转换为物理地址
    size_t i;
    int ret = 0;
    uint32_t writesize = mtd->writesize;

    /* 地址对齐检查 */
    if (current_addr % writesize != 0 || len % writesize != 0)
        return -1;

    /* 检查Flash是否锁定并解锁 */
    if (FLASH->CR & FLASH_CR_LOCK) {
        if (HAL_FLASH_Unlock() != HAL_OK)
            return -1;
    }

    *retlen = 0;
    for (i = 0; i < len; i += writesize) {
        uint64_t data_word = 0xFFFFFFFFFFFFFFFFULL; // 适配最大8字节
        memcpy(&data_word, buf + i, writesize);

        /* 检查目标地址是否已擦除 */
        if (writesize == 8) {
            if (*(volatile uint64_t*)(current_addr + i) != 0xFFFFFFFFFFFFFFFFULL) {
                ret = -1;
                break;
            }
        } else if (writesize == 4) {
            if (*(volatile uint32_t*)(current_addr + i) != 0xFFFFFFFF) {
                ret = -1;
                break;
            }
        } else if (writesize == 4) {
            if (*(volatile uint32_t*)(current_addr + i) != 0xFFFFFFFF) {
                ret = -1;
                break;
            }
        } else {
            ret = -1;
            break;
        }

        __set_PRIMASK(1); /* 禁用中断 */
#if defined(STM32F429xx)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, current_addr + i, (uint32_t)data_word);
#elif defined(STM32G474xx)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, current_addr + i, data_word);
#else
        // 可根据其他芯片系列扩展
        HAL_FLASH_Program((writesize == 8) ? FLASH_TYPEPROGRAM_DOUBLEWORD : FLASH_TYPEPROGRAM_WORD, current_addr + i, data_word);
#endif
        __set_PRIMASK(0);

        /* 验证写入数据 */
        if (writesize == 8) {
            if (*(volatile uint64_t*)(current_addr + i) != data_word) {
                ret = -1;
                break;
            }
        } else if (writesize == 4) {
            if (*(volatile uint32_t*)(current_addr + i) != (uint32_t)data_word) {
                ret = -1;
                break;
            }
        }

        *retlen += writesize;
    }

    HAL_FLASH_Lock();

    return ret;
}

struct mtd_info stm32_flash_info = {
    .name = "stm32_flash",
    .size = STM32_FLASH_USER_SIZE,
    .offset = STM32_FLASH_USER_START_ADDR,
    .flags = MTD_WRITEABLE,
#ifdef STM32F429xx
    .erasesize = 16 * 1024,
    .writesize = 4,
    .numeraseregions = sizeof(stm32_erase_regions) / sizeof(struct mtd_erase_region_info),
    .eraseregions = stm32_erase_regions,
#elif STM32G474xx
    .erasesize = 4 * 1024,
    .writesize = 8,
    .numeraseregions = 0,
    .eraseregions = NULL,
#endif
    ._read = stm32_flash_read,
    ._write = stm32_flash_write,
    ._erase = stm32_flash_erase,
};
/* Private functions ---------------------------------------------------------*/
