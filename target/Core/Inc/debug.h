/*
 * debug.h
 *
 *  Created on: Aug 23, 2024
 *      Author: sahin
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_


#define MAX_BREAKPOINT_NUM  	8U
#define BREAKPOINT_INSTRUCTION(x) (uint16_t)( 0xBE00 | x)


#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

typedef struct __attribute__((packed)) ContextStateFrame
{
	uint32_t sp;
	uint32_t reserved;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;



	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;

} sContextStateFrame;

typedef struct __attribute__((packed)) ExceptionFrame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;

} sExceptionFrame;


typedef enum
{
  kDebugState_None,
  kDebugState_SingleStep,
} eDebugState;

typedef enum DebugState
{
	NONE,
	READ_MEMORY,
	WRITE_MEMORY,
	CONTINUE,
	HALT,
	STEP,
	INSERT_BREAKPOINT,
	REMOVE_BREAKPOINT,


}DebugStateEnum;

typedef struct BreakPoint
{
	uint32_t addr;
	uint32_t isActive;
	uint32_t backUpInstruction;

}BreakPointStc;




bool debug_monitor_enable(void);
void debug_monitor_handler_c(sContextStateFrame *frame);
bool InsertBreakPoint(uint32_t addr);
bool RemoveBreakPoint(uint32_t addr);



#endif /* INC_DEBUG_H_ */
