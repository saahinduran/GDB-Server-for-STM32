/*
 * debug.c
 *
 *  Created on: Aug 23, 2024
 *      Author: sahin
 */

#include "debug.h"

extern UART_HandleTypeDef huart3;
extern char rcv;
static volatile int willStop = 1;

void ProcessDebugRequest(char ch);

static eDebugState s_user_requested_debug_state = kDebugState_None;

static BreakPointStc BreakPointArray[MAX_BREAKPOINT_NUM];

static uint32_t ActiveBreakPointCnt = 0;

void EXAMPLE_LOG(const char *format, ...) {
    char buffer[256];  // Buffer to hold the formatted string
    va_list args;

    // Initialize the va_list with the variable arguments
    va_start(args, format);

    // Format the string into the buffer
    vsnprintf(buffer, sizeof(buffer), format, args);

    // Send the formatted string over UART
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    // Clean up the va_list
    va_end(args);
}




bool debug_monitor_enable(void) {


  volatile uint32_t *dhcsr = (uint32_t*)0xE000EDF0;
  if ((*dhcsr & 0x1) != 0) {
    EXAMPLE_LOG("Halting Debug Enabled - "
                "Can't Enable Monitor Mode Debug!\r\n");
    return false;
  }

  volatile uint32_t *demcr = (uint32_t*)0xE000EDFC;
  const uint32_t mon_en_bit = 16;
  *demcr |= 1 << mon_en_bit;


  // Priority for DebugMonitor Exception is bits[7:0].
  // We will use the lowest priority so other ISRs can
  // fire while in the DebugMonitor Interrupt
  volatile uint32_t *shpr3 = (uint32_t *)0xE000ED20;
  *shpr3 = 0xff;

  EXAMPLE_LOG("Monitor Mode Debug Enabled!\r\n");
  return true;
}




void debug_monitor_handler_c(sContextStateFrame *frame)
{

	volatile uint32_t *demcr = (uint32_t *)0xE000EDFC;
	const uint32_t addrofR7 = (uint32_t)frame - 0x08;
	const uint32_t addrofLR = (uint32_t)frame - 0x04;

	volatile uint32_t *dfsr = (uint32_t *)0xE000ED30;
	const uint32_t dfsr_dwt_evt_bitmask = (1 << 2);
	const uint32_t dfsr_bkpt_evt_bitmask = (1 << 1);
	const uint32_t dfsr_halt_evt_bitmask = (1 << 0);
	const bool is_dwt_dbg_evt = (*dfsr & dfsr_dwt_evt_bitmask);
	const bool is_bkpt_dbg_evt = (*dfsr & dfsr_bkpt_evt_bitmask);
	const bool is_halt_dbg_evt = (*dfsr & dfsr_halt_evt_bitmask);
	const uint32_t demcr_single_step_mask = (1 << 18);

	DEactivateAllBreakPoints();
	HAL_UART_AbortReceive_IT(&huart3);
	//TODO: make sure to deactivate all breakpoints
	if(is_bkpt_dbg_evt)
	{
		RemoveBreakPoint(frame->pc) ;
		asm volatile("dsb");
		asm volatile("isb");

	}

	EXAMPLE_LOG("DebugMonitor Exception\r\n");

	EXAMPLE_LOG("DEMCR: 0x%08x\r\n", *demcr);
	EXAMPLE_LOG("DFSR:  0x%08x (bkpt:%d, halt:%d, dwt:%d\r\n)", *dfsr,
			(int)is_bkpt_dbg_evt, (int)is_halt_dbg_evt,
			(int)is_dwt_dbg_evt);

	EXAMPLE_LOG("Register Dump\r\n");
	EXAMPLE_LOG(" r0  =0x%08x\r\n", frame->r0);
	EXAMPLE_LOG(" r1  =0x%08x\r\n", frame->r1);
	EXAMPLE_LOG(" r2  =0x%08x\r\n", frame->r2);
	EXAMPLE_LOG(" r3  =0x%08x\r\n", frame->r3);
	EXAMPLE_LOG(" r4  =0x%08x\r\n", frame->r4);
	EXAMPLE_LOG(" r5  =0x%08x\r\n", frame->r5);
	EXAMPLE_LOG(" r6  =0x%08x\r\n", frame->r6);
	EXAMPLE_LOG(" r7  =0x%08x\r\n", frame->r7);
	EXAMPLE_LOG(" r8  =0x%08x\r\n", frame->r8);
	EXAMPLE_LOG(" r9  =0x%08x\r\n", frame->r9);
	EXAMPLE_LOG(" r10 =0x%08x\r\n", frame->r10);
	EXAMPLE_LOG(" r11 =0x%08x\r\n", frame->r11);
	EXAMPLE_LOG(" r12 =0x%08x\r\n", frame->r12);
	EXAMPLE_LOG(" sp  =0x%08x\r\n", frame->sp);
	EXAMPLE_LOG(" lr  =0x%08x\r\n", frame->lr);
	EXAMPLE_LOG(" pc  =0x%08x\r\n", frame->pc);
	EXAMPLE_LOG(" xpsr=0x%08x\r\n", frame->xpsr);



	if (is_dwt_dbg_evt || is_bkpt_dbg_evt ||
			(s_user_requested_debug_state == kDebugState_SingleStep))  {
		EXAMPLE_LOG("Debug Event Detected, Awaiting 'c' or 's'");
		while (1) {
			char c;
			/* we have to call this function in every loop
			 * because there are common codes that this ISR and user share...
			 * for example, HAL_GetTick...
			 */

			DEactivateAllBreakPoints();

			HAL_UART_AbortReceive_IT(&huart3);
			if (HAL_OK != HAL_UART_Receive(&huart3, &c, 1, HAL_MAX_DELAY) ) {
				continue;
			}


			if (c == 'c')
			{
				s_user_requested_debug_state = kDebugState_None;
				EXAMPLE_LOG("Got char '%c'!\n", c);
				break;
			}

			else if (c == 's')
			{
				s_user_requested_debug_state = kDebugState_SingleStep;
				EXAMPLE_LOG("Got char '%c'!\n", c);
				break;
			}

			else
			{
				ProcessDebugRequest(c);
			}

		}
	}
	else
	{
		EXAMPLE_LOG("Resuming ...\n\r");
	}

    if (is_bkpt_dbg_evt)
    {
      // single-step to the next instruction
      // This will cause a DebugMonitor interrupt to fire
      // once we return from the exception and a single
      // instruction has been executed. The HALTED bit
      // will be set in the DFSR when this happens.


      if(kDebugState_SingleStep == s_user_requested_debug_state)
      {
    	  *demcr |= (demcr_single_step_mask);
      }

      // We have serviced the breakpoint event so clear mask
      *dfsr = dfsr_bkpt_evt_bitmask;

    }
    else if (is_halt_dbg_evt)
    {

      // re-enable FPB in case we got here via single-step
      // for a BKPT debug event
      //fpb_enable();


      if (s_user_requested_debug_state != kDebugState_SingleStep)
      {
        *demcr &= ~(demcr_single_step_mask);
      }

      // We have serviced the single step event so clear mask
      *dfsr = dfsr_halt_evt_bitmask;

    }
    REActivateAllBreakPoints();
    HAL_UART_Receive_IT(&huart3, &rcv, 1);

}

