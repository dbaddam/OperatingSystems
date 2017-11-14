#ifndef _UTIL_H
#define _UTIL_H

#define bit(value, flag) ((value & flag) == flag)
#define bis(value, flag) (value |= flag)

void sleep(uint32_t sec);
#endif
