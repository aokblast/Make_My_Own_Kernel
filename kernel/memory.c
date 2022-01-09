#include "memory.h"
#include "lib.h"
#include "printk.h"

unsigned long page_init(struct Page *p, unsigned long flags){
    if(!p->attr){
        *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << ((p->PHY_address) >> PAGE_2M_SHIFT) % 64;
        p->attr = flags;
        ++p->ref_cnt;
        --p->zone_struct->page_using_cnt;
        ++p->zone_struct->page_free_cnt;
        ++p->zone_struct->tot_pages_link;
    }else if((p->attr & PG_Referenced) || (p->attr & PG_K_Share_To_U) || (flags & PG_Referenced || (flags & PG_K_Share_To_U))){
        p->attr |= flags;
        ++p->ref_cnt;
        ++p->zone_struct->tot_pages_link;
    }else{
        *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        p->attr |= flags;
    }
    return 0;
}

void init_memory(){
    int i, j;
    unsigned long totalMem = 0;
    struct E820 *p = NULL;
    color_printk(BLUE, BLACK, "Diaplay Physics Address MAP, Type(1: RAM, 2:ROM or Reserved, 3:ACPI Reclaim Memory, 4:ACPI NVS Memory, Others:Undefine)\n");
    p = (struct E820 *)0xffff800000007e00;

    for(int i = 0; i < 32; ++i){
        color_printk(ORANGE,BLACK,"Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address, p->len, p->type);
        unsigned long tmp = 0;
        
        memory_management_struct.e820[i] = *p;
        if(p->type == 1)totalMem += memory_management_struct.e820[i].len;
        memory_management_struct.e820_len = i + 1;
        p++;
        if(p->type > 4 || p->len == 0 || p->type < 1)break;
    }
    color_printk(ORANGE,BLACK,"OS Can Used Total RAM:%#018lx\n",totalMem);

    totalMem = 0;
    unsigned long start, end;

    for(int i = 0; i < memory_management_struct.e820_len; ++i){
        if(memory_management_struct.e820[i].type != 1) continue;
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].len) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        totalMem += (end - start) >> PAGE_2M_SHIFT;
    }

    color_printk(ORANGE,BLACK,"OS Can Used Total 2M PAGEs:%#010x=%010d\n", totalMem, totalMem);
    

    totalMem = memory_management_struct.e820[memory_management_struct.e820_len - 1].address + memory_management_struct.e820[memory_management_struct.e820_len - 1].len;

    memory_management_struct.bits_map = (unsigned long *)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.bits_size = totalMem >> PAGE_2M_SHIFT;
    memory_management_struct.bits_len = (((unsigned long)(totalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) >> 3) & (~(sizeof(long) - 1));
    memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_len);

    memory_management_struct.pages_struct = (struct Page *)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_len + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.pages_size = totalMem >> PAGE_2M_SHIFT;
    memory_management_struct.pages_len = ((totalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    memset(memory_management_struct.pages_struct, 0x00, memory_management_struct.pages_len);

    memory_management_struct.zones_struct = (struct Zone *)(((unsigned long)memory_management_struct.pages_struct + memory_management_struct.pages_len + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.zones_size = 0;
    memory_management_struct.zones_len = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.zones_struct, 0x00, memory_management_struct.zones_len);

    for(int i = 0; i < memory_management_struct.e820_len; ++i){
        struct Zone *z;
        struct Page *p;
        unsigned long *b;
        if(memory_management_struct.e820[i].type != 1) continue;
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].len) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if(end <= start)continue;

        z = memory_management_struct.zones_struct + memory_management_struct.zones_size;
        ++memory_management_struct.zones_size;
        
        z->zone_start_addr = start;
        z->zone_end_addr = end;
        z->zone_len = end - start;

        z->page_using_cnt = 0;
        z->page_free_cnt = (end - start) >> PAGE_2M_SHIFT;

        z->tot_pages_link = 0;

        z->attr = 0;
        z->GMD_struct = &memory_management_struct;

        z->pages_len = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page *)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));

        p = z->pages_group;
        for(int j = 0; j < z->pages_len; ++j, ++p){
            p->zone_struct = z;
            p->attr = 0;
            p->PHY_address = start + j * PAGE_2M_SIZE;
            p->ref_cnt = 0;
            p->age = 0;

            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;
        }

    }
    
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;

    memory_management_struct.pages_struct->PHY_address = 0UL;
    memory_management_struct.pages_struct->attr = memory_management_struct.pages_struct->ref_cnt = memory_management_struct.pages_struct->age = 0;

    memory_management_struct.zones_len = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1)  & (~(sizeof(long) - 1));

    color_printk(ORANGE,BLACK,"bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",memory_management_struct.bits_map,memory_management_struct.bits_size,memory_management_struct.bits_len);

	color_printk(ORANGE,BLACK,"pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",memory_management_struct.pages_struct,memory_management_struct.pages_size,memory_management_struct.pages_len);

	color_printk(ORANGE,BLACK,"zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",memory_management_struct.zones_struct,memory_management_struct.zones_size,memory_management_struct.zones_len);

    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;

    for(int i = 0; i < memory_management_struct.zones_size; ++i){
        struct Zone *z = memory_management_struct.zones_struct + i;

        color_printk(ORANGE,BLACK,"zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",z->zone_start_addr,z->zone_end_addr,z->zone_len,z->pages_group,z->pages_len);

        if(z->zone_start_addr == 0x100000000){
            ZONE_UNMAPED_INDEX = i;
        }
    }

    memory_management_struct.end_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_len + sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE,BLACK,"start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk, memory_management_struct.end_struct);

    int idx = Virt_To_Phy(memory_management_struct.end_struct) >> PAGE_2M_SHIFT;

    for(int j = 0; j <= idx; ++j){
        page_init(memory_management_struct.pages_struct + j, PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }

    Global_CR3 = get_GDT();

    color_printk(INDIGO,BLACK,"Global_CR3\t:%#018lx\n",Global_CR3);
	color_printk(INDIGO,BLACK,"*Global_CR3\t:%#018lx\n",*Phy_To_Virt(Global_CR3) & (~0xff));
	color_printk(PURPLE,BLACK,"**Global_CR3\t:%#018lx\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) & (~0xff));

    for(int i = 0; i < 10; ++i) *(Phy_To_Virt(Global_CR3) + i) = 0UL;

    flush_tlb();

    
}

struct Page* alloc_pages(int zone_select, int number, unsigned long flags){

}