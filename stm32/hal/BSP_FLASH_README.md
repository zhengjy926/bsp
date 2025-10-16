# STM32内部Flash驱动使用说明

## 概述

本驱动实现了STM32内部Flash的MTD抽象层接口，支持**F1、F4和G4**系列微控制器。

## 支持的芯片系列

| 系列 | 擦除单元 | 编程单元 | 特点 |
|------|----------|----------|------|
| **STM32F1** | Page（1-2KB） | Half Word（2字节） | 按页擦除 |
| **STM32F4** | Sector（16-128KB） | Word（4字节） | 不均匀扇区 |
| **STM32G4** | Page（2KB） | Double Word（8字节） | 按页擦除 |

## 文件结构

```
drivers/bsp/stm32/hal/
├── bsp_flash.c         # Flash驱动实现
├── bsp_flash.h         # Flash驱动头文件
└── BSP_FLASH_README.md # 本文档

users/
└── bsp_conf.h          # BSP配置文件（包含Flash配置）

devices/
├── inc/mtd.h           # MTD抽象层接口
└── mtd_core.c          # MTD核心实现
```

## 配置方法

### 1. 在 `bsp_conf.h` 中选择芯片系列

```c
/* 选择你的芯片系列（三选一） */
#define SOC_SERIES_STM32F1
// #define SOC_SERIES_STM32F4
// #define SOC_SERIES_STM32G4
```

### 2. 配置Flash参数

**STM32F1系列：**
```c
#define STM32_FLASH_PAGE_NUM        (128UL)      // 根据实际芯片修改
#define STM32_FLASH_USE_NUM         (16)         // 使用的最后16页
```

**STM32F4系列：**
```c
#define STM32_FLASH_SECTOR_NUM      (12UL)       // 根据实际芯片修改
#define STM32_FLASH_USE_SECTORS     (2)          // 使用最后2个扇区
#define STM32_FLASH_START_ADDR      (0x080E0000) // Sector 11起始地址
#define STM32_FLASH_END_ADDR        (0x080FFFFF) // Flash末尾地址
```

**STM32G4系列：**
```c
#define STM32_FLASH_PAGE_NUM        (64UL)       // 根据实际芯片修改
#define STM32_FLASH_USE_NUM         (16)         // 使用的最后16页
```

### 3. 工程包含路径配置

确保以下路径在编译器的包含路径中：
```
devices/inc/
utilities/
users/
drivers/bsp/stm32/hal/
```

## 使用示例

### 1. 基本操作

```c
#include "bsp_flash.h"
#include "mtd.h"

// Flash设备已在 bsp_flash.c 中定义
extern struct mtd_info bsp_flash_info;

void flash_example(void)
{
    uint8_t write_buf[256];
    uint8_t read_buf[256];
    size_t retlen;
    int ret;
    
    // 1. 填充测试数据
    for (int i = 0; i < 256; i++) {
        write_buf[i] = i;
    }
    
    // 2. 擦除（必须先擦除再写入）
    struct erase_info ei = {
        .addr = 0,              // 相对地址
        .len = 4096,            // 擦除4KB
        .fail_addr = MTD_FAIL_ADDR_UNKNOWN,
    };
    
    ret = mtd_erase(&bsp_flash_info, &ei);
    if (ret != 0) {
        printf("擦除失败: %d\n", ret);
        return;
    }
    
    // 3. 写入数据
    ret = mtd_write(&bsp_flash_info, 0, 256, &retlen, write_buf);
    if (ret != 0) {
        printf("写入失败: %d\n", ret);
        return;
    }
    printf("写入成功: %zu 字节\n", retlen);
    
    // 4. 读取数据
    ret = mtd_read(&bsp_flash_info, 0, 256, &retlen, read_buf);
    if (ret != 0) {
        printf("读取失败: %d\n", ret);
        return;
    }
    printf("读取成功: %zu 字节\n", retlen);
    
    // 5. 验证数据
    if (memcmp(write_buf, read_buf, 256) == 0) {
        printf("数据校验成功!\n");
    } else {
        printf("数据校验失败!\n");
    }
}
```

### 2. 存储配置参数示例

```c
#define CONFIG_FLASH_ADDR   0       // 配置区起始地址（相对地址）
#define CONFIG_SIZE         1024    // 配置区大小

typedef struct {
    uint32_t magic;         // 魔数
    uint32_t version;       // 版本号
    uint8_t data[1000];     // 配置数据
    uint32_t crc;           // CRC校验
} config_t;

int save_config(const config_t *config)
{
    size_t retlen;
    int ret;
    
    // 擦除配置区
    struct erase_info ei = {
        .addr = CONFIG_FLASH_ADDR,
        .len = bsp_flash_info.erasesize,
    };
    
    ret = mtd_erase(&bsp_flash_info, &ei);
    if (ret != 0) {
        return ret;
    }
    
    // 写入配置
    ret = mtd_write(&bsp_flash_info, CONFIG_FLASH_ADDR, 
                    sizeof(config_t), &retlen, (const uint8_t*)config);
    
    return ret;
}

int load_config(config_t *config)
{
    size_t retlen;
    int ret;
    
    ret = mtd_read(&bsp_flash_info, CONFIG_FLASH_ADDR, 
                   sizeof(config_t), &retlen, (uint8_t*)config);
    
    if (ret == 0 && config->magic == 0x12345678) {
        return 0;  // 配置有效
    }
    
    return -1;  // 配置无效
}
```

## 关键特性

### 1. 自动地址转换
驱动内部自动将相对地址转换为绝对物理地址：
```c
// 用户使用相对地址
mtd_write(&bsp_flash_info, 0, len, &retlen, buf);

// 驱动内部转换为物理地址
uint32_t phys_addr = STM32_FLASH_START_ADDR + 0;
```

### 2. 统一的错误码
```c
#define EIO         5       // I/O错误
#define EINVAL      22      // 参数无效
#define EROFS       30      // 设备只读
```

### 3. 擦除对齐
不同系列的擦除单元大小不同：
- F1：1-2KB页
- F4：16-128KB扇区（不均匀）
- G4：2KB页

**重要**：擦除地址和长度必须对齐到 `erasesize`！

### 4. 写入对齐
- F1：按2字节对齐（Half Word）
- F4：按4字节对齐（Word）
- G4：按8字节对齐（Double Word）

驱动会自动处理非对齐情况。

## 注意事项

### ⚠️ 必读事项

1. **先擦除再写入**
   ```c
   // ❌ 错误：直接写入
   mtd_write(&bsp_flash_info, addr, len, &retlen, buf);
   
   // ✅ 正确：先擦除
   mtd_erase(&bsp_flash_info, &ei);
   mtd_write(&bsp_flash_info, addr, len, &retlen, buf);
   ```

2. **地址范围检查**
   - 使用相对地址（从0开始）
   - 不要超出配置的Flash区域

3. **中断安全**
   - Flash操作会解锁/加锁
   - 操作期间禁止中断访问Flash代码
   - F1系列：程序在Flash运行时无法擦写Flash

4. **擦除时间**
   | 系列 | 页/扇区擦除时间 |
   |------|----------------|
   | F1   | 20-40ms/page   |
   | F4   | 500-2000ms/sector |
   | G4   | 20-40ms/page   |

5. **写入次数限制**
   - 擦写次数：10,000次（典型值）
   - 建议实现磨损均衡

## 性能优化建议

1. **批量操作**：减少擦除次数，一次写入更多数据
2. **缓存机制**：频繁访问的数据先缓存到RAM
3. **后台擦除**：使用RTOS任务后台擦除
4. **分区管理**：将Flash分为多个区域

## 故障排查

### 问题1：写入失败
**原因**：未先擦除，或地址不在可用范围
**解决**：
```c
// 检查地址范围
if (addr + len > bsp_flash_info.size) {
    printf("地址越界\n");
}

// 先擦除
struct erase_info ei = { .addr = addr, .len = erase_size };
mtd_erase(&bsp_flash_info, &ei);
```

### 问题2：数据校验失败
**原因**：Flash未擦除干净（0xFF）
**解决**：
```c
// 擦除后验证
uint8_t buf[256];
mtd_read(&bsp_flash_info, addr, 256, &retlen, buf);
for (int i = 0; i < 256; i++) {
    if (buf[i] != 0xFF) {
        printf("擦除未完成\n");
        break;
    }
}
```

### 问题3：F1系列无法边运行边写
**原因**：程序在Flash中运行，无法同时读写
**解决**：
- 将Flash操作代码放到RAM执行
- 使用双Bank Flash（如果芯片支持）
- 使用外部存储器运行代码

## 芯片特定说明

### STM32F103系列（常用）
```c
// 64KB: 1KB x 64页
// 128KB: 1KB x 128页  
// 256KB: 2KB x 128页（前64页1KB，后64页2KB）
// 512KB: 2KB x 256页
```

### STM32F407系列（常用）
```c
// Sector 0-3:  16KB  (共64KB)
// Sector 4:    64KB
// Sector 5-11: 128KB (共896KB)
// 总计: 1MB
```

### STM32G431系列（常用）
```c
// 128KB: 2KB x 64页（双Bank）
// 256KB: 2KB x 128页（双Bank）
```

## API参考

参见 `devices/MTD_README.md` 获取完整的MTD接口说明。

## 更新日志

- **V1.1** (2024-11-03): 重构代码，统一F1/F4/G4支持，修复MTD接口适配
- **V1.0** (2025-10-16): 初始版本

## 技术支持

- **MTD文档**：`devices/MTD_README.md`
- **配置文件**：`users/bsp_conf.h`
- **示例代码**：本文档"使用示例"章节

