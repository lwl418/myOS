#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "xv6-user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static void
printint(int fd, int xx, int base, int sgn, int width, int zero_pad)
{
  char buf[16];
  int i, neg, len, pad;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  len = i;
  pad = (width > len) ? width - len : 0;

  // Print padding characters
  if (!zero_pad) {
    for (int p = 0; p < pad; p++)
      putc(fd, ' ');
  }

  // Print zero padding if enabled
  if (zero_pad) {
    for (int p = 0; p < pad; p++)
      putc(fd, '0');
  }

  while(--i >= 0)
    putc(fd, buf[i]);
}

static void
printptr(int fd, uint64 x) {
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd. Supports %d, %x, %p, %s, %c, %%, %ld, %04d, %4d, etc.
void
vprintf(int fd, const char *fmt, va_list ap)
{
  char *s;
  int c, i, state;
  int width;      // Width specifier
  int zero_pad;   // Zero padding flag

  state = 0;
  width = 0;
  zero_pad = 0;

  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;

    if(state == 0){
      if(c == '%'){
        state = '%';
        width = 0;
        zero_pad = 0;
      } else {
        putc(fd, c);
      }
    } else if(state == '%'){
      if(c >= '0' && c <= '9'){
        if (c == '0' && width == 0) {
          zero_pad = 1;  // Zero padding
        } else {
          width = width * 10 + (c - '0');  // Width specifier
        }
      } else {
        // Process the format specifier
        if(c == 'd'){
          printint(fd, va_arg(ap, int), 10, 1, width, zero_pad);
        } else if(c == 'l') {
          // Handle long format by casting, since our printint doesn't support 64-bit yet
          printint(fd, (int)va_arg(ap, uint64), 10, 0, width, zero_pad);
        } else if(c == 'x') {
          printint(fd, va_arg(ap, int), 16, 0, width, zero_pad);
        } else if(c == 'p') {
          printptr(fd, va_arg(ap, uint64));
        } else if(c == 's'){
          s = va_arg(ap, char*);
          if(s == 0)
            s = "(null)";
          while(*s != 0){
            putc(fd, *s);
            s++;
          }
        } else if(c == 'c'){
          putc(fd, va_arg(ap, uint));
        } else if(c == '%'){
          putc(fd, c);
        } else {
          // Unknown % sequence.  Print it to draw attention.
          putc(fd, '%');
          putc(fd, c);
        }
        state = 0;
        width = 0;
        zero_pad = 0;
      }
    }
  }
}

void
fprintf(int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void
printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
