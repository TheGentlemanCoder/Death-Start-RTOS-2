#include "TM4C123GH6PM.h"

#include "globals.h"

void Main_Thread_Two(void) {
	GPIOF->DATA |= 0X0E & NextColor;             // loading port F
}