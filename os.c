#include "TM4C123GH6PM.h"
#include "tm4c123gh6pm_def.h"
#include "LCD.h"
#include "main_thread_one.h"
#include "main_thread_two.h"
#include "main_thread_three.h"
#include "globals.h"
#include "fifo.h"
#include "os.h"

/* #define NVIC_ST_CTRL_R          (*((volatile uint32_t *)0xE000E010))
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_R        (*((volatile uint32_t *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile uint32_t *)0xE000E018))
#define NVIC_INT_CTRL_R         (*((volatile uint32_t *)0xE000ED04))
#define NVIC_INT_CTRL_PENDSTSET 0x04000000  // Set pending SysTick interrupt
#define NVIC_SYS_PRI3_R         (*((volatile uint32_t *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
*/


// function definitions in osasm_V2.s
void OS_DisableInterrupts(void); // Disable interrupts
void OS_EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);
void EndCritical(int32_t primask);
void Clock_Init(void);
void StartOS(void);

// Main Threads
void Main_Thread_One(void);
void Main_Thread_Two(void);
void Main_Thread_Three(void);

#define NUMTHREADS  3        // maximum number of threads
#define STACKSIZE   100      // number of 32-bit words in stack
struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running
  struct tcb *next;  // linked-list pointer
	int32_t* blocked;	// semaphore blocking the thread (not blocked if null)
	int32_t sleep;			// timeslices left to sleep (not asleep if 0)
};
typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];
int32_t GBR;
int32_t sLCD;

void PortFD_Init(){
 //Setting up RGB output
	SYSCTL->RCGCGPIO |= 0x28;                   // initialize clock for port F and D 
	while ((SYSCTL->PRGPIO & 0x28) != 0x28) {}; // wait until ready
	GPIOF->PCTL &= ~0x0000FFF0;     // configure port PF1-PF3 as GPIO
	GPIOF->AMSEL &= ~0x0E;          // disable analog mode PF1-PF3
	GPIOF->AFSEL &= ~0x0E;          // disable alternative functions PF1-PF3
	GPIOF->DIR |= 0x0E;             // set pins PF0-PF3 as outputs	
	GPIOF->DEN |= 0x0E;             // enable ports PF1-PF3
	//Setting up DIP switch input
	GPIOD->PCTL &= ~0x0000FFFF;     // configure port D as GPIO
	GPIOD->AMSEL &= ~0x0F;          // disable analog mode PD0-PD3
	GPIOD->AFSEL &= ~0x0F;          // disable alternative functions PD0-PD3
	GPIOD->DIR &= ~0x0F;             // set pins PD0-PD3 as outputs	
	GPIOD->DEN |= 0x0F;             // enable port PD0-PD3
}


// ******** OS_Init ************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: systick, 16 MHz clock
// input:  none
// output: none
void OS_Init(void){
  OS_DisableInterrupts();
  Clock_Init();                 // set processor clock to 16 MHz
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0x00FFFFFF)|0xE0000000; // priority 7
	PortFD_Init(); //Setting up RGB output
	Init_LCD_Ports(); // Init LCD
	Init_LCD();
	OS_InitSemaphore(&sLCD, 0);
	OS_FIFO_Init();
	OS_EnableInterrupts();
}

void SetInitialStack(int i){
  tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}

//******** OS_AddThread ***************
// add three foregound threads to the scheduler
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads(void(*task0)(void),
                 void(*task1)(void),
                 void(*task2)(void)){ int32_t status;
  status = StartCritical();
  tcbs[0].next = &tcbs[1]; // 0 points to 1
  tcbs[1].next = &tcbs[2]; // 1 points to 2
  tcbs[2].next = &tcbs[0]; // 2 points to 0
  SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(task0); // PC
  SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(task1); // PC
  SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(task2); // PC
  RunPt = &tcbs[0];       // thread 0 will run first
  EndCritical(status);
  return 1;               // successful
}

///******** OS_Launch ***************
// start the scheduler, enable interrupts
// Inputs: number of 60ns clock cycles for each time slice
//         (maximum of 24 bits)
// Outputs: none (does not return)
void OS_Launch(uint32_t theTimeSlice){
  NVIC_ST_RELOAD_R = theTimeSlice - 1; // reload value
  NVIC_ST_CTRL_R = 0x00000007; // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}


void Clock_Init(void){
	SYSCTL_RCC_R|=0x810;
	SYSCTL_RCC_R&=~(0x400020);
}

//new code

void Scheduler(void) {
	tcbType* pt;
	pt = RunPt;
	if (NVIC_ST_CTRL_R & 0x10000) {
		// full thread timeslice has passed
		while (pt->next != RunPt) {
			pt = pt->next;
			if (pt->sleep) {
				pt->sleep = (pt->sleep) - 1;
			}
		}
	}
	
	RunPt = RunPt->next; // skip at least one
	while((RunPt->sleep) || (RunPt->blocked)) {
		RunPt = RunPt->next; // find one not sleeping and not blocked
	}
}

void OS_Suspend(void) {
	NVIC_INT_CTRL_R |= 0x04000000; // trigger SysTick
}

void OS_Wait(int32_t *s){
	OS_DisableInterrupts();
	(*s) = (*s) - 1;
	if ((*s) < 0) {
		RunPt->blocked = s;
		OS_EnableInterrupts();
		OS_Suspend();
	}
	
	OS_EnableInterrupts();
}

void OS_Signal(int32_t *s){
	OS_DisableInterrupts();
	tcbType* pt;
	(*s) = (*s) + 1;
	if ((*s) <= 0) {
		pt = RunPt->next;
		
		// search for thread blocked on this semaphore
		while (pt->blocked != s) {
			pt = pt->next;
		}
		
		// wakeup this thread
		pt->blocked = 0;
	}
	
	OS_EnableInterrupts();
}

void OS_Sleep(uint32_t SleepCtr) {
	// SleepCtr is measured in timeslices
	RunPt->sleep = SleepCtr;
	OS_Suspend();
}

void OS_InitSemaphore(int32_t *s, int32_t initialValue) {
	OS_DisableInterrupts();
	*s = initialValue;
	OS_EnableInterrupts();
}

#define TIMESLICE               32000

int main(void){
	OS_AddThreads(&Main_Thread_One, &Main_Thread_Two, &Main_Thread_Three);
  OS_Init();           // initialize, disable interrupts, 16 MHz
  OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}

