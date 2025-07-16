/**
  ******************************************************************************
  * @file        : pwm.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 2025-05-28
  * @brief       : pwm driver
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.add pwm driver
  *
  *
  ******************************************************************************
  */
#ifndef __PWM_H__
#define __PWM_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"
#include "my_list.h"
/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/
typedef struct pwm_device pwm_t;

/**
 * @brief polarity of a PWM signal
 */
enum pwm_polarity {
	PWM_POLARITY_NORMAL,    /**< 开始为高电平且持续占空比时间，剩余时间为低电平 */
	PWM_POLARITY_INVERSED,  /**< 开始为低电平且持续占空比时间，剩余时间为高电平 */
};

/**
 * @brief PWM 通道状态
 */
struct pwm_state {
	uint32_t period;                /**< 周期（纳秒） */
	uint32_t duty_cycle;            /**< 占空比（纳秒） */
	enum pwm_polarity polarity;     /**< PWM 极性 */
};

/**
 * @brief PWM 设备对象结构体
 */
struct pwm_device {
    const char *name;           /**< 设备标识符，用于调试或日志 */
    uint32_t flags;             /**< 标志位 */
    uint32_t channel;           /**< per-chip relative index of the PWM device */
    struct pwm_chip *chip;      /**< PWM chip providing this PWM device */
    struct pwm_state state;     /**< current applied state */
    bool enabled;               /**< PWM 使能状态 */
    list_t node;               /**< 链表节点 */
};

/**
 * @brief PWM capture data
 */
struct pwm_capture {
	uint32_t period;        /**< period of the PWM signal (in nanoseconds) */
	uint32_t duty_cycle;    /**< duty cycle of the PWM signal (in nanoseconds) */
};

/**
 * @brief PWM 操作函数集合
 * 
 * 定义PWM设备的硬件操作接口，芯片驱动需要实现这些接口。
 */
struct pwm_ops {
	int (*set)(struct pwm_chip *chip, struct pwm_device *pwm,
                 const struct pwm_state *state);  /**< 设置PWM状态 */
    int (*enable)(struct pwm_chip *chip, struct pwm_device *pwm);  /**< 使能PWM输出 */
    int (*disable)(struct pwm_chip *chip, struct pwm_device *pwm); /**< 禁用PWM输出 */
    int (*capture)(struct pwm_chip *chip, struct pwm_device *pwm,
                   struct pwm_capture *result, unsigned long timeout);  /**< 捕获PWM信号 */
};

/**
 * @brief PWM控制器抽象
 */
struct pwm_chip {
	const struct pwm_ops *ops;  /**< 硬件操作函数集合 */
	uint32_t id;                /**< 该 PWM 芯片的唯一编号 */
	uint32_t npwm;              /**< 该芯片控制的 PWM 设备数量 */
	bool atomic;                /**< 标记该驱动的 apply() 方法是否可以在原子上下文中调用 */
	bool operational;           /**< 运行状态标志： true：控制器已注册且可用；false：已注销或初始化失败 */
    void *hw_data;              /**< 硬件相关私有数据 */
};
/* Exported macro ------------------------------------------------------------*/
/* Exported variable prototypes ----------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/
/**
 * @brief 获取PWM设备的当前状态
 * @param pwm: PWM设备指针
 * @param state: 用于存储状态的结构体指针
 * @return 无
 */
static inline void pwm_get_state(const struct pwm_device *pwm,
                                 struct pwm_state *state)
{
	*state = pwm->state;
}

/**
 * @brief 检查PWM设备是否使能
 * @param pwm: PWM设备指针
 * @return true: 已使能; false: 未使能
 */
static inline bool pwm_is_enabled(const struct pwm_device *pwm)
{
    return pwm->enabled;
}

/**
 * @brief 获取PWM周期值（纳秒）
 * @param pwm: PWM设备指针
 * @return 周期值（纳秒）
 */
static inline uint32_t pwm_get_period(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.period;
}

/**
 * @brief 获取PWM占空比值（纳秒）
 * @param pwm: PWM设备指针
 * @return 占空比值（纳秒）
 */
static inline uint32_t pwm_get_duty_cycle(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.duty_cycle;
}

/**
 * @brief 获取PWM极性
 * @param pwm: PWM设备指针
 * @return PWM极性枚举值
 */
static inline enum pwm_polarity pwm_get_polarity(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.polarity;
}

/* PWM 上层调用 APIs */
/**
 * @brief 根据名称查找PWM设备
 * @param name: 设备名称
 * @return 成功返回设备指针，失败返回NULL
 */
struct pwm_device* pwm_find(const char *name);

/**
 * @brief 设置PWM参数
 * @param pwm: PWM设备指针
 * @param state: 要设置的状态参数
 * @return 成功返回0，失败返回负的错误码
 */
int pwm_set(struct pwm_device *pwm, const struct pwm_state *state);

/**
 * @brief 使能PWM输出
 * @param pwm: PWM设备指针
 * @return 成功返回0，失败返回负的错误码
 */
int pwm_enable(struct pwm_device *pwm);

/**
 * @brief 禁用PWM输出
 * @param pwm: PWM设备指针
 * @return 成功返回0，失败返回负的错误码
 */
int pwm_disable(struct pwm_device *pwm);

/* PWM 底层调用 APIs */
/**
 * @brief 注册PWM设备到系统
 * @param pwm: 要注册的PWM设备指针
 * @return 成功返回0，失败返回负的错误码
 */
int pwm_register_device(struct pwm_device *pwm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PWM_H__ */


