#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define GPIO_ON     1
#define GPIO_OFF    0
#define GPIO_TOGGLE 2
#define GPIO_GET    3

int main(int argc, char *argv[]) {
    int fd = open("/dev/gpio_ctl", O_RDWR);
    if (fd < 0) { perror("open"); return -1; }

    if (argc > 1) {
        if (!strcmp(argv[1], "on")) write(fd, "1", 1);
        else if (!strcmp(argv[1], "off")) write(fd, "0", 1);
        else if (!strcmp(argv[1], "toggle")) ioctl(fd, GPIO_TOGGLE);
        else if (!strcmp(argv[1], "status")) {
            char buf[2]; read(fd, buf, 2);
            printf("LED status: %c\n", buf[0]);
        }
    } else {
        printf("Usage: %s [on|off|toggle|status]\n", argv[0]);
    }

    close(fd);
    return 0;
}
