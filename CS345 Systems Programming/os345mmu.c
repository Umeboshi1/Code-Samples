// os345mmu.c - LC-3 Memory Management Unit	06/21/2020
//
//		03/12/2015	added PAGE_GET_SIZE to accessPage()
//
// **************************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "os345.h"
#include "os345lc3.h"

// ***********************************************************************
// mmu variables

// LC-3 memory
unsigned short int memory[LC3_MAX_MEMORY];

// statistics
int memAccess;						// memory accesses
int memHits;						// memory hits
int memPageFaults;					// memory faults
int clockRPT;						// RPT clock
int clockUPT;						// UPT clock

bool hasValid = FALSE;

int getFrame(int);
int getAvailableFrame(void);
extern TCB tcb[];					// task control block
extern int curTask;					// current task #

int getFrame(int notme)
{
	int frame;
	frame = getAvailableFrame();
	if (frame >=0) return frame;

	while (1) {
		if (clockRPT < LC3_RPT) {
			clockRPT = LC3_RPT;
		}
		//printf("%x\n", clockRPT);
		if (DEFINED(MEMWORD(clockRPT))) {
			//printf("here %x %x\n", FRAME(MEMWORD(clockRPT)), notme);
			if (REFERENCED(MEMWORD(clockRPT))) {
				MEMWORD(clockRPT) = CLEAR_REF(MEMWORD(clockRPT));
			}
			else {
				//go into user page table
				int clockUPTa;
				clockUPTa = FRAME(MEMWORD(clockRPT)) << 6;
				//hasValid = FALSE;
				
				while (clockUPT < LC3_FRAME_SIZE) {
					if (DEFINED(MEMWORD(clockUPTa + clockUPT))) {
						hasValid = TRUE;
						if (REFERENCED(MEMWORD(clockUPTa + clockUPT))) {
							MEMWORD(clockUPTa + clockUPT) = CLEAR_REF(MEMWORD(clockUPTa + clockUPT));
						}
						else if(FRAME(MEMWORD(clockUPTa + clockUPT)) != notme){
							//PAGED check if paged if paged, swappage
							if (PAGED(MEMWORD(clockUPTa + clockUPT + 1))) {
								//get page number
								if (DIRTY(MEMWORD(clockUPTa + clockUPT))) {
									int pnum = SWAPPAGE(MEMWORD(clockUPTa + clockUPT + 1));
									accessPage(pnum, FRAME(MEMWORD(clockUPTa + clockUPT)), PAGE_OLD_WRITE);
								}
							}
							else {
								int pnum = accessPage(SWAPPAGE(MEMWORD(clockUPTa + clockUPT + 1)), FRAME(MEMWORD(clockUPTa + clockUPT)), PAGE_NEW_WRITE);
								MEMWORD(clockUPTa + clockUPT + 1) = pnum;
								MEMWORD(clockUPTa + clockUPT + 1) = SET_PAGED(MEMWORD(clockUPTa + clockUPT + 1)); //into swap space
							}
							MEMWORD(clockUPTa + clockUPT) = CLEAR_DIRTY(MEMWORD(clockUPTa + clockUPT));
							MEMWORD(clockUPTa + clockUPT) = CLEAR_DEFINED(MEMWORD(clockUPTa + clockUPT));
							MEMWORD(clockUPTa + clockUPT) = CLEAR_REF(MEMWORD(clockUPTa + clockUPT));
							//accesspage pnum
							int newUPTa = clockUPTa;
							int newUPT = clockUPT;
							clockUPT += 2; //check if greater than frame size -> set UPTclock to 0 ++RPTclock
							if (!(clockUPT < LC3_FRAME_SIZE)) {
								clockUPT = 0;
								//hasValid = FALSE;
								clockRPT+=2;
								if (!(clockRPT < LC3_RPT_END)) {
									clockRPT = LC3_RPT;
								}
							}
							//printf("frame %d RPT %x\n", FRAME(MEMWORD(newUPTa + newUPT)), clockRPT);
							return FRAME(MEMWORD(newUPTa + newUPT));
						}
					}
					clockUPT += 2;
					//printf("%d clockUPT | %x clockRPT\n", clockUPT, clockRPT);
				}
				hasValid = FALSE;
				for (int i = 0; i < LC3_FRAME_SIZE; i+=2) {
					if (DEFINED(MEMWORD(clockUPTa + i))) {
						hasValid = TRUE;
					}
				}
				if (!hasValid) {
					if (!REFERENCED(FRAME(MEMWORD(clockRPT)))) {
						if (FRAME(MEMWORD(clockRPT)) != notme) {
							//PAGED check if paged if paged, swappage
							if (PAGED(MEMWORD(clockRPT + 1))) {
								if (DIRTY(MEMWORD(clockRPT))) {
									int pnum = SWAPPAGE(MEMWORD(clockRPT + 1));
									accessPage(pnum, FRAME(MEMWORD(clockRPT)), PAGE_OLD_WRITE);
								}
							}
							else {
								int pnum = accessPage(SWAPPAGE(MEMWORD(clockRPT + 1)), FRAME(MEMWORD(clockRPT)), PAGE_NEW_WRITE);
								MEMWORD(clockRPT + 1) = pnum;
								MEMWORD(clockRPT + 1) = SET_PAGED(MEMWORD(clockRPT + 1)); //into swap space
							}
							MEMWORD(clockRPT) = CLEAR_DIRTY(MEMWORD(clockRPT));
							MEMWORD(clockRPT) = CLEAR_DEFINED(MEMWORD(clockRPT));
							MEMWORD(clockRPT) = CLEAR_REF(MEMWORD(clockRPT));
						}
						else { //
							clockRPT += 2;
							clockUPT = 0;
							if (!(clockRPT < LC3_RPT_END)) {
								clockRPT = LC3_RPT;
							}
							continue;
						}
					}
					int newRPT = clockRPT;
					clockRPT += 2;
					clockUPT = 0;
					if (!(clockRPT < LC3_RPT_END)) {
						clockRPT = LC3_RPT;
					}
					return FRAME(MEMWORD(newRPT));
				}
				clockUPT = 0;
				hasValid = FALSE;
			}
		}
		clockRPT+=2;
		clockUPT = 0;
		if (!(clockRPT < LC3_RPT_END)) {
			clockRPT = LC3_RPT;
		}
	}
	return frame;
}
// **************************************************************************
// **************************************************************************
// LC3 Memory Management Unit
// Virtual Memory Process
// **************************************************************************
//           ___________________________________Frame defined
//          / __________________________________Dirty frame
//         / / _________________________________Referenced frame
//        / / / ________________________________Pinned in memory
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023) (2^10)
//     / / / /     /                 / _________page defined
//    / / / /     /                 / /       __page # (0-4096) (2^12)
//   / / / /     /                 / /       /
//  / / / /     / 	             / /       /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p

