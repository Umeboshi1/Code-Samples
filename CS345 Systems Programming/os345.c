// os345.c - OS Kernel	06/21/2020
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the BYU CS345 projects.      **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>

#include "os345.h"
#include "os345signals.h"
#include "os345config.h"
#include "os345lc3.h"
#include "os345fat.h"

#define NUM_PARENTS			5

// **********************************************************************
//	local prototypes
//
void pollInterrupts(void);
static int scheduler(void);
static int dispatcher(void);

//static void keyboard_isr(void);
//static void timer_isr(void);

int sysKillTask(int taskId);
static int initOS(void);

// **********************************************************************
// **********************************************************************
// global semaphores

Semaphore* semaphoreList;			// linked list of active semaphores

Semaphore* keyboard;				// keyboard semaphore
Semaphore* charReady;				// character has been entered
Semaphore* inBufferReady;			// input buffer ready semaphore

Semaphore* tics1sec;				// 1 second semaphore
Semaphore* tics10sec;
Semaphore* tics10thsec;				// 1/10 second semaphore

// **********************************************************************
// **********************************************************************
// global system variables

TCB tcb[MAX_TASKS];					// task control block
Semaphore* taskSems[MAX_TASKS];		// task semaphore
jmp_buf k_context;					// context of kernel stack
jmp_buf reset_context;				// context of kernel stack
volatile void* temp;				// temp pointer used in dispatcher

int scheduler_mode;					// scheduler mode
int superMode;						// system mode
int curTask;						// current task #
long swapCount;						// number of re-schedule cycles
char inChar;						// last entered character
int charFlag;						// 0 => buffered input
int inBufIndx;						// input pointer into input buffer
char inBuffer[INBUF_SIZE+1];		// character input buffer
//Message messages[NUM_MESSAGES];		// process message buffers

int pollClock;						// current clock()
int lastPollClock;					// last pollClock
bool diskMounted;					// disk has been mounted

time_t oldTime1;					// old 1sec time
time_t oldTime10;
clock_t myClkTime;
clock_t myOldClkTime;
int * rq;							// ready priority queue
DeltaClock * dc; 

extern int superMode;

Semaphore* deltaMutex;

extern long int group_count[];
extern int num_siblings[];

int parentCount = 0;
int totalTicks = 0;
int totalTicksLeft = 0;

int parentID[NUM_PARENTS];
int childTicks[NUM_PARENTS];

// **********************************************************************
// **********************************************************************
// OS startup
//
// 1. Init OS
// 2. Define reset longjmp vector
// 3. Define global system semaphores
// 4. Create CLI task
// 5. Enter scheduling/idle loop
//
int main(int argc, char* argv[])
{
	// save context for restart (a system reset would return here...)
	int resetCode = setjmp(reset_context);
	superMode = TRUE;						// supervisor mode

	switch (resetCode)
	{
		case POWER_DOWN_QUIT:				// quit
			powerDown(0);
			printf("\nGoodbye!!");
			return 0;

		case POWER_DOWN_RESTART:			// restart
			powerDown(resetCode);
			printf("\nRestarting system...\n");

		case POWER_UP:						// startup
			break;

		default:
			printf("\nShutting down due to error %d", resetCode);
			powerDown(resetCode);
			return resetCode;
	}

	// output header message
	printf("%s", STARTUP_MSG);

	// initalize OS
	if ( resetCode = initOS()) return resetCode;

	// create global/system semaphores here
	//?? vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	charReady = createSemaphore("charReady", BINARY, 0);
	inBufferReady = createSemaphore("inBufferReady", BINARY, 0);
	keyboard = createSemaphore("keyboard", BINARY, 1);
	tics1sec = createSemaphore("tics1sec", BINARY, 0);
	tics10thsec = createSemaphore("tics10thsec", BINARY, 0);
	tics10sec = createSemaphore("tics10sec", COUNTING, 0);

	deltaMutex = createSemaphore("delta mutex", BINARY, 1);

	//?? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// schedule CLI task
	createTask("myShell",			// task name
					P1_main,		// task
					MED_PRIORITY,	// task priority
					argc,			// task arg count
					argv);			// task argument pointers

	// HERE WE GO................

	// Scheduling loop
	// 1. Check for asynchronous events (character inputs, timers, etc.)
	// 2. Choose a ready task to schedule
	// 3. Dispatch task
	// 4. Loop (forever!)

	while(1)									// scheduling loop
	{
		// check for character / timer interrupts
		pollInterrupts();

		// schedule highest priority ready task
		if ((curTask = scheduler()) < 0) continue;

		// dispatch curTask, quit OS if negative return
		if (dispatcher() < 0) break;
	}											// end of scheduling loop

	// exit os
	longjmp(reset_context, POWER_DOWN_QUIT);
	return 0;
} // end main



