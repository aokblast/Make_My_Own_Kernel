#include "lib.h"
#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"

struct Global_Memory_Descriptor memory_management_struct = {{0},0};

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

void Start_Kernel(void){
    int *addr = (int *)0xffff800000a00000;
    Pos.xRes = 1440;
    Pos.yRes = 900;

    Pos.xPos = Pos.yPos = 0;
    
    Pos.xCharSize = 8;
    Pos.yCharSize = 16;

    Pos.FB_addr = addr;
    Pos.FB_length = (Pos.xRes * Pos.yRes * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;
    
    load_TR(8);

    set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

    sys_vector_init();

    memory_management_struct.start_code = (unsigned long)& _text;
    memory_management_struct.end_code = (unsigned long)& _etext;
    memory_management_struct.end_data = (unsigned long)& _edata;
    memory_management_struct.end_brk = (unsigned long)& _end;

    color_printk(RED, BLACK, "Memory Init\n");

    init_memory();

    color_printk(RED,BLACK,"memory_management_struct.bits_map:%#018lx\n",*memory_management_struct.bits_map);
	color_printk(RED,BLACK,"memory_management_struct.bits_map:%#018lx\n",*(memory_management_struct.bits_map + 1));
	struct Page *page = alloc_pages(ZONE_NORMAL,64,PG_PTable_Maped | PG_Active | PG_Kernel);
	for(int i = 0;i < 64;i++){
		color_printk(INDIGO,BLACK,"page%d\tattribute:%#018lx\taddress:%#018lx\t",i,(page + i)->attr,(page + i)->PHY_address);
		i++;
		color_printk(INDIGO,BLACK,"page%d\tattribute:%#018lx\taddress:%#018lx\n",i,(page + i)->attr,(page + i)->PHY_address);
	}
	color_printk(RED,BLACK,"memory_management_struct.bits_map:%#018lx\n",*memory_management_struct.bits_map);
	color_printk(RED,BLACK,"memory_management_struct.bits_map:%#018lx\n",*(memory_management_struct.bits_map + 1));
    
    while(1);

}
