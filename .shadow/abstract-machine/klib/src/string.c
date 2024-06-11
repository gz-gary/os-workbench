#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *e = s;
  while (*e != '\0') ++e;
  return e - s;
}

char *strcpy(char *dst, const char *src) {
  panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 != *s2) return *s1 < *s2 ? -1 : 1;
    else ++s1, ++s2;
  }
  switch (((*s1 == '\0') << 1) | (*s2 == '\0')) {
    case 1: return 1;
    case 2: return -1;
    case 3: return 0;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  for (; n > 0; --n, ++s) *(unsigned char *)s = (unsigned char)c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
