#include "xv6-user/user.h"

// 用户级函数，调用系统时间设置系统调用
int set_system_time(int year, int month, int day, int hour, int min, int sec) {
    // 直接调用系统调用
    return settime(year, month, day, hour, min, sec);
}

int main() {
    printf("=== System Time Change Demo ===\n");

    // 先创建一个目录，查看当前时间
    printf("1. Creating directory 'beforesettime' to check current time...\n");
    if (mkdir("beforesettime") < 0) {
        printf("ERROR: Failed to create directory\n");
        exit(1);
    }

    // 设置系统时间到2023年
    printf("2. Setting system time to 2023-05-15 14:30:00...\n");
    int ret = set_system_time(2023, 5, 15, 14, 30, 0);

    if (ret < 0) {
        printf("ERROR: Failed to set system time\n");
        // 清理
        remove("beforesettime");
        exit(1);
    }

    // 创建目录，查看时间是否已更改
    printf("3. Creating directory 'aftersettime' to check the changed time...\n");
    if (mkdir("aftersettime") < 0) {
        printf("ERROR: Failed to create directory\n");
        // 清理
        remove("beforesettime");
        exit(1);
    }

    // 列出目录，比较时间
    printf("4. Comparing directory timestamps:\n");
    printf("   Type 'ls -l' to see the difference:\n");
    printf("   - beforesettime: should be the original time\n");
    printf("   - aftersettime: should be 2023-05-15 14:30:00\n");

    exit(0);
}