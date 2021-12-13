#include <stdarg.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"

void putchar(unsigned int *fb, int xsize, int x, int y, unsigned int FRcolor, unsigned int BKColor, unsigned char font){
    int i = 0, j = 0;
    unsigned int *addr = NULL;
    unsigned char *fontp = NULL;
    int mask = 0;
    fontp = font_ascii[font];
    for(i = 0 ; i < 16; ++i){
        addr = fb + xsize * (y + i) + x;
        mask  = 0x100;
        for(j = 0; j < 8; ++j){
            mask = mask >> 1;
            if(*fontp & mask) *addr = FRcolor;
            else *addr = BKColor;
            ++addr;
        }
        ++fontp;
    }
}


int strlen(char *str){
    register int res;
    __asm__ __volatile__ ( "cld \n\t"
                           "repne \n\t"
                           "scasb \n\t"
                           "notl %0 \n\t"
                           "dec %0 \n\t"
                           :"=c"(res)
                           :"D"(str), "a"(0), "0"(0xffffffff)
                           :

    );
    return res;
}


int skip_atoi(const char **s){
    int res = 0;
    while(is_digit(**s)) res = res * 10 - '0' + *((*s)++); 
    return res;
}

static char *itoa(char *str, long num, int base, int size, int precision, int attr){
    char c, sign, tmp[50];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i ;
    if(attr & LOWCASE) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    if(attr & LEFT) attr &= ~ZEROPAD;
    if(base < 2 || base > 36)return NULL;
    c = (attr & ZEROPAD ? '0' : ' ');
    sign = 0;
    if(attr & SIGN && num < 0)sign = '-', num = -num;
    else sign =  (attr & SIGN) ? '+' : ((attr & SPACE) ? ' ' : 0);
    if(sign)size--;
    if(attr & SPECIAL) if(base == 16) size -= 2; else if(base == 8) --size;
    i = 0;
    if(num == 0) tmp[i++] = '0';
    else while(num != 0)tmp[i++] = digits[do_div(num, base)];
    if(i > precision) precision = i;
    size -= precision;
    if(!(attr & (ZEROPAD | LEFT))) while(size -- > 0) *str ++ = ' ';
    if(sign) *str++ = sign;
    if(attr & SPECIAL) if(base == 8) *str++ = '0'; else if(base == 16) *str++ = '0', *str++ = digits[33];
    if(!(attr & LEFT)) while(size-- > 0) *str++ = c;
    while(i < precision--) *str++ = '0';
    while(i-- > 0) *str++ = tmp[i];
    while(size-- > 0) *str++ = ' ';
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args){
    char *str, *s;
    int flags;
    int field_width;
    int precision;
    int len, i;

    int qualifier;

    for(str = buf; *fmt; ++fmt){
        if(*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
        flags = 0;
        repeat:
            ++fmt;
            switch(*fmt){
                case '-': flags |= LEFT;
                goto repeat;
                case '+': flags |= PLUS;
                goto repeat;
                case ' ': flags |= SPACE;
                goto repeat;
                case '#': flags |= SPECIAL;
                goto repeat;
                case '0': flags |= ZEROPAD;
                goto repeat;
            }

            field_width = - 1;
            if(is_digit(*fmt)) field_width = skip_atoi(&fmt);
            else if(*fmt == '*'){
                ++fmt;
                field_width = va_arg(args, int);
                if(field_width < 0) {
                    field_width = -field_width;
                    flags |= LEFT;
                }
            }

            precision = -1;
            if(*fmt == '.'){
                fmt++;
                if(is_digit(*fmt)) precision = skip_atoi(&fmt);
                else if(*fmt == '*'){
                    ++fmt;
                    precision = va_arg(args, int);
                }
                if(precision < 0) precision = 0;
            }

            qualifier = -1;
            if(*fmt == 'h' || *fmt == '1' || *fmt == 'L' || *fmt == 'Z'){
                qualifier = *fmt;
                ++fmt;
            }

            switch(*fmt){
                case 'c':
                    if(!(flags & LEFT)) while((--field_width) > 0) *str++ = ' ';
                    *str = (unsigned char) va_arg(args, int);
                    while((--field_width) > 0) *str++ = ' ';
                    break;
                case 's':
                    s = va_arg(args, char *);
                    if(!s) s = '\0';
                    len = strlen(s);
                    if(precision < 0) precision = 0;
                    else if (len > precision) len = precision;
                    if(!(flags & LEFT)) while(len < field_width--) *str++ = ' ';
                    for(i = 0; i < len ; ++i)*str++ = *s++;
                    while(len < field_width--) *str++ = ' ';
                    break;
                case 'o':
                    if(qualifier == 'l') str = itoa(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
                    else str = itoa(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
                    break;
                case 'p':
                    if(field_width == -1) {
                        field_width = 2 * sizeof(void *);
                        flags |= ZEROPAD;
                    }
                    str = itoa(str, (unsigned long)(va_arg(args, void *)), 16, field_width, precision, flags);
                    break;
                case 'x':
                    flags |= LOWCASE;
                case 'X':
                    if(qualifier == 'l'){
                        str = itoa(str, (unsigned long)(va_arg(args, unsigned long)), 16, field_width, precision, flags);
                    }else{
                        str = itoa(str, (unsigned int)(va_arg(args, unsigned int)), 16, field_width, precision, flags);
                    }
                    break;
                case 'd':
                case 'i':
                    flags |= SIGN;
                case 'u':
                    if(qualifier == 'l')str = itoa(str, va_arg(args, unsigned long), 10 ,field_width, precision, flags);
                    else str  = itoa(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
                    break;
                case 'n':
                    if(qualifier == 'l') {
                        long *ip = va_arg(args, long *);
                        *ip = (str - buf);
                    }else{
                        int *ip = va_arg(args, int *);
                        *ip = (str - buf);
                    }
                    break;
                case '%':
                    *str++ = '%';
                    break;
                default:
                    *str++ = '%';
                    if(*fmt)
                    *str++ = *fmt;
                    else fmt--;
                    break;

            }
    }
    *str = '\0';
    return str - buf;

}

int color_printk(unsigned int FRcolor, unsigned BKcolor, const char *fmt, ...){
    int i = 0;
    int count = 0;
    int spaces = 0;
    va_list args;
    va_start(args, fmt);
    
    i = vsprintf(buf, fmt, args);
    
    va_end(args);
    
    for(count = 0; count < i || spaces; count++){
        if(spaces > 0){
            --count;
            goto Label_tab;
        }
        if((unsigned char) *(buf + count) == '\n'){
            ++Pos.yPos;
            Pos.xPos = 0;
        }else if((unsigned char) *(buf + count) == '\b'){
            if((--Pos.xPos) < 0){
                Pos.xPos = (Pos.xRes / Pos.xCharSize - 1) * Pos.xCharSize;
                if((--Pos.yPos) < 0)Pos.yPos = (Pos.yRes / Pos.yCharSize - 1) * Pos.yCharSize;
            }
            putchar(Pos.FB_addr, Pos.xRes, Pos.xRes * Pos.xCharSize, Pos.yPos * Pos.yCharSize, FRcolor, BKcolor, ' ');
        }else if((unsigned char) *(buf + count) == '\t'){
            spaces = ((Pos.xPos  + 8) & ~(7)) - Pos.xPos;
            Label_tab:
            spaces--;
            putchar(Pos.FB_addr, Pos.xRes, Pos.xPos * Pos.xCharSize, Pos.yPos * Pos.yCharSize, FRcolor, BKcolor, ' ');
            ++Pos.xPos;
        }else{
            putchar(Pos.FB_addr, Pos.xRes, Pos.xPos * Pos.xCharSize, Pos.yPos * Pos.yCharSize, FRcolor, BKcolor, (unsigned char) * (buf + count));
            ++Pos.xPos;
        }
        if(Pos.xPos >= (Pos.xRes / Pos.xCharSize) ){
            ++Pos.yPos;
            Pos.xPos = 0;
        }
        if(Pos.yPos >= (Pos.yRes / Pos.yCharSize)){
            Pos.yPos = 0;
        }

    }
    return i;

}