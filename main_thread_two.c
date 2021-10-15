#include "TM4C123GH6PM.h"
#include "globals.h"
#include "fifo.h"

void Main_Thread_Two(void) {
	uint32_t data = OS_FIFO_Peek();
	GPIOF->DATA |= 0x0E & data;             // loading port F
}