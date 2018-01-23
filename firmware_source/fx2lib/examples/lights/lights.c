#include <fx2regs.h>
#include <delay.h>

void main(void)
{
    OEA=0xFF;
    for(;;) {
        IOA = 0;
        delay(100);
        IOA = 0xff;
        delay(100);
    }
}
