#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "lib.h"
#include "printk.h"

#define PAGE_OFFSET	((unsigned long)0xffff800000000000)
#define PTRS_PER_PAGE 512

#define PAGE_GDT_SHIFT	39
#define PAGE_1G_SHIFT	30
#define PAGE_2M_SHIFT	21
#define PAGE_4K_SHIFT	12

#define PAGE_2M_SIZE	(1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE	(1UL << PAGE_4K_SHIFT)

#define PAGE_2M_MASK	(~ (PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK	(~ (PAGE_4K_SIZE - 1))

#define PAGE_2M_ALIGN(addr)	(((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr)	(((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)

#define Virt_To_Phy(addr)	((unsigned long)(addr) - PAGE_OFFSET)
#define Phy_To_Virt(addr)	((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))

//page struct flag
#define PG_PTable_Maped	(1 << 0)
#define PG_Kernel_Init	(1 << 1)
#define PG_Referenced	(1 << 2)
#define PG_Dirty	(1 << 3)
#define PG_Active	(1 << 4)
#define PG_Up_To_Date	(1 << 5)
#define PG_Device	(1 << 6)
#define PG_Kernel	(1 << 7)
#define PG_K_Share_To_U	(1 << 8)
#define PG_Slab		(1 << 9)

#define ZONE_DMA (1 << 0)
#define ZONE_NORMAL (1 << 1)
#define ZONE_UNMAPED (1 << 2)

typedef struct {unsigned long pml4t;} pml4t_t;
#define	mk_mpl4t(addr,attr)	((unsigned long)(addr) | (unsigned long)(attr))
#define set_mpl4t(mpl4tptr,mpl4tval)	(*(mpl4tptr) = (mpl4tval))


struct Memory_E820_Format{
    unsigned int address1;
    unsigned int address2;
    unsigned int len1;
    unsigned int len2;
    unsigned int type;
};

struct E820 {
    unsigned long address;
    unsigned long len;
    unsigned int type;
}__attribute__((packed));


struct Global_Memory_Descriptor{
    struct E820 e820[32];
    unsigned long e820_len;
    
    unsigned long *bits_map;
    unsigned long bits_size;
    unsigned long bits_len;

    struct Page *pages_struct;
    unsigned long pages_size;
    unsigned long pages_len;

    struct Zone *zones_struct;
    unsigned long zones_size;
    unsigned long zones_len;

    unsigned long start_code, end_code, end_data, end_brk;
    unsigned long end_struct;
};


struct Page{
    struct Zone * zone_struct;
    unsigned long PHY_address;
    unsigned long attr;
    unsigned long ref_cnt;
    unsigned long age;
};

struct Zone{
    struct Page * pages_group;
    unsigned long pages_len;

    unsigned long zone_start_addr;
    unsigned long zone_end_addr;
    unsigned long zone_len;
    unsigned long attr;

    struct Global_Memory_Descriptor *GMD_struct;

    unsigned long page_using_cnt;
    unsigned long page_free_cnt;

    unsigned long tot_pages_link;

};



extern struct Global_Memory_Descriptor memory_management_struct;

void init_memory();
struct Page* alloc_pages(int zone_select, int number, unsigned long flags);


int ZONE_DMA_INDEX	= 0;
int ZONE_NORMAL_INDEX	= 0;	//low 1GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX	= 0;	//above 1GB RAM,unmapped in pagetable

static inline unsigned long* get_GDT(){
    unsigned long *tmp;
    __asm__ __volatile__ (
                "movq %%cr3, %0 \n\t"
                :"=r"(tmp)
                :
                :"memory"
    );
    return tmp;
}

#define flush_tlb() \
do \
{   \
    unsigned long tmp;\
    __asm__ __volatile__ (\
        "movq %%cr3, %0 \n\t"\
        "movq %0, %%cr3 \n\t"\
        :"=r"(tmp) \
        :           \
        :"memory"\
    );\
}while(0)\

unsigned long * Global_CR3 = NULL;

#endif