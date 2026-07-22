#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/string.h>

#define DEVICE_NAME "gpio_ctl"
#define CLASS_NAME  "gpio_ctl"

static struct gpio_desc *led_gpio;
static struct class *gpio_class;
static struct cdev gpio_cdev;
static dev_t dev_num;

enum {
    IOCTL_GET_STATUS = 0,
    IOCTL_SET_ON,
    IOCTL_SET_OFF,
    IOCTL_TOGGLE,
};

static int led_status = 0;

static int gpio_open(struct inode *inode, struct file *file) {
    file->f_pos = 0;
    return 0;
}

static int gpio_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    char kbuf[2] = {0};
    if (len == 0) return 0;
    
    if (copy_from_user(kbuf, buf, 1)) return -EFAULT;

    if (kbuf[0] == '1') {
        gpiod_set_value(led_gpio, 1);
        led_status = 1;
    } else if (kbuf[0] == '0') {
        gpiod_set_value(led_gpio, 0);
        led_status = 0;
    }
    return len;
}

static ssize_t gpio_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    char kbuf[4];
    int bytes;

    bytes = snprintf(kbuf, sizeof(kbuf), "%d\n", led_status);
    return simple_read_from_buffer(buf, len, off, kbuf, bytes);
}

static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_GET_STATUS:
            return led_status;
        case IOCTL_SET_ON:
            gpiod_set_value(led_gpio, 1);
            led_status = 1;
            break;
        case IOCTL_SET_OFF:
            gpiod_set_value(led_gpio, 0);
            led_status = 0;
            break;
        case IOCTL_TOGGLE:
            led_status = !led_status;
            gpiod_set_value(led_gpio, led_status);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = gpio_open,
    .release = gpio_release,
    .write   = gpio_write,
    .read    = gpio_read,
    .unlocked_ioctl = gpio_ioctl,
};

static int gpio_probe(struct platform_device *pdev) {
    int ret;
    led_gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_gpio)) return PTR_ERR(led_gpio);

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    cdev_init(&gpio_cdev, &fops);
    cdev_add(&gpio_cdev, dev_num, 1);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    gpio_class = class_create(CLASS_NAME);
#else
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
#endif
    if (IS_ERR(gpio_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(gpio_class);
    }

    device_create(gpio_class, NULL, dev_num, NULL, DEVICE_NAME);

    pr_info("gpio_ctl: Driver probed successfully!\n");
    return 0;
}

static int gpio_remove(struct platform_device *pdev) {
    device_destroy(gpio_class, dev_num);
    class_destroy(gpio_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("gpio_ctl: Driver removed!\n");
    return 0;
}

static const struct of_device_id gpio_of_match[] = {
    { .compatible = "demo,gpio-ctl" },
    {},
};
MODULE_DEVICE_TABLE(of, gpio_of_match);

static struct platform_driver gpio_driver = {
    .probe  = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = DEVICE_NAME,
        .of_match_table = gpio_of_match,
    },
};

module_platform_driver(gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hieu");
MODULE_DESCRIPTION("GPIO LED Control Driver");