void DEactivateAllBreakPoints(void)
{
	int i;
	for(i = 0; i < MAX_BREAKPOINT_NUM; i++)
	{
		if(BreakPointArray[i].isActive == 1)
		{
			*(uint32_t *)BreakPointArray[i].addr =
					BreakPointArray[i].backUpInstruction;
		}
	}

}

void REActivateAllBreakPoints(void)
{
	int i;
	for(i = 0; i < MAX_BREAKPOINT_NUM; i++)
	{
		if(BreakPointArray[i].isActive == 1)
		{
			*(uint16_t *)BreakPointArray[i].addr = BREAKPOINT_INSTRUCTION(0);
		}
	}
}


bool InsertBreakPoint(uint32_t addr)
{
	int i, availableSlot;
	bool retVal = true;

	if(addr & 0x01) addr--;
	if(ActiveBreakPointCnt +1 > MAX_BREAKPOINT_NUM)
	{
		return false;
	}
	for(i = ActiveBreakPointCnt; i > 0; i--)
	{
		if(BreakPointArray[i].addr == addr && BreakPointArray[i].isActive == 1 )
		{
			return false;
		}
	}

	availableSlot = FindAvailableSlot();
	BreakPointArray[availableSlot].addr = addr;
	BreakPointArray[availableSlot].backUpInstruction = *(uint32_t *)addr;
	BreakPointArray[availableSlot].isActive = 1;

	*(uint16_t *)addr = BREAKPOINT_INSTRUCTION(0);
	asm volatile("dsb");
	asm volatile("isb");

	ActiveBreakPointCnt += 1;
	return true;
}

bool RemoveBreakPoint(uint32_t addr)
{
	int i, availableSlot;
	bool isBkptFound = false;

	if(addr & 0x01) addr--;

	for(i = 0; i < MAX_BREAKPOINT_NUM; i++)
	{
		if(BreakPointArray[i].addr == addr && BreakPointArray[i].isActive == 1) /* If breakpoint is found and also active */
		{
			BreakPointArray[i].isActive = 0;
			*(uint32_t *)addr = BreakPointArray[i].backUpInstruction;
			ActiveBreakPointCnt -= 1;

			isBkptFound = true;

			asm volatile("dsb");
			asm volatile("isb");
			break;
		}
	}

	return isBkptFound;

}

int FindAvailableSlot(void)
{
	int i;
	if(!ActiveBreakPointCnt) return 0;

	for(i = 0; i < MAX_BREAKPOINT_NUM; i++)
	{
		if(BreakPointArray[i].isActive == 0)
		{
			return i;
		}
	}

	return -1;


}
