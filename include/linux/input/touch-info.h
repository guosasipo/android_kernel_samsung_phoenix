
#ifndef _TOUCH_INFO_H_
#define _TOUCH_INFO_H_

#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/input.h>
#include <linux/seq_file.h>

/**
 * Lock down info format:
 *   XX XX XX XX XXXX XX XX (8 bytes)
 *    |  |  |  |   |   |  |
 *    |  |  |  |   |   |  +----> [7] Reservation byte default = 0
 *    |  |  |  |   |   +-------> [6] CG maker
 *    |  |  |  |   +-----------> [4:5] Project ID
 *    |  |  |  +---------------> [3] HW version
 *    |  |  +------------------> [2] CG ink color
 *    |  +---------------------> [1] Display maker
 *    +------------------------> [0] The maker of Touch Panel and CG lamination
 */
enum {
	LOCKDOWN_INFO_PANEL_MAKER_INDEX = 0,
	LOCKDOWN_INFO_DISPLAY_MAKER_INDEX,
	LOCKDOWN_INFO_PANEL_COLOR_INDEX,
	LOCKDOWN_INFO_HW_VERSION_INDEX,
	LOCKDOWN_INFO_PROJECT_ID1_INDEX,
	LOCKDOWN_INFO_PROJECT_ID2_INDEX,
	LOCKDOWN_INFO_CG_MAKER_INDEX,
	LOCKDOWN_INFO_RESERVE_INDEX,

	/**
	 * How many bytes to describte lockdown info?
	 * Now, 8 bytes is enough.
	 */
	LOCKDOWN_INFO_SIZE,
};

/**
 * Gesture report core supported key code.
 * You can report gesture key code via tid_report_key().
 */
enum gesture_key {
	GS_KEY_SWIPE_LEFT = 0,
	GS_KEY_SWIPE_RIGHT,
	GS_KEY_SWIPE_UP,
	GS_KEY_SWIPE_DOWM,
	GS_KEY_DOUBLE_TAP,
	GS_KEY_ONECE_TAP,
	GS_KEY_LONG_PRESS,
	GS_KEY_E,
	GS_KEY_C,
	GS_KEY_W,
	GS_KEY_M,
	GS_KEY_O,
	GS_KEY_S,
	GS_KEY_V,
	GS_KEY_Z,

	/* do not use the following enumeration */
	GS_KEY_END,					/* end */
	GS_KEY_ENABLE = 31,
};

struct tid_private;

/**
 * struct touch_info_dev_operations - The methods of touch_info_dev
 * @reset:                  Reset the IC.
 * @get_version:            Get the firmware version of the IC. The parameter
 *  	@major maybe NULL sometimes. So, you should check whether it is
 *  	NULL to prevent NULL pointer reference.
 * @firmware_upgrade:       Firmware upgrade.
 * @open_short_test:        The callback of the open-short test. If open-short
 *  	test success return 1 and fail return 0. Return an error
 *  	number(negative number) if there is an error.
 * @get_lockdown_info:      Get the lockdown info. The buf size
 *  	is LOCKDOWN_INFO_SIZE.
 * @gesture_set_capability: Enable or disable all gesture.
 *
 * Notice: If success return 0, otherwise return an error
 * number(negative number) exclude @open_short_test. You can find all error
 * numbers in include/uapi/asm-gerneric/errno-base.h.
 */
struct touch_info_dev_operations {
	int (*reset) (struct device *dev);
	int (*get_version) (struct device *dev, int *major, int *minor);
	int (*firmware_upgrade) (struct device *dev, const struct firmware *fw,
							 bool force);
#ifdef CONFIG_TOUCHSCREEN_OPEN_SHORT_TEST
	int (*open_short_test) (struct device *dev, struct seq_file *seq,
							const struct firmware *fw);
#endif
#ifdef CONFIG_TOUCHSCREEN_LOCKDOWN_INFO
	/* the buf size is LOCKDOWN_INFO_SIZE */
	int (*get_lockdown_info) (struct device *dev, char *buf);
#endif
#ifdef CONFIG_TOUCHSCREEN_GESTURE
	int (*gesture_set_capability) (struct device *dev, bool enable);
#endif
};

/**
 * struct touch_info_dev - The touch infomation structure
 *
 * @rst_gpio:             The reset gpio num.
 * @irq_gpio:             The irq gpio num.
 * @vendor:               The vendor of the touch ic.
 * @product:              The name of the touch ic.
 * @panel_maker:          The maker of the touch panel.If you enable
 *  	CONFIG_TOUCHSCREEN_LOCKDOWN_INFO, @panel_maker can set NULL.Because
 *  	we will get maker via @tid_ops->get_lockdown_info().
 * @use_dev_path:         If your driver has implemented the sysfs file,
 *  	you can set @use_dev_path %true. And also, @reset,
 *  	@get_version and @firmware_upgrade do not need to
 *  	implement. Otherwise set @use_dev_path %false. We will
 *  	help you create those sysfs file.
 * @fw_name_use_color:    Firmware name should include panel color?
 * @ini_name_use_color:   Ini(open-short test) name should include panel color?
 * @tid_ops:              The methods of touch_info_dev.
 * @p:                    Touch core specific data, your driver should not touch it.
 * @dev                   Device.
 */
