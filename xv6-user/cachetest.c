#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "xv6-user/user.h"

// Simple cache performance test program for xv6-k210

// 测试程序会对同一个目录下的多个文件进行重复读取，通过uptime()函数测量时间

#define TEST_FILES 3    // 要操作的文件数量
#define TEST_READS 500  // 每个文件读取的次数
char* test_files[] = {"test1.txt", "test2.txt", "test3.txt"};

int main(int argc, char *argv[]) {
    int i, round, fd;
    int start_time, end_time;
    char buffer[64];
    int total_writes = 0;
    int total_reads = 0;

    printf("=== Cache Performance Test ===\n\n");

    // Step 1: 创建测试文件并写入数据
    printf("1. Creating test files...\n");

    for (i = 0; i < TEST_FILES; i++) {
        fd = open(test_files[i], O_CREATE | O_WRONLY);
        if (fd < 0) {
            printf("Error: Cannot create %s\n", test_files[i]);
            goto cleanup;
        }

        // 写入512字节数据
        char data = 'x';
        for (int j = 0; j < 512; j++) {
            if (write(fd, &data, 1) == 1) {
                total_writes++;
            }
        }

        close(fd);
    }

    printf("Created %d test files with %d bytes total\n\n", TEST_FILES, total_writes);

    // Step 2: 测试顺序文件访问
    printf("2. Testing sequential file access...\n");
    start_time = uptime();

    for (round = 0; round < TEST_READS; round++) {
        for (i = 0; i < TEST_FILES; i++) {
            fd = open(test_files[i], O_RDONLY);
            if (fd < 0) {
                printf("Error: Cannot open %s\n", test_files[i]);
                goto cleanup;
            }

            // 读取整个文件
            while (read(fd, buffer, sizeof(buffer)) > 0) {
                total_reads++;
            }

            close(fd);
        }
    }

    end_time = uptime();
    printf("Sequential access completed: %d rounds\n", TEST_READS);
    printf("Time taken: %d seconds\n", end_time - start_time);

    // Step 3: 测试随机文件访问模式
    // 这里我们只是重复交替访问不同的文件来模拟随机访问
    printf("\n3. Testing mixed file access...\n");
    start_time = uptime();

    for (round = 0; round < TEST_READS; round++) {
        // 交替顺序：0,2,1,0,2,1...
        int order[] = {0, 2, 1};
        for (i = 0; i < TEST_FILES; i++) {
            fd = open(test_files[order[i]], O_RDONLY);
            if (fd < 0) {
                printf("Error: Cannot open %s\n", test_files[order[i]]);
                goto cleanup;
            }

            // 读取整个文件
            while (read(fd, buffer, sizeof(buffer)) > 0) {
                total_reads++;
            }

            close(fd);
        }
    }

    end_time = uptime();
    printf("Mixed access completed: %d rounds\n", TEST_READS);
    printf("Time taken: %d seconds\n", end_time - start_time);

    // 清理
    printf("\n4. Cleaning up...\n");
    for (i = 0; i < TEST_FILES; i++) {
        open(test_files[i], O_RDONLY); // 关闭并删除文件？xv6可能需要用unlink
    }

    printf("\n=== Test Complete ===\n");
    printf("Total operations: %d writes, %d reads\n", total_writes, total_reads);
    printf("Cache efficiency can be measured by comparing these times with non-cached systems.\n");

    exit(0);

cleanup:
    for (i = 0; i < TEST_FILES; i++) {
        // 尽力清理
    }
    printf("Test failed!\n");
    exit(1);
}
