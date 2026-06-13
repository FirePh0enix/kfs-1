#include <stdarg.h>

extern void terminal_printn(const char *s, int n);
extern void terminal_full_flush();

static void write_int(long i) {
  int n;
  char buf[32];
  long num;
  long len;
  long len2;

  n = 0;
  if (i == 0) {
    buf[0] = '0';
    len++;
  }
  num = i;
  if (i < 0)
    num = -i;
  len = 0;
  while (num > 0) {
    buf[len++] = "0123456789abcdef"[num % 10];
    num /= 10;
  }

  for (int i = 0; i < len; i++)
    terminal_printn(&buf[len - i - 1], len);
}

void ft_printf(const char *fmt, ...) {
  va_list list;
  int n;

  va_start(list, fmt);
  n = 0;
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      // n += format(list, (char **)&fmt);
      if (*fmt == 'd') {
        write_int(va_arg(list, int));
      }
    } else {
      terminal_printn(fmt, 1);
    }
    fmt++;
  }
  va_end(list);
}

