#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void put_int(int x) {
  if (x < 0) { putch('-'); x = -x; }
  put_int('0' + x % 10);
  if (x > 0) put_int(x / 10);
}

int printf(const char *fmt, ...) {
  put_int(0);
  put_int(-2430);
  put_int(2147483647);
  put_int(-2147483648);
  put_int(6657);
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
