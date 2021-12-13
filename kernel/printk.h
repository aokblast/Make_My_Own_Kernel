#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>
#include "font.h"
#include "linkage.h"

#define ZEROPAD 1 
#define SIGN 2 
#define PLUS 4 
#define SPACE 8 
#define LEFT 16
#define SPECIAL 32
#define LOWCASE 64

#define is_digit(c) (c >= '0' && c <= '9')

#define WHITE 0x00ffffff
#define BLACK 0x00000000
#define RED   0x00ff0000
#define ORANGE 0x00ff8000
#define YELLOW 0x00ffff00
#define GREEN 0x0000ff00
#define BLUE  0x000000ff
#define INDIGO 0x006600ff
#define PURPLE 0x00f000ff



char buf[4096] = {0};

struct position{
    int xRes;
    int yRes;

    int xPos;
    int yPos;

    int xCharSize;
    int yCharSize;

    unsigned int *FB_addr;
    unsigned long FB_length;
}Pos;

void putchar(unsigned int *fb, int xSize , int x, int y, unsigned int FRcolor, unsigned BKcolor, unsigned char font);

int skip_atoi(const char **s);

#define do_div(n, base)({ \
    int res; \
    __asm__("divq %%rcx":"=a"(n), "=d"(res): "0"(n),"1"(0), "c"(base));\
    res; })

static char *itoa(char *str, long num, int base ,int size, int presition, int attr);

int vsprintf(char *buf, const char *fmt, va_list args);

int color_printk(unsigned int FRcolor, unsigned int BKcolor, const char *fmt, ...);


#endif