#define MMU_ENABLE	0

unsigned short int* getMemAdr(int va, int rwFlag) {
	unsigned short int pa;
	int rpta, rpte1, rpte2;
	int rptFrame, uptFrame;
	int upta, upte1, upte2;

	if (va < 0x3000) return &memory[va];

	rpta = tcb[curTask].RPT + RPTI(va); rpte1 = memory[rpta]; rpte2 = memory[rpta + 1];
	memAccess++;
	if (DEFINED(rpte1)) {
		memHits++;
	}
	else {
		memPageFaults++;
		//rpte1 undefined
		rptFrame = getFrame(-1);
		rpte1 = SET_DEFINED(rptFrame);
		if (PAGED(rpte2)) // UPT frame passed out - read from SWAPPAGE(rpte2) into frame
		{
			accessPage(SWAPPAGE(rpte2), FRAME(rpte1), PAGE_READ);
		}
		else {
			rpte1 = SET_DIRTY(rpte1); rpte2 = 0;
			memset(&memory[FRAME(rpte1) << 6], 0, 64 * sizeof(memory[0]));
			//undefine all uptes
		}

	}
	memory[rpta] = rpte1 = SET_REF(SET_PINNED(rpte1));
	memory[rpta + 1] = rpte2;
	upta = (FRAME(rpte1) << 6) + UPTI(va); upte1 = memory[upta]; upte2 = memory[upta + 1];
	memAccess++;
	if (DEFINED(upte1)) {
		memHits++;
	}
	else {
		memPageFaults++;
		int uptframe = getFrame(FRAME(rpte1));
		upte1 = SET_DEFINED(uptframe);
		if (PAGED(upte2)) {
			accessPage(SWAPPAGE(upte2), FRAME(upte1), PAGE_READ);
		}
		else {
			upte1 = SET_DIRTY(upte1); upte2 = 0;
		}
	}
	memory[upta] = upte1 = SET_REF(upte1); memory[upta + 1] = upte2;
	if (rwFlag) {
		memory[rpta] = SET_DIRTY(rpte1);
		memory[upta] = SET_DIRTY(upte1);
	}
	return &memory[(FRAME(upte1) << 6) + FRAMEOFFSET(va)];
}