// **********************************************************************
// **********************************************************************
// scheduler
//
static int scheduler()
{
	int nextTask;
	// ?? Design and implement a scheduler that will select the next highest
	// ?? priority ready task to pass to the system dispatcher.

	// ?? WARNING: You must NEVER call swapTask() from within this function
	// ?? or any function that it calls.  This is because swapping is
	// ?? handled entirely in the swapTask function, which, in turn, may
	// ?? call this function.  (ie. You would create an infinite loop.)

	// ?? Implement a round-robin, preemptive, prioritized scheduler.

	// ?? This code is simply a round-robin scheduler and is just to get
	// ?? you thinking about scheduling.  You must implement code to handle
	// ?? priorities, clean up dead tasks, and handle semaphores appropriately.
	if (scheduler_mode == 1) {
		if (totalTicks <= 0) {
			for (int i = 0; i < MAX_TASKS; i++) {
				if (tcb[i].name != NULL) {
					if (tcb[i].parent == 0 && i > 0) {
						//printf("PARENTID %d : my ID %d\n", tcb[i].parent, i);
						parentID[parentCount] = i;
						parentCount++;
						if (parentCount > NUM_PARENTS) {
							parentCount = 0; //shouldn't get here
						}
					}
				}
			}
			for (int i = 0; i < NUM_PARENTS; i++) {
				totalTicks += num_siblings[i] + 1;
			}
			totalTicks *= 20;
			totalTicks /= NUM_PARENTS;
			//printf("totalTicks: %d\n", totalTicks);
			for (int i = 0; i < NUM_PARENTS; i++) {
				if (num_siblings[i] > 0) {
					childTicks[i] = totalTicks / (num_siblings[i] + 1);
					//printf("Num ticks %d\n", childTicks[i]);
				}
			}
			for (int i = 0; i < MAX_TASKS; i++) { //give each task its tick values
				if (tcb[i].name != NULL) {
					//printf("task: %d, parent:%d\n", i, tcb[i].parent);
					for (int j = 0; j < NUM_PARENTS; j++) {
						if (tcb[i].parent == parentID[j]) { //is a child
							tcb[i].ticks = childTicks[j];
							totalTicksLeft += childTicks[j];
							break;
						}
						else if (i == parentID[j]) { //is a parent
							tcb[i].ticks = childTicks[j] + (totalTicks % childTicks[j]);
							totalTicksLeft += childTicks[j] + (totalTicks % childTicks[j]);
							break;
						}
					}
					if (tcb[i].ticks == 0) {
						tcb[i].ticks=1;
						totalTicksLeft++;
					}
				}
			}
		}
		//printf("total ticks left: %d\n", totalTicksLeft);
		//return task if 
		int queueLen = rq[0];
		bool nonZeroFound = FALSE;
		while (totalTicksLeft > 0) {
			if ((nextTask = deQ(rq, -1)) >= 0) {

				if (tcb[nextTask].parent == parentID[4]) {
					//printf("here\n");
				}

				if (tcb[nextTask].ticks > 0) {
					tcb[nextTask].ticks--;
					totalTicksLeft--;
					enQ(rq, nextTask, tcb[nextTask].priority);
					if (tcb[nextTask].signal & mySIGSTOP) return -1;
					if ((nextTask >= 0) && (tcb[nextTask].signal & mySIGSTOP)) return -1;
					nonZeroFound = TRUE;
					return nextTask;
				}
				else { //move to back of queue
					enQ(rq, nextTask, tcb[nextTask].priority);
					if (tcb[nextTask].signal & mySIGSTOP) return -1;
				}
				queueLen -= 1;
				if (queueLen <= 0) {
					if (nonZeroFound) {
						nonZeroFound = FALSE;
						queueLen = rq[0];
					}
					else {
						break;
					}
				}
			}
			
		}
		totalTicks = 0;
		totalTicksLeft = 0;
		parentCount = 0;
	}
	if ((nextTask = deQ(rq, -1)) >= 0) {
		enQ(rq, nextTask, tcb[nextTask].priority);
		if (tcb[nextTask].signal & mySIGSTOP) return -1;
	}
	// mask sure nextTask is valid
	if ((nextTask >= 0) && (tcb[nextTask].signal & mySIGSTOP)) return -1;

	return nextTask;

} // end scheduler



