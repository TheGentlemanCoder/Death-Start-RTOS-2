#include <stdint.h>

void OS_FIFO_Init(void);
int OS_FIFO_Put(uint32_t data);
uint32_t OS_FIFO_Get(void);
int OS_FIFO_Full(void);
int OS_FIFO_Empty(void);
uint32_t OS_FIFO_Peek();
int32_t OS_FIFO_Double_Peek();