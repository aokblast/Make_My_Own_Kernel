#include "lib.h"
#include "printk.h"

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
    int i = 1 / 0;
    while(1);

}