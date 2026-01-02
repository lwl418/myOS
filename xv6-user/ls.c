#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "xv6-user/user.h"

// Convert FAT32 time to human-readable format (HH:MM:SS)
char*
fmttime(uint16 time) {
  static char buf[9];  // Format: HH:MM:SS + null terminator
  int hour = (time >> 11) & 0x1F;
  // 转换为CST时间（UTC + 8小时）
  hour = (hour + 8) % 24;
  int min = (time >> 5) & 0x3F;
  int sec = (time & 0x1F) * 2;  // seconds are stored multiplied by 2

  // Manually format the time string
  buf[0] = '0' + (hour / 10);
  buf[1] = '0' + (hour % 10);
  buf[2] = ':';
  buf[3] = '0' + (min / 10);
  buf[4] = '0' + (min % 10);
  buf[5] = ':';
  buf[6] = '0' + (sec / 10);
  buf[7] = '0' + (sec % 10);
  buf[8] = '\0';

  return buf;
}

// Convert FAT32 date to human-readable format (YYYY-MM-DD)
char*
fmtdate(uint16 date) {
  static char buf[11];  // Format: YYYY-MM-DD + null terminator
  int year = ((date >> 9) & 0x7F) + 1980;  // year is since 1980
  int month = (date >> 5) & 0x0F;
  int day = date & 0x1F;

  // Manually format the date string
  // Year
  buf[0] = '0' + (year / 1000);
  buf[1] = '0' + ((year / 100) % 10);
  buf[2] = '0' + ((year / 10) % 10);
  buf[3] = '0' + (year % 10);
  buf[4] = '-';
  // Month
  buf[5] = '0' + (month / 10);
  buf[6] = '0' + (month % 10);
  buf[7] = '-';
  // Day
  buf[8] = '0' + (day / 10);
  buf[9] = '0' + (day % 10);
  buf[10] = '\0';

  return buf;
}

char*
fmtname(char *name)
{
  return name;  // Return raw name instead of blank-padded for ls default format
}

void
ls(char *path, int long_format)
{
  int fd;
  struct stat st;
  static char *types[] = {  // Declare as static to ensure proper initialization
    [T_DIR]   "DIR ",
    [T_FILE]  "FILE",
    [T_DEVICE] "DEV ",
    [0]       "UNKN",  // Initialize index 0 to avoid NULL pointer
  };

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_DIR){
    int first_file = 1;
    while(readdir(fd, &st) == 1){
      if (long_format) {
        // Improved ls -l format
        // File type, permissions (dummy values since FAT32 doesn't support Unix permissions), size, date, time, name
        printf("%c%c%c%c%c%c%c%c%c%c %8d %s %s %s\n",
          types[st.type][0],    // File type
          '-', '-', '-', '-', '-', '-', '-', '-', '-' // Dummy permissions (FAT32 doesn't support Unix permissions)
          , (int)st.size, fmtdate(st.last_write_date), fmttime(st.last_write_time), fmtname(st.name));
      } else {
        if (!first_file) {
          printf(" ");
        }
        printf("%s", fmtname(st.name));
        first_file = 0;
      }
    }
    if (!long_format) {
      printf("\n"); // Add newline at the end
    }
  } else {
    if (long_format) {
      // Improved ls -l format for single file
      printf("%c%c%c%c%c%c%c%c%c%c %8d %s %s %s\n",
        types[st.type][0],    // File type
        '-', '-', '-', '-', '-', '-', '-', '-', '-' // Dummy permissions (FAT32 doesn't support Unix permissions)
        , (int)st.size, fmtdate(st.last_write_date), fmttime(st.last_write_time), fmtname(st.name));
    } else {
      printf("%s\n", fmtname(st.name));
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i, start;
  int long_format = 0;

  // Parse options
  start = 1;
  if(argc >= 2 && strcmp(argv[1], "-l") == 0){
    long_format = 1;
    start = 2;
  }

  if(argc < start + 1){
    ls(".", long_format);
    exit(0);
  }
  for(i=start; i<argc; i++)
    ls(argv[i], long_format);
  exit(0);
}
