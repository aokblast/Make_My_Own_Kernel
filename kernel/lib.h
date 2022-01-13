#ifndef __LIB_H__
#define __LIB_H__

#define NULL 0

#define container_of(ptr, type, member)  \
({                                       \
    typeof(((type *)0)->member) *p = (ptr); \
    (type*)(unsigned long)p - (unsigned long)&((((type *)0)->member)); \
})

#define sti() __asm__ __volatile__ ("sti    \n\t":::"memory")
#define cli() __asm__ __volatile__ ("cli    \n\t":::"memory")
#define nop() __asm__ __volatile__ ("nop    \n\t")
#define io_mfence() __asm __volatile__ ("mfence \n\t":::memory)

struct List{
    struct List *prev;
    struct List *next;
};

static inline void list_init(struct List *list){
    list->prev = list;
    list->next = list;
}

static inline void list_add_before(struct List *entry, struct List *newNode){
    newNode->next = entry;
    entry->prev->next = newNode;
    newNode->prev = entry->prev;
    entry->prev = newNode;
}

static inline void list_add_behind(struct List *entry, struct List *newNode){
    newNode->next = entry->next;
    newNode->prev = entry;
    entry->next->prev = newNode;
    entry->next = newNode;
}

static inline void list_del(struct List *entry){
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

static inline long list_isEmpty(struct List* entry){
    return entry == entry->next && entry == entry->prev;
}

static inline struct List *list_prev(struct List *entry){
    return entry->prev;
}

static inline struct List *list_next(struct List *entry){
    return entry->next;
}

static inline void *memcpy(void *from, void *to, long size){
    int d0, d1, d2;
    __asm__ __volatile__ (  "cld     \n\t"
                            "rep     \n\t"
                            "movsq   \n\t"
                            "testb $4, %b4\n\t"
                            "je    1f\n\t"
                            "movsl \n\t"
                            "1:\ttestb $2, %b4 \n\t"
                            "je 2f \n\t"
                            "movsw \n\t"
                            "2:\ttestb $1, %b4 \n\t"
                            "je 3f"
                            "movsb \n\t"
                            "3: \n\t"
                            :"=&c"(d0), "=&D"(d1), "=&S"(d2)
                            :"0"(size / 8), "q"(size), "1"(to), "2"(from)
                            :"memory"
    );

    return to;
}

static inline int memcmp(void *from, void *to, long size){
    register int res;
    __asm__ __volatile__ ( "cld \n\t"
                           "rep \n\t"
                           "cmpsb \n\t"
                           "je 1f \n\t"
                           "movl $1, %%eax \n\t"
                           "jl 1f \n\t"
                           "negl %%eax \n\t"
                           "1: \n\t"
                           :"=a"(res)
                           :"0"(0), "D"(from), "S"(to), "C"(size):
    );
    return res;
}

static inline void *memset(void *to,  unsigned char c, long size){
    int d0, d1;
    unsigned long tmp =  0x0101010101010101UL * c;
    __asm__ __volatile__ (
                           "cld \n\t"
                           "rep \n\t"
                           "stosq \n\t"
                           "testb $4, %b3 \n\t"
                           "je 1f \n\t"
                           "stosl \n\t"
                           "1:\ttestb $2, %b3 \n\t"
                           "je 2f\n\t"
                           "stosw \n\t"
                           "2:\ttestb $1, %b3\n\t"
                           "je 3f \n\t"
                           "stosb \n\t"
                           "3: \n\t"
                           :"=&c"(d0), "=%D"(d1)
                           :"a"(tmp), "q"(size), "0"(size / 8), "1"(to)
                           :"memory"
    );
    return to;
}

static inline char *strcpy(char *dest, char *src){
    __asm__ __volatile__ (
                            "cld \n\t"
                            "1: \n\t"
                            "lodsb \n\t"
                            "stosb \n\t"
                            "testb %%al, %%al \n\t"
                            "jne 1b \n\t"
                            :
                            :"S"(src), "D"(dest)
    );
    return dest;
}

static inline char *strncpy(char *dest, char *src, long size){
    __asm__ __volatile__ ( "cld \n\t"
                           "1: \n\t"
                           "decq %2 \n\t"
                           "js 2f \n\t"
                           "lodsb \n\t"
                           "stosb \n\t"
                           "testb %%al %%al \n\t"
                           "jne 1b \n\t"
                           "rep \n\t"
                           "stosb \n\t"
                           "2: \n\t"
                           :
                           :"S"(src), "D"(dest), "c"(size)
                           :

    );
    return dest;
}

static inline char *strcat(char *dest, char *src){
    __asm__ __volatile__ ( "cld \n\t"
                           "repne \n\t"
                           "scasb \n\t"
                           "decq %1 \n\t"
                           "1: \n\t"
                           "lodsb \n\t"
                           "stosb \n\t"
                           "testb %%al, %%al \n\t"
                           "jne 1b \n\t"
                           :
                           :"S"(src), "D"(dest), "a"(0), "c"(0xffffffff)
                           :

    );
    return dest;
}

static inline int strcmp(char *str1, char *str2){
    register int res;
    __asm__ __volatile__ ( "cld \n\t"
                           "1: \n\t"
                           "lodsb \n\t"
                           "scasb \n\t"
                           "jne 2f \n\t"
                           "testb %%al %%al \n\t"
                           "jne 1b \n\t"
                           "xorl %%eax %%eax \n\t"
                           "jmp 3f \n\t"
                           "2: \n\t"
                           "movl $1, %%eax \n\t"
                           "jl 3f \n\t"
                           "negl %%eax \n\t"
                           "3: \n\t"
                           :"=a"(res)
                           :"D"(str1), "S"(str2)
                           :

    );
    return res;
}

static inline int strncmp(char *str1, char *str2, long size){
    register int res;
    __asm__ __volatile__ ( "cld \n\t"
                           "1: \n\t"
                           "decq %3 \n\t"
                           "js 2f \n\t"
                           "lodsb \n\t"
                           "scasb \n\t"
                           "jne 3f \n\t"
                           "testb %%al, %%al \n\t"
                           "jne 1b \n\t"
                           "2: \n\t"
                           "xorl %%eax, %%eax \n\t"
                           "jmp 4f \n\t"
                           "3: \n\t"
                           "movl $1, %%eax \n\t"
                           "jl 4f \n\t"
                           "negl %%eax \n\t"
                           "4: \n\t"
                           :"=a"(res)
                           :"D"(str1), "S"(str2), "c"(size)
                           :

    );
    return res;
}

static inline int strlen(char *str){
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

static inline unsigned long bit_set(unsigned long *addr, unsigned long loc){
    return *addr | (1UL << loc);
}

static inline unsigned long bit_get(unsigned long *addr, unsigned long loc){
    return *addr & (1UL << loc);
}

static inline unsigned long bit_clean(unsigned long *addr, unsigned long loc){
    return *addr & (~(1UL << loc));
}

static inline unsigned char io_in8(unsigned short port){
    unsigned char res = 0;
    __asm__ __volatile__( "inb %%dx, %0 \n\t"
                          "mfence \n\t"
                          :"=a"(res)
                          :"d"(port)
                          :"memory"

    );
    return res;
}

static inline unsigned int io_in32(unsigned short port){
    unsigned int res = 0;
    __asm__ __volatile__( "inl %%dx, %0 \n\t"
                          "mfence \n\t"
                          :"=a"(res)
                          :"d"(port)
                          :"memory"

    );
    return res;
}

static inline void io_out8(unsigned short port,unsigned char value)
{
	__asm__ __volatile__(	"outb	%0,	%%dx	\n\t"
				"mfence			\n\t"
				:
				:"a"(value),"d"(port)
				:"memory");
}

static inline void io_out32(unsigned short port,unsigned int value)
{
	__asm__ __volatile__(	"outl	%0,	%%dx	\n\t"
				"mfence			\n\t"
				:
				:"a"(value),"d"(port)
				:"memory");
}


#define port_insw(port, buffer, size) __asm__ __volatile__("cld;rep;insw;mfence;"::"d"(port), "D"(buffer), "c"(size):"memory")

#define port_outsw(port, buffer, size) __asm__ __volatile__("cld;rep;outsw;mfence;":"S"(buffer):"d"(port), "c"(size):"memory")

#endif