#include "TM4C123GH6PM.h"
#include "globals.h"
#include "fifo.h"
#include "os.h"

void Main_Thread_Two(void) {
	for (;;) {
		OS_Sleep(7500);
		uint32_t data = OS_FIFO_Get();
		GPIOF->DATA |= 0x0E & data;             // loading port F
	}
}