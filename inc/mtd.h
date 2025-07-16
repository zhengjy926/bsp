/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxx.h
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
#ifndef __MTD_H__
#define __MTD_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
/* Exported define -----------------------------------------------------------*/
#define MTD_WRITEABLE		0x400	/* Device is writeable */
#define MTD_BIT_WRITEABLE	0x800	/* Single bits can be flipped */
#define MTD_NO_ERASE		0x1000	/* No erase necessary */
/* Exported typedef ----------------------------------------------------------*/
struct mtd_info;

struct erase_info {
	uint32_t addr;
	uint32_t len;
    uint32_t fail_addr;
};

struct mtd_erase_region_info {
	uint32_t offset;		/* At which this region starts, from the beginning of the MTD */
	uint32_t erasesize;		/* For this region */
	uint32_t numblocks;		/* Number of blocks of erasesize in this region */
};

struct mtd_req_stats {
	unsigned int uncorrectable_errors;
	unsigned int corrected_bitflips;
	unsigned int max_bitflips;
};

/**
 * struct mtd_ecc_stats - error correction stats
 *
 * @corrected:	number of corrected bits
 * @failed:	number of uncorrectable errors
 * @badblocks:	number of bad blocks in this partition
 * @bbtblocks:	number of blocks reserved for bad block tables
 */
struct mtd_ecc_stats {
	uint32_t corrected;
	uint32_t failed;
	uint32_t badblocks;
	uint32_t bbtblocks;
};

/**
 * MTD operation modes
 *
 * @MTD_OPS_PLACE_OOB:	OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:	OOB data are automatically placed at the free areas
 *			which are defined by the internal ecclayout
 * @MTD_OPS_RAW:	data are transferred as-is, with no error correction;
 *			this mode implies %MTD_OPS_PLACE_OOB
 *
 * These modes can be passed to ioctl(MEMWRITE) and ioctl(MEMREAD); they are
 * also used internally. See notes on "MTD file modes" for discussion on
 * %MTD_OPS_RAW vs. %MTD_FILE_MODE_RAW.
 */
enum {
	MTD_OPS_PLACE_OOB = 0,
	MTD_OPS_AUTO_OOB = 1,
	MTD_OPS_RAW = 2,
};

struct mtd_oob_ops {
    unsigned int    mode;           ///< 操作模式
    size_t          len;            ///< 主数据区操作长度（字节）
    size_t          retlen;         ///< 实际处理的主数据长度（输出参数）
    size_t          ooblen;         ///< OOB 区操作长度（字节）
    size_t          oobretlen;      ///< 实际处理的 OOB 数据长度（输出参数）
    uint32_t        ooboffs;        ///< OOB 区操作的起始偏移（字节）
    uint8_t         *datbuf;        ///< 主数据缓冲区指针
    uint8_t         *oobbuf;        ///< OOB 数据缓冲区指针
    struct mtd_req_stats *stats;    ///< 统计信息（如 ECC 错误计数）
};

/**
 * @brief Flash information
 */
struct mtd_info {
    const char *name;
    uint32_t flags;         ///< 设备标志位
    uint32_t size;          ///< 设备总容量（字节）
    uint32_t offset;        ///< 物理地址偏移
    uint32_t erasesize;     ///< 擦除大小（字节）
    uint32_t writesize;     ///< 最小写入单元（字节）
    uint32_t writebufsize;  ///< 写入缓冲区大小。某些设备允许一次性写入多页数据，该值通常等于或大于 writesize
    
    uint32_t oobsize;   // 每个 block 的 OOB (Out-of-Band) 区域大小（用于存储 ECC、元数据等） (e.g. 16)
	uint32_t oobavail;  // 每个 block 用户可用的 OOB 字节数（部分保留给驱动或硬件使用）
    
	/*
	 若 erasesize 或 writesize 是 2 的幂次，则记录对应的位移值，
     用于快速计算地址掩码（代替除法运算）,否则值为0。
	 */
	uint32_t erasesize_shift;
	uint32_t writesize_shift;
    
	/* 基于 erasesize_shift 和 writesize_shift 生成的掩码，用于地址对齐操作 */
	uint32_t erasesize_mask;
	uint32_t writesize_mask;
    
    uint32_t bitflip_threshold; ///< 比特翻转阈值。当 ECC 纠正的比特数超过此值时，系统认为数据可能不可靠
    
	/* 若设备包含多个不同大小的擦除区域（非均匀擦除块），
       eraseregions 指向 struct mtd_erase_region_info 数组，描述每个区域的起始偏移和块大小.
	 */
	int numeraseregions;
	struct mtd_erase_region_info *eraseregions;
    
    int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
	int (*_read) (struct mtd_info *mtd, uint32_t from, size_t len,
		      size_t *retlen, uint8_t *buf);
	int (*_write) (struct mtd_info *mtd, uint32_t to, size_t len,
		       size_t *retlen, const uint8_t *buf);
	int (*_read_oob) (struct mtd_info *mtd, uint32_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, uint32_t to,
			   struct mtd_oob_ops *ops);
    int (*_lock) (struct mtd_info *mtd, uint32_t ofs, uint64_t len);
	int (*_unlock) (struct mtd_info *mtd, uint32_t ofs, uint64_t len);
    int (*_is_locked) (struct mtd_info *mtd, uint32_t ofs, uint64_t len);
    
	uint32_t ecc_step_size; ///< ECC 校验的步长（每个 ECC 块的大小）
	uint32_t ecc_strength;  ///< 每个 ECC 块最多可纠正的比特错误数
	struct mtd_ecc_stats ecc_stats; ///< 用于统计 ECC 纠正的比特错误数、失败次数等信息
    
    void *priv;
};
/* Exported macro ------------------------------------------------------------*/
/* Exported variable prototypes ----------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/
int mtd_read(struct mtd_info *mtd, uint32_t from, size_t len, size_t *retlen, uint8_t *buf);
int mtd_read_oob(struct mtd_info *mtd, uint32_t from, struct mtd_oob_ops *ops);
int mtd_write(struct mtd_info *mtd, uint32_t to, size_t len, size_t *retlen,
                const uint8_t *buf);
int mtd_write_oob(struct mtd_info *mtd, uint32_t to, struct mtd_oob_ops *ops);
int mtd_erase(struct mtd_info *mtd, struct erase_info *instr);

static inline uint32_t mtd_oobavail(struct mtd_info *mtd, struct mtd_oob_ops *ops)
{
	return ops->mode == MTD_OPS_AUTO_OOB ? mtd->oobavail : mtd->oobsize;
}

static inline uint32_t mtd_div_by_ws(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->writesize_shift)
		return sz >> mtd->writesize_shift;
	sz /=  mtd->writesize;
	return sz;
}

static inline uint32_t mtd_mod_by_ws(uint64_t sz, struct mtd_info *mtd)
{
	if (mtd->writesize_shift)
		return sz & mtd->writesize_mask;
    sz /=  mtd->writesize;
	return sz;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MTD_H__ */

