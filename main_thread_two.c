#include "TM4C123GH6PM.h"

#include "globals.h"
#include "fifo.h"

void Main_Thread_Two(void) {
	GPIOF->DATA |= 0X0E & OS_FIFO_Peek();             // loading port F
}