struct touch_info_dev {
	int rst_gpio;
	int irq_gpio;
	const char *vendor;
	const char *product;
	const char *panel_maker;
	bool use_dev_path:1;
	bool fw_name_use_color:1;
	bool ini_name_use_color:1;
	bool open_short_not_use_fw:1;
	struct touch_info_dev_operations *tid_ops;

	/**
	 * touch core specific data, your driver should
	 * not touch it.
	 */
	struct tid_private *p;
	struct device dev;
	const char *panel_color;
};

/* Initialize a touch_info_dev structure */
#define TOUCH_INFO_DEV_INIT(name) {         \
		.rst_gpio          = -1,            \
		.irq_gpio          = -1,            \
		.vendor            = NULL,          \
		.product           = NULL,          \
		.panel_maker       = NULL,          \
		.use_dev_path      = false,         \
		.fw_name_use_color = false,         \
		.ini_name_use_color = false,        \
		.open_short_not_use_fw = false,     \
		.tid_ops           = NULL,          \
		.p                 = NULL,          \
	}

/* Define a touch_info_dev varible statically and initialize it */
#define TOUCH_INFO_DEV(name) \
	struct touch_info_dev name = TOUCH_INFO_DEV_INIT(name)

#define PROC_ENTRY_FUNCTION(name)                                          \
	static int name##_proc_open(struct inode *inode, struct file *file)	   \
	{                                                                      \
		return single_open(file, name##_proc_show, PDE_DATA(inode));       \
	}

#define PROC_ENTRY_RO(name)                                                \
	PROC_ENTRY_FUNCTION(name)                                              \
	static const struct file_operations name##_fops = {                    \
		.owner          = THIS_MODULE,                                     \
		.open           = name##_proc_open,                                \
		.read           = seq_read,	                                       \
		.llseek         = seq_lseek,                                       \
		.release        = single_release,                                  \
	}

#define PROC_ENTRY_RW(name)                                                \
	PROC_ENTRY_FUNCTION(name)                                              \
	static const struct file_operations name##_fops = {                    \
		.owner          = THIS_MODULE,                                     \
		.open           = name##_proc_open,                                \
		.read           = seq_read,	                                       \
		.write          = name##_proc_write,                               \
		.llseek         = seq_lseek,                                       \
		.release        = single_release,                                  \
	}

#ifdef CONFIG_TOUCHSCREEN_INFORMATION_INTERFACE
struct touch_info_dev *devm_touch_info_dev_allocate(struct device *dev,
													bool alloc_ops);
int devm_touch_info_dev_register(struct device *dev, const char *name,
								 struct touch_info_dev *tid);
int touch_info_dev_register(struct device *dev, const char *name,
							struct touch_info_dev *tid);
void devm_touch_info_dev_unregister(struct device *dev,
									struct touch_info_dev *tid);
void touch_info_dev_unregister(struct touch_info_dev *tid);
int tid_upgrade_firmware_nowait(struct touch_info_dev *tid);
int tid_hardware_info_get(char *buf, size_t size);
#ifdef CONFIG_TOUCHSCREEN_GESTURE
int tid_report_key(enum gesture_key key);
#else
static inline int tid_report_key(enum gesture_key key)
{
	return -ENODEV;
}
#endif /* CONFIG_TOUCHSCREEN_GESTURE */
const char *tid_panel_maker(void);
const char *tid_panel_color(void);
#else
static inline struct touch_info_dev *
devm_touch_info_dev_allocate(struct device *dev,
							 bool alloc_ops)
{
	return NULL;
}

static inline int devm_touch_info_dev_register(struct device *dev,
											   const char *name,
											   struct touch_info_dev *tid)
{
	return -ENODEV;
}

static inline int touch_info_dev_register(struct device *dev,
										  const char *name,
										  struct touch_info_dev *tid)
{
	return -ENODEV;
}

static inline void devm_touch_info_dev_unregister(struct device *dev,
												  struct touch_info_dev *tid)
{
}

static inline void touch_info_dev_unregister(struct touch_info_dev *tid)
{
}

static inline int tid_hardware_info_get(char *buf, size_t size)
{
	return -ENODEV;
}

static inline int tid_report_key(enum gesture_key key)
{
	return -ENODEV;
}

static inline int tid_upgrade_firmware_nowait(struct touch_info_dev *tid)
{
	return -ENODEV;
}

static inline const char *tid_panel_maker(void)
{
	return NULL;
}

static inline const char *tid_panel_color(void)
{
	return NULL;
}

#endif /* CONFIG_TOUCHSCREEN_INFORMATION_INTERFACE */

static inline int devm_tid_register(struct device *dev,
									struct touch_info_dev *tid)
{
	return devm_touch_info_dev_register(dev, NULL, tid);
}

static inline void devm_tid_unregister(struct device *dev,
									   struct touch_info_dev *tid)
{
	devm_touch_info_dev_unregister(dev, tid);
}

static inline struct touch_info_dev *devm_tid_allocate(struct device *dev)
{
	return devm_touch_info_dev_allocate(dev, false);
}

static inline struct touch_info_dev *
devm_tid_and_ops_allocate(struct device *dev)
{
	return devm_touch_info_dev_allocate(dev, true);
}

#endif /* _TOUCH_INFO_H_ */
