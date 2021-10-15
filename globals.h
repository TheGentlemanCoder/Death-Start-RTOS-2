// global pointers
#include <stdint.h>
extern uint32_t volatile *PutPt;	// put next
extern uint32_t volatile *GetPt;	// get next

// FIFO
#define FIFOSIZE 10		// Colors in FIFO = 10
extern uint32_t FIFO[FIFOSIZE];
extern int32_t CurrentSize;		// 0 means FIFO is empty
extern int32_t FIFOMutex;			// exclusive access to FIFO
extern uint32_t LostData;

extern uint32_t CurrentColor;
extern uint32_t NextColor;

// Input values
extern uint32_t DIP_VALUES;

// LCD
extern int32_t sLCD;