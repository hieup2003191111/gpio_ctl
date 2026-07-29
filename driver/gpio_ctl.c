#include <linux/module.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define GPIO_ON     1
#define GPIO_OFF    0
#define GPIO_TOGGLE 2
#define GPIO_GET    3

static struct gpio_desc *led_gpio;
static int led_status = 0;
static struct class *gpio_ctl_class;
static struct device *gpio_ctl_dev;
static dev_t gpio_ctl_devno;
static struct cdev gpio_ctl_cdev;

static ssize_t status_show(struct device *dev, struct device_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", led_status);
}
static DEVICE_ATTR_RO(status);

static ssize_t gpio_ctl_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos) {
    char kbuf[2];
    if (copy_from_user(kbuf, buf, 1)) return -EFAULT;
    if (kbuf[0] == '1') { gpiod_set_value(led_gpio, 1); led_status = 1; }
    else if (kbuf[0] == '0') { gpiod_set_value(led_gpio, 0); led_status = 0; }
    return len;
}

static ssize_t gpio_ctl_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
    char kbuf[2];
    int ret;
    kbuf[0] = led_status ? '1' : '0';
    kbuf[1] = '\n';
    if (*ppos > 0) return 0;
    ret = copy_to_user(buf, kbuf, 2);
    if (ret) return -EFAULT;
    *ppos += 2;
    return 2;
}

static long gpio_ctl_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch(cmd) {
        case GPIO_ON:
            gpiod_set_value(led_gpio, 1);
            led_status = 1;
            break;
        case GPIO_OFF:
            gpiod_set_value(led_gpio, 0);
            led_status = 0;
            break;
         case GPIO_TOGGLE:
            led_status = !gpiod_get_value(led_gpio);
            gpiod_set_value(led_gpio, led_status);
            led_status = gpiod_get_value(led_gpio);
            printk(KERN_INFO "gpio_ctl: toggled, new status = %d\n", led_status);
            break;
        case GPIO_GET:
            return led_status;
        default:
            return -EINVAL;
    }
    return 0;
}

static const struct file_operations gpio_ctl_fops = {
    .owner = THIS_MODULE,
    .write = gpio_ctl_write,
    .read  = gpio_ctl_read,
    .unlocked_ioctl = gpio_ctl_ioctl,
};

static int gpio_ctl_probe(struct platform_device *pdev) {
    int ret;
    printk(KERN_INFO "gpio_ctl: probe called\n");

    led_gpio = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpio)) {
        printk(KERN_ERR "gpio_ctl: failed to get GPIO\n");
        return PTR_ERR(led_gpio);
    }

    ret = alloc_chrdev_region(&gpio_ctl_devno, 0, 1, "gpio_ctl");
    if (ret < 0) return ret;

    cdev_init(&gpio_ctl_cdev, &gpio_ctl_fops);
    gpio_ctl_cdev.owner = THIS_MODULE;
    ret = cdev_add(&gpio_ctl_cdev, gpio_ctl_devno, 1);
    if (ret < 0) return ret;

    gpio_ctl_class = class_create(THIS_MODULE, "gpio_ctl");
    gpio_ctl_dev = device_create(gpio_ctl_class, NULL, gpio_ctl_devno, NULL, "gpio_ctl");
    device_create_file(gpio_ctl_dev, &dev_attr_status);

    printk(KERN_INFO "gpio_ctl: probe finished successfully\n");
    return 0;
}

static int gpio_ctl_remove(struct platform_device *pdev) {
    device_remove_file(gpio_ctl_dev, &dev_attr_status);
    device_destroy(gpio_ctl_class, gpio_ctl_devno);
    class_destroy(gpio_ctl_class);
    cdev_del(&gpio_ctl_cdev);
    unregister_chrdev_region(gpio_ctl_devno, 1);
    printk(KERN_INFO "gpio_ctl: removed\n");
    return 0;
}

static const struct of_device_id gpio_ctl_dt_ids[] = {
    { .compatible = "demo,gpio-ctl" },
    { }
};
MODULE_DEVICE_TABLE(of, gpio_ctl_dt_ids);

static struct platform_driver gpio_ctl_driver = {
    .probe = gpio_ctl_probe,
    .remove = gpio_ctl_remove,
    .driver = {
        .name = "gpio_ctl",
        .of_match_table = gpio_ctl_dt_ids,
    },
};
module_platform_driver(gpio_ctl_driver);

MODULE_LICENSE("GPL");
