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
#define NR_RESOLVER sizeof(resolvers) / sizeof(struct resolver)

const char * prefix_match(const char *str, const char *prefix) {
  const char *tmp = str;
  for (; *str != '\0' && *str == *prefix; ++str, ++prefix);
  return *prefix == '\0' ? str : tmp;
}

int printf(const char *fmt, ...) {
  while (*fmt) {
    int resolver_id = -1;
    const char *end = NULL;
    for (int i = 0; i < NR_RESOLVER; ++i) {
      end = prefix_match(fmt, resolvers[i].match_pattern);
      if (end != fmt) {
        resolver_id = i;
        break;
      }
    }
    int x = 1;
    if (resolver_id != -1) {
      resolvers[resolver_id].resolve_func(&x);
      fmt = end;
    } else putch(*(fmt++));
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
