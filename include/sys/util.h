#ifndef _UTIL_H
#define _UTIL_H

#define bit(value, flag) ((value & flag) == flag)
#define bis(value, flag) (value |= flag)
#define bic(value, flag) (value &= ~flag)

void sleep(uint32_t sec);
uint64_t min(uint64_t a, uint64_t b);
void sbustrncpy(char* dest, char* src, int len);
#endif
