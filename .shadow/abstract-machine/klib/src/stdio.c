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

static void put_int(void *arg) {
  int x = *(int *)arg;
  if (x == 0) putch('0');
  else if (x < 0) {
    putch('-');
    recursive_put_int(-x);
  } else recursive_put_int(x);
}

static void put_str(void *arg) {
  for (const char *str = arg; *str != '\0'; ++str) putch(*str);
}

struct resolver {
  const char *match_pattern;
  void (*resolve_func)(void *);
} resolvers[] = {
  {"%d", put_int},
  {"%p", put_int},
  {"%s", put_str},
};

const char * prefix_match(const char *str, const char *prefix) {
  const char *tmp = str;
  for (; *str != '\0' && *str == *prefix; ++str, ++prefix);
  return *prefix == '\0' ? str : tmp;
}

int printf(const char *fmt, ...) {
  if (prefix_match(fmt, "%d") != fmt) {
    put_str("success\n");
  } else {
    put_str("failed\n");
  }
  //regmatch_t pos;
  //va_list ap;
  //va_start(ap, fmt);
  //while (fmt) {
  //}
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
