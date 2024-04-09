#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static const char HEX_DIGITS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static void recursive_put_uint(unsigned int x) {
  int pre_dig = x / 10;
  if (pre_dig) recursive_put_uint(pre_dig);
  putch(x % 10 + '0');
}

static void put_int(int x) {
  if (x == 0) putch('0');
  else if (x < 0) {
    putch('-');
    recursive_put_uint(-x);
  } else recursive_put_uint(x);
}

static void put_str(const char *str) {
  for (; *str != '\0'; ++str) putch(*str);
}

static void recursive_put_hex(uintptr_t x) {
  uintptr_t pre_dig = (x / 16);
  if (pre_dig) recursive_put_hex(pre_dig);
  putch(HEX_DIGITS[x % 16]);
}

static void put_ptr(void *ptr) {
  if (ptr == NULL) put_str("(nil)");
  else recursive_put_hex((uintptr_t)ptr);
}

struct arg_type {
  const char *match_pattern;
  enum {
    ARG_TYPE_INT = 1,
    ARG_TYPE_PTR,
    ARG_TYPE_STRING,
  } type;
} arg_types[] = {
  {"%d", ARG_TYPE_INT},
  {"%p", ARG_TYPE_PTR},
  {"%s", ARG_TYPE_STRING},
};
#define NR_ARG_TYPE sizeof(arg_types) / sizeof(struct arg_type)

const char * prefix_match(const char *str, const char *prefix) {
  const char *tmp = str;
  for (; *str != '\0' && *str == *prefix; ++str, ++prefix);
  return *prefix == '\0' ? str : tmp;
}

int printf(const char *fmt, ...) {
  union {
    int d;
    void *p;
    const char *s;
  } arg;
  va_list ap;
  va_start(ap, fmt);
  arg.s = va_arg(ap, const char *);
  while (*fmt) {
    int type_id = -1;
    const char *end = NULL;
    for (int i = 0; i < NR_ARG_TYPE; ++i) {
      end = prefix_match(fmt, arg_types[i].match_pattern);
      if (end != fmt) {
        type_id = i;
        break;
      }
    }
    if (type_id != -1) {
      switch (arg_types[type_id].type) {
      case ARG_TYPE_INT:
        arg.d = va_arg(ap, int);
        put_int(arg.d);
        break;
      case ARG_TYPE_PTR:
        arg.p = va_arg(ap, void *);
        put_ptr(arg.p);
        break;
      case ARG_TYPE_STRING:
        arg.s = va_arg(ap, const char *);
        put_str(arg.s);
        break;
      }
    } else putch(*(fmt++));
  }
  va_end(ap);
  // TODO: finish correct return value
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
