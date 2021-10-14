#include <stdint.h>

#include "globals.h"
#include "LCD.h"
#include "fifo.h"

void Output_Color_LCD(int32_t color) {
  // colors
	
	// G B R
	// 0 0 0 -> LED is off
	// 0 0 1 -> LED is Red
	// 0 1 0 -> LED is Blue
	// 0 1 1 -> LED is Purple
	// 1 0 0 -> LED is Green
	// 1 0 1 -> LED is Yellow
	// 1 1 0 -> LED is Cyan
	// 1 1 1 -> LEd is White
	char* unknown = "??";
	char* off = "Off";
	char* green = "Grn";
	char* red = "Red";
	char* blue = "Blu";
	char* purple = "Prpl";
	char* yellow = "Yelw";
	char* cyan = "Cyan";
	char* white = "Whte";
	
	switch (color) {
			case 0x00:
				Display_Msg(off);
				break;
			
			case 0x01:
				Display_Msg(red);
				break;
			
			case 0x02:
				Display_Msg(blue);
				break;
			
			case 0x03:
				Display_Msg(purple);
				break;
		
			case 0x04:
				Display_Msg(green);
				break;
			
			case 0x05:
				Display_Msg(yellow);
				break;
			
			case 0x06:
				Display_Msg(cyan);
				break;
			
			case 0x07:
				Display_Msg(white);
				break;
			
			default:
				Display_Msg(unknown);
				break;
	}
}

void Main_Thread_Three(void) {
	// Step #1: Clear LCD
	WriteCMD(0x01);
	OS_Sleep(2); // sleep for 4 ms 
	
	char* full = "Buffer full!";
	char* empty = "Input a color!";
	char* switches = "Switches: ";
	
	uint32_t CurrentColor;
	int32_t NextColor;
	
	// Step #2: Check if FIFO is full
	if (OS_FIFO_Full()) {
		Set_Position(0x02);
		Display_Msg(full);
	} else {
		// FIFO not full
		// Output current value of switches
		Set_Position(0x00);
		Display_Msg(switches);
		
		// output current value of dip switches
		Output_Color_LCD((int32_t) DIP_VALUES >> 1);
	}
	
	if (OS_FIFO_Empty()) {
		// FIFO empty
		Set_Position(0x40);
		Display_Msg(empty);
	} else {
		// Output current color on LED
		Set_Position(0x40);
		Display_Msg("C:");
		CurrentColor = OS_FIFO_Peek() >> 1;
		Output_Color_LCD((int32_t) CurrentColor);
		
		// Output next color on LED
		Set_Position(0x47);
		Display_Msg("N:");
		NextColor = OS_FIFO_Double_Peek() >> 1;
		Output_Color_LCD(NextColor);
	}
}