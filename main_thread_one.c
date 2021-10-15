#include <stdint.h>

#include "os.h"
#include "globals.h"
#include "fifo.h"
#include "tm4c123gh6pm_def.h"

uint32_t DIP_VALUES;

void Main_Thread_One(void) {
	// Step #1: Read DIP switch values
	DIP_VALUES = GPIO_PORTD_DATA_R & 0x0E; // read PD3, PD2, & PD1
	
	// Step #2: Is SW5 pressed and debounced?
	if (GPIO_PORTD_DATA_R & 0x01) {
		OS_Sleep(5);	// 10 ms
		GPIO_PORTD_DATA_R |= 0x0E & DIP_VALUES;
		
		// check if button still pressed
		if (GPIO_PORTD_DATA_R & 0x01) {
			OS_FIFO_Put(DIP_VALUES);
		}
	}
}