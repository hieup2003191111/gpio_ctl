# 🪜 Step-by-step Tích Hợp Driver Built-in Cho Raspberry Pi 4

### 1️⃣ Tạo thư mục driver và copy mã nguồn
```bash
cd buildroot/output/build/linux-custom/drivers/char
mkdir -p gpio_ctl
cp gpio_ctl.c gpio_ctl/
cd gpio_ctl
### 2️⃣ Tạo Makefile cho driver
echo 'obj-$(CONFIG_GPIO_CTL) += gpio_ctl.o' > Makefile
### 3️⃣ Tạo Kconfig cho driver
cat <<EOF> Kconfig
config GPIO_CTL
    bool "Demo GPIO LED Control Driver"
    depends on GPIOLIB
    help
      Simple platform driver to control LED via GPIO.
EOF
### 4️⃣ Liên kết vào Kconfig tổng của Character devices
Mở file drivers/char/Kconfig:
