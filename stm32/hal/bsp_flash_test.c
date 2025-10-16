/**
  ******************************************************************************
  * @file        : bsp_flash_test.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2024-11-03
  * @brief       : STM32 internal Flash test code example
  * @attention   : For testing only, remove in production environment
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_flash.h"
#include "mtd.h"
#include <string.h>
#include <stdio.h>

#define  LOG_TAG             "flash_test"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TEST_DATA_SIZE      256

/* Private variables ---------------------------------------------------------*/
extern struct mtd_info bsp_flash_info;

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Flash basic read/write test
  * @retval 0=success, negative=failure
  */
int flash_basic_test(void)
{
    uint8_t write_buf[TEST_DATA_SIZE];
    uint8_t read_buf[TEST_DATA_SIZE];
    size_t retlen;
    int ret;
    
    LOG_I("=== Flash Basic Read/Write Test Started ===");
    LOG_I("Flash Information:");
    LOG_I("  Name: %s", bsp_flash_info.name);
    LOG_I("  Size: %u bytes", (unsigned int)bsp_flash_info.size);
    LOG_I("  Erase Block: %u bytes", bsp_flash_info.erasesize);
    LOG_I("  Write Unit: %u bytes", bsp_flash_info.writesize);
    
    /* 1. Prepare test data */
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        write_buf[i] = (uint8_t)i;
    }
    
    /* 2. Erase Flash */
    LOG_I("Erasing Flash...");
    struct erase_info ei = {
        .addr = 0,
        .len = bsp_flash_info.erasesize,
        .fail_addr = MTD_FAIL_ADDR_UNKNOWN,
    };
    
    ret = mtd_erase(&bsp_flash_info, &ei);
    if (ret != 0) {
        LOG_E("Erase failed: %d", ret);
        return ret;
    }
    LOG_I("Erase successful");
    
    /* 3. Verify erase (should be all 0xFF) */
    LOG_I("Verifying erase...");
    ret = mtd_read(&bsp_flash_info, 0, TEST_DATA_SIZE, &retlen, read_buf);
    if (ret != 0) {
        LOG_E("Read failed: %d", ret);
        return ret;
    }
    
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        if (read_buf[i] != 0xFF) {
            LOG_E("Erase verification failed: offset=%d, value=0x%02X", i, read_buf[i]);
            return -1;
        }
    }
    LOG_I("Erase verification successful");
    
    /* 4. Write data */
    LOG_I("Writing data...");
    ret = mtd_write(&bsp_flash_info, 0, TEST_DATA_SIZE, &retlen, write_buf);
    if (ret != 0) {
        LOG_E("Write failed: %d", ret);
        return ret;
    }
    
    if (retlen != TEST_DATA_SIZE) {
        LOG_E("Write length mismatch: expected=%d, actual=%zu", TEST_DATA_SIZE, retlen);
        return -1;
    }
    LOG_I("Write successful: %zu bytes", retlen);
    
    /* 5. Read data */
    LOG_I("Reading data...");
    memset(read_buf, 0, TEST_DATA_SIZE);
    ret = mtd_read(&bsp_flash_info, 0, TEST_DATA_SIZE, &retlen, read_buf);
    if (ret != 0) {
        LOG_E("Read failed: %d", ret);
        return ret;
    }
    
    if (retlen != TEST_DATA_SIZE) {
        LOG_E("Read length mismatch: expected=%d, actual=%zu", TEST_DATA_SIZE, retlen);
        return -1;
    }
    LOG_I("Read successful: %zu bytes", retlen);
    
    /* 6. Verify data */
    LOG_I("Verifying data...");
    if (memcmp(write_buf, read_buf, TEST_DATA_SIZE) != 0) {
        LOG_E("Data verification failed");
        LOG_E("Write data:");
        for (int i = 0; i < 16; i++) {
            printf("%02X ", write_buf[i]);
        }
        printf("...\n");
        
        LOG_E("Read data:");
        for (int i = 0; i < 16; i++) {
            printf("%02X ", read_buf[i]);
        }
        printf("...\n");
        
        return -1;
    }
    LOG_I("Data verification successful");
    
    LOG_I("=== Flash Basic Read/Write Test Passed ===");
    return 0;
}

/**
  * @brief  Flash boundary test
  * @retval 0=success, negative=failure
  */
int flash_boundary_test(void)
{
    uint8_t buf[16];
    size_t retlen;
    int ret;
    
    LOG_I("=== Flash Boundary Test Started ===");
    
    /* Test out-of-bounds write */
    LOG_I("Testing out-of-bounds write...");
    ret = mtd_write(&bsp_flash_info, bsp_flash_info.size, 16, &retlen, buf);
    if (ret == 0) {
        LOG_E("Out-of-bounds write should fail, but returned success");
        return -1;
    }
    LOG_I("Out-of-bounds write correctly returned error: %d", ret);
    
    /* Test out-of-bounds read */
    LOG_I("Testing out-of-bounds read...");
    ret = mtd_read(&bsp_flash_info, bsp_flash_info.size, 16, &retlen, buf);
    if (ret == 0) {
        LOG_E("Out-of-bounds read should fail, but returned success");
        return -1;
    }
    LOG_I("Out-of-bounds read correctly returned error: %d", ret);
    
    LOG_I("=== Flash Boundary Test Passed ===");
    return 0;
}

/**
  * @brief  Run all Flash tests
  * @retval 0=all passed, negative=some test failed
  */
int run_all_flash_tests(void)
{
    int ret;
    
    LOG_I("========================================");
    LOG_I("     STM32 Flash Driver Test Started");
    LOG_I("========================================");
    
    /* Basic read/write test */
    ret = flash_basic_test();
    if (ret != 0) {
        LOG_E("Basic read/write test failed");
        return ret;
    }
    
    /* Boundary test */
    ret = flash_boundary_test();
    if (ret != 0) {
        LOG_E("Boundary test failed");
        return ret;
    }
    
    LOG_I("========================================");
    LOG_I("         All Tests Passed!");
    LOG_I("========================================");
    
    return 0;
}

/* Private functions ---------------------------------------------------------*/

