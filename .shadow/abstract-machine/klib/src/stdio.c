#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void recursive_put_int(unsigned int x) {
  int pre_dig = x / 10;
  if (pre_dig) recursive_put_int(pre_dig);
  putch(x % 10 + '0');
}

static void put_int(int x) {
  if (x == 0) putch('0');
  else if (x < 0) {
    putch('-');
    recursive_put_int(-x);
  } else recursive_put_int(x);
}

int printf(const char *fmt, ...) {
  put_int(2147483647);
  put_int(-2147483647);
  put_int(-2147483648);
  put_int(6657);
  put_int(0);
  //panic("Not implemented");
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
