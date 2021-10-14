#include <stdint.h>
#include "globals.h"
#include "os.h"

uint32_t volatile *PutPt;	// put next
uint32_t volatile *GetPt;	// get next
uint32_t static FIFO[FIFOSIZE];
int32_t CurrentSize;		// 0 means FIFO is empty
int32_t FIFOMutex;			// exclusive access to FIFO
uint32_t LostData;

void OS_FIFO_Init(void) {
	PutPt = &FIFO[0];
	GetPt = &FIFO[0];
	OS_InitSemaphore(&CurrentSize, 0);
	OS_InitSemaphore(&FIFOMutex, 1);
	LostData = 0;
}

int OS_FIFO_Put(uint32_t data) {
	if (CurrentSize == FIFOSIZE) {
		LostData++;	// error
		return -1;
	}
	
	*(PutPt) = data;	// Put
	PutPt++;					// place for next
	
	if (PutPt == &FIFO[FIFOSIZE]) {
		PutPt = &FIFO[0];
	}
	
	OS_Signal(&CurrentSize);
	return 0; // success
}

uint32_t OS_FIFO_Get(void) {
	uint32_t data;
	OS_Wait(&CurrentSize);
	OS_Wait(&FIFOMutex);
	data = *(GetPt);	// CurrentSize != 0 and exclusive access
	GetPt++; // points to next data to get
	
	if (GetPt == &FIFO[FIFOSIZE]) {
		GetPt = &FIFO[0];	// wrap
	}
	
	OS_Signal(&FIFOMutex);
	return data;
}