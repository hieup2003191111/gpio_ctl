# 🪜 Step-by-step Tích Hợp Driver Built-in Cho Raspberry Pi 4

### 1️⃣ Tạo thư mục driver và copy mã nguồn
```bash
cd buildroot/output/build/linux-custom/drivers/char
mkdir -p gpio_ctl
cp gpio_ctl.c gpio_ctl/
cd gpio_ctl
```

### 2️⃣ Tạo Makefile cho driver
```bash
echo 'obj-$(CONFIG_GPIO_CTL) += gpio_ctl.o' > Makefile
```

### 3️⃣ Tạo Kconfig cho driver
```bash
cat <<EOF> Kconfig
config GPIO_CTL
    bool "Demo GPIO LED Control Driver"
    depends on GPIOLIB
    help
      Simple platform driver to control LED via GPIO.
EOF
```

### 4️⃣ Liên kết vào Kconfig tổng của Character devices
Mở file `drivers/char/Kconfig`:
```bash
nano ../Kconfig
```

Thêm dòng sau vào cuối file:
```kconfig
source "drivers/char/gpio_ctl/Kconfig"
```

### 5️⃣ Liên kết vào Makefile tổng của Character devices
Mở file `drivers/char/Makefile`:
```bash
nano ../Makefile
```

Thêm dòng sau vào cuối file:
```makefile
obj-$(CONFIG_GPIO_CTL) += gpio_ctl/
```

### 6️⃣ Cấu hình Kernel qua Buildroot
Chạy lệnh cấu hình:
```bash
cd ~/buildroot
make linux-menuconfig
```

Di chuyển theo đường dẫn:  
`Device Drivers` -> `Character devices` -> `Demo GPIO LED Control Driver`

Nhấn phím **Y** hoặc **dấu cách** để chọn `[*] Built-in`, sau đó **Save** và **Exit**.

### 7️⃣ Build lại Kernel và Hệ thống
```bash
make
```

### 8️⃣ Thêm Device Tree Node cho Raspberry Pi 4
Mở file Device Tree:
```bash
nano output/build/linux-custom/arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dts
```

Thêm đoạn mã DTS vào node root `/`:
```dts
gpio_ctl {
    compatible = "demo,gpio-ctl";
    led-gpios = <&gpio 17 GPIO_ACTIVE_HIGH>;
    status = "okay";
};
```

### 9️⃣ Ghi Image, Boot và Kiểm tra
Ghi file image vào USB/Thẻ nhớ:
```bash
lsblk
sudo dd if=output/images/sdcard.img of=/dev/sdb bs=4M status=progress conv=fsync
```

Sau khi boot trên Pi 4, kiểm tra thiết bị:
```bash
ls -l /dev/gpio_ctl
cat /sys/class/gpio_ctl/status
```