// **************************************************************************
// **************************************************************************
// set frames available from sf to ef
//    flg = 0 -> clear all others
//        = 1 -> just add bits
//
void setFrameTableBits(int flg, int sf, int ef)
{	int i, data;
	int adr = LC3_FBT-1;             // index to frame bit table
	int fmask = 0x0001;              // bit mask

	// 1024 frames in LC-3 memory
	for (i=0; i<LC3_FRAMES; i++)
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;
			adr++;
			data = (flg)?MEMWORD(adr):0;
		}
		else fmask = fmask >> 1;
		// allocate frame if in range
		if ( (i >= sf) && (i < ef)) data = data | fmask;
		MEMWORD(adr) = data;
	}
	return;
} // end setFrameTableBits


// **************************************************************************
// get frame from frame bit table (else return -1)
int getAvailableFrame()
{
	int i, data;
	int adr = LC3_FBT - 1;				// index to frame bit table
	int fmask = 0x0001;					// bit mask

	for (i=0; i<LC3_FRAMES; i++)		// look thru all frames
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;				// move to next work
			adr++;
			data = MEMWORD(adr);
		}
		else fmask = fmask >> 1;		// next frame
		// deallocate frame and return frame #
		if (data & fmask)
		{  MEMWORD(adr) = data & ~fmask;
			return i;
		}
	}
	return -1;
} // end getAvailableFrame



// **************************************************************************
// read/write to swap space
int accessPage(int pnum, int frame, int rwnFlg)
{
	static int nextPage;						// swap page size
	static int pageReads;						// page reads
	static int pageWrites;						// page writes
	static unsigned short int swapMemory[LC3_MAX_SWAP_MEMORY];

	if ((nextPage >= LC3_MAX_PAGE) || (pnum >= LC3_MAX_PAGE))
	{
		printf("\nVirtual Memory Space Exceeded!  (%d)", LC3_MAX_PAGE);
		exit(-4);
	}
	switch(rwnFlg)
	{
		case PAGE_INIT:                    		// init paging
			clockRPT = 0;						// clear RPT clock
			clockUPT = 0;						// clear UPT clock
			memAccess = 0;						// memory accesses
			memHits = 0;						// memory hits
			memPageFaults = 0;					// memory faults
			nextPage = 0;						// disk swap space size
			pageReads = 0;						// disk page reads
			pageWrites = 0;						// disk page writes
			return 0;

		case PAGE_GET_SIZE:                    	// return swap size
			return nextPage;

		case PAGE_GET_READS:                   	// return swap reads
			return pageReads;

		case PAGE_GET_WRITES:                    // return swap writes
			return pageWrites;

		case PAGE_GET_ADR:                    	// return page address
			return (int)(&swapMemory[pnum<<6]);

		case PAGE_NEW_WRITE:                   // new write (Drops thru to write old)
			pnum = nextPage++;

		case PAGE_OLD_WRITE:                   // write
			//printf("\n    (%d) Write frame %d (memory[%04x]) to page %d", p.PID, frame, frame<<6, pnum);
			memcpy(&swapMemory[pnum<<6], &memory[frame<<6], 1<<7);
			pageWrites++;
			return pnum;

		case PAGE_READ:                    	// read
			//printf("\n    (%d) Read page %d into frame %d (memory[%04x])", p.PID, pnum, frame, frame<<6);
			memcpy(&memory[frame<<6], &swapMemory[pnum<<6], 1<<7);
			pageReads++;
			return pnum;

		case PAGE_FREE:                   // free page
			printf("\nPAGE_FREE not implemented");
			break;
   }
   return pnum;
} // end accessPage
