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
	if (CurrentSize == FIFOSIZE ) {
		LostData++;	// error
		return -1;
	}
	
	*(PutPt) = data;	// Put
	PutPt++;					// place for next
	
	if (PutPt == &FIFO[FIFOSIZE]) {
		PutPt = &FIFO[0]; // wrap
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

int OS_FIFO_Full(void) {
	return CurrentSize == FIFOSIZE;
}

int OS_FIFO_Empty(void) {
	return CurrentSize == 0;
}

// Get the data at GetPt's position
// without changing the FIFO.
uint32_t OS_FIFO_Peek() {
	uint32_t data;
	OS_Wait(&CurrentSize);
	OS_Wait(&FIFOMutex);

	// FIFO has at least one item, exclusive access
	
	data = *(GetPt);
	
	return data;
}

// Get the data at GetPt's next position
// without changing the FIFO

// Returns -1 if FIFO is empty after next
// call to OS_FIFO_Get()
int32_t OS_FIFO_Double_Peek() {
	uint32_t data;
	uint32_t* pt;
	
	OS_Wait(&CurrentSize);
	OS_Wait(&FIFOMutex);
	
	// FIFO has at least one item, exclusive access
	
	if (CurrentSize == 1) {
		// FIFO only had one item, return -1
		return -1;
	}
	
	// FIFO has at least two items
	
	pt = GetPt + 1;
	if (pt == &FIFO[FIFOSIZE]) {
		pt = &FIFO[0]; // wrap
	}
	
	data = *(pt);
	return (int32_t) data;
}