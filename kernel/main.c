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
    Pos.FB_length = (Pos.xRes * Pos.yRes * 4);

    for(int i = 0; i < 1440 * 20; ++i){
        *((char*) addr + 0) = (char) 0x00;
        *((char*) addr + 1) = (char) 0x00;
        *((char*) addr + 2) = (char) 0xff;
        *((char*) addr + 3) = (char) 0x00;
        ++addr;

    }
    color_printk(YELLOW, BLACK, "Hello\t\t World!\n");

    
    load_TR(8);

    set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

    sys_vector_init();

    memory_management_struct.start_code = (unsigned long)& _text;
    memory_management_struct.end_code = (unsigned long)& _etext;
    memory_management_struct.end_data = (unsigned long)& _edata;
    memory_management_struct.end_brk = (unsigned long)& _end;

    color_printk(RED, BLACK, "Memory Init\n");

    init_memory();
    
    while(1);

}