// **********************************************************************
// **********************************************************************
// dispatch curTask
//
static int dispatcher()
{
	int result;

	// schedule task
	switch(tcb[curTask].state)
	{
		case S_NEW:
		{
			// new task
			printf("\nNew Task[%d] %s", curTask, tcb[curTask].name);
			tcb[curTask].state = S_RUNNING;	// set task to run state

			// save kernel context for task SWAP's
			if (setjmp(k_context))
			{
				superMode = TRUE;					// supervisor mode
				break;								// context switch to next task
			}

			// move to new task stack (leave room for return value/address)
			temp = (int*)tcb[curTask].stack + (STACK_SIZE-8);
			SET_STACK(temp);
			superMode = FALSE;						// user mode

			// begin execution of new task, pass argc, argv
			result = (*tcb[curTask].task)(tcb[curTask].argc, tcb[curTask].argv);

			// task has completed
			if (result) printf("\nTask[%d] returned error %d", curTask, result);
			else printf("\nTask[%d] returned %d", curTask, result);
			tcb[curTask].state = S_EXIT;			// set task to exit state

			// return to kernal mode
			longjmp(k_context, 1);					// return to kernel
		}

		case S_READY:
		{
			tcb[curTask].state = S_RUNNING;			// set task to run
		}

		case S_RUNNING:
		{
			if (setjmp(k_context))
			{
				// SWAP executed in task
				superMode = TRUE;					// supervisor mode
				break;								// return from task
			}
			if (signals()) break;
			longjmp(tcb[curTask].context, 3); 		// restore task context
		}

		case S_BLOCKED:
		{
			break;
		}

		case S_EXIT:
		{
			if (curTask == 0) return -1;			// if CLI, then quit scheduler
			// release resources and kill task
			sysKillTask(curTask);					// kill current task
			break;
		}

		default:
		{
			printf("Unknown Task[%d] State", curTask);
			longjmp(reset_context, POWER_DOWN_ERROR);
		}
	}
	return 0;
} // end dispatcher



// **********************************************************************
// **********************************************************************
// Do a context switch to next task.

// 1. If scheduling task, return (setjmp returns non-zero value)
// 2. Else, save current task context (setjmp returns zero value)
// 3. Set current task state to READY
// 4. Enter kernel mode (longjmp to k_context)

void swapTask()
{
	assert("SWAP Error" && !superMode);		// assert user mode

	// increment swap cycle counter
	swapCount++;

	// either save current task context or schedule task (return)
	if (setjmp(tcb[curTask].context))
	{
		superMode = FALSE;					// user mode
		return;
	}

	// context switch - move task state to ready
	if (tcb[curTask].state == S_RUNNING) tcb[curTask].state = S_READY;

	// move to kernel mode (reschedule)
	longjmp(k_context, 2);
} // end swapTask



// **********************************************************************
// **********************************************************************
// system utility functions
// **********************************************************************
// **********************************************************************

// **********************************************************************
// **********************************************************************
// initialize operating system
static int initOS()
{
	int i;

	// make any system adjustments (for unblocking keyboard inputs)
	INIT_OS

	// reset system variables
	curTask = 0;						// current task #
	swapCount = 0;						// number of scheduler cycles
	scheduler_mode = 0;					// default scheduler
	inChar = 0;							// last entered character
	charFlag = 0;						// 0 => buffered input
	inBufIndx = 0;						// input pointer into input buffer
	semaphoreList = 0;					// linked list of active semaphores
	diskMounted = 0;					// disk has been mounted

	// malloc ready queue
	rq = calloc((MAX_TASKS + 1) , sizeof(int));
	rq[0] = 0;

	//malloc DeltaClock
	dc = (DeltaClock*)calloc(1, sizeof(DeltaClock));
	dc->times = calloc((MAX_TASKS + 1), sizeof(int));
	dc->size = 0;

	if (rq == NULL) return 99;

	// capture current time
	lastPollClock = clock();			// last pollClock
	time(&oldTime1);
	time(&oldTime10);

	// init system tcb's
	for (i=0; i<MAX_TASKS; i++)
	{
		tcb[i].name = NULL;				// tcb
		taskSems[i] = NULL;				// task semaphore
	}

	// init tcb
	for (i=0; i<MAX_TASKS; i++)
	{
		tcb[i].name = NULL;
	}

	// initialize lc-3 memory
	initLC3Memory(LC3_MEM_FRAME, 0xF800>>6);

	// ?? initialize all execution queues

	return 0;
} // end initOS



