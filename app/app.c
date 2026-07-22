#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_GET_STATUS 0
#define IOCTL_SET_ON     1
#define IOCTL_SET_OFF    2
#define IOCTL_TOGGLE     3

int main() {
    int fd = open("/dev/gpio_ctl", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    // Bật LED
    write(fd, "1", 1);

    // Đọc trạng thái
    char buf[2];
    read(fd, buf, 2);
    printf("LED status: %s\n", buf);

    // Toggle LED
    ioctl(fd, IOCTL_TOGGLE);

    // Đọc lại trạng thái
    read(fd, buf, 2);
    printf("LED status after toggle: %s\n", buf);

    close(fd);
    return 0;
}