// **********************************************************************
// **********************************************************************
// Causes the system to shut down. Use this for critical errors
void powerDown(int code)
{
	int i;
	printf("\nPowerDown Code %d", code);

	// release all system resources.
	printf("\nRecovering Task Resources...");

	// kill all tasks
	for (i = MAX_TASKS-1; i >= 0; i--)
		if(tcb[i].name) sysKillTask(i);

	// delete all semaphores
	while (semaphoreList)
		deleteSemaphore(&semaphoreList);

	// free ready queue
	free(rq);

	// ?? release any other system resources
	// ?? deltaclock (project 3)

	RESTORE_OS
	return;
} // end powerDown

TID deQ(int* queue, TID tid) {
	int indexOfLast = queue[0];
	int foundId;
	if (indexOfLast < 1) {
		//printf("queue is empty");
		return -1;
	}
	int indexToRemove = -1;
	if (tid == -1) {
		indexToRemove = 1; //highest priority
	}
	else { //need to find
		for (int i = 1; i < (indexOfLast+1); i++) {
			if (tid == queue[i]) {
				//printf("%d tid found at %d index\n", tid, i);
				indexToRemove = i;
				break;
			}
			if (i == indexOfLast) {
				//printf("%d tid not found\n");
				return -1;
			}
		}
	}
	foundId = queue[indexToRemove];
	for (int j = indexToRemove; j < indexOfLast; j++) {
		//printf("moving %d tid to index %d\n", queue[j + 1], j);
		queue[j] = queue[j + 1];
	}
	//printf("removing index %d\n", indexOfLast);
	queue[indexOfLast] = 0;
	queue[0]--;
	//printf("after deQ: %d\n", queue[0]);
	return foundId;
}

TID enQ(int* queue, TID tid, int priority) {
	//(index * 2) + 1 => TID ; (index * 2) + 2 => priority
	if (queue[0] < MAX_TASKS - 1) { //if there is room
		int index = queue[0] + 1; //index points at first empty slot
		//printf("index: %d ", index);
		if (index == 1) {
			queue[1] = tid;
			queue[0] = 1; //top of the array
			//printf("-here?-");
			return tid;
		}
		while (1) {
			if ((index > 1) && (priority > tcb[queue[index-1]].priority)) { //move that element up
				queue[index] = queue[index-1]; //TID
				index--; //
			}
			else {
				queue[index] = tid;  //set TID value
				queue[0] += 1;
				//printf("up-");

				//printf("after enQ: %d\n", queue[0]);
				return tid;
			}
		}
	}
	else {
		printf("Queue is full");
		return -1;
	}
}

void insertDeltaClock(int time, Semaphore * sem){
	int timeLeft = time; swapTask();
	int indexToInsert = 0; swapTask();
	semWait(deltaMutex); swapTask();
	for (int i = 0; i < dc->size; i++) {
		swapTask();
		//check if timeLeft is less than time at index
		indexToInsert = i; swapTask();
		if (timeLeft < dc->times[i]) {
			swapTask();
			break;
		}
		else {
			swapTask();
			timeLeft -= dc->times[i]; swapTask();
			indexToInsert++; swapTask();
		}
	}
	for (int i = dc->size; i > indexToInsert; i--) {
		swapTask();
		dc->times[i] = dc->times[i - 1]; swapTask();
		dc->sem[i] = dc->sem[i - 1]; swapTask();
		if (i == indexToInsert + 1) {
			swapTask();
			dc->times[i] -= timeLeft; swapTask();
		}
	}
	
	dc->times[indexToInsert] = timeLeft; swapTask();
	dc->sem[indexToInsert] = sem; swapTask();
	dc->size++; swapTask();
	semSignal(deltaMutex); swapTask();
	return;
}

Semaphore* removeFromFront() {
	Semaphore* tempSem;
	if (dc->size == 0) {
		printf("Queue is empty");
		return NULL;
	}
	tempSem = dc->sem[0];
	for (int i = 0; i < dc->size; i++) {
		dc->times[i] = dc->times[i + 1];
		dc->sem[i] = dc->sem[i + 1];
	}
	dc->size--;
	return tempSem;
}

void tickDeltaClock() {
	assert("Error tick delta" && superMode);
	if (dc->size > 0) {
		dc->times[0]--;
		while (dc-> size > 0 && dc->times[0] <= 0) {
			Semaphore* temp = removeFromFront();
			if (temp != NULL) {
				semSignal(temp);
			}
		}
	}
}