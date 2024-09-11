/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern UART_HandleTypeDef huart3;
extern char rcv;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern int willHalt;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */

void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
	sExceptionFrame *frame;
	asm volatile("b .");


	/* Demo purposes, to see if a breakpoint is reached at exception mode */
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

	/* IMPORTANT EXCEPITON HANDLING HERE!
	 * WHEN THE PROCESSOR EXECUTES A BKPT INSTRUCTION WHILE IT IS IN HANDLER MODE,
	 * IT WILL GENERATE A HARD FAULT
	 * THIS IS EXPLAINED IN ARM V7-M ARM DOCUMENT WITH NUMER DDI0403_e
	 * WE HAVE TO RECOVER THIS SITUATION BEFORE OUR APPLICATION CRASHES
	 */

	/* IF YOU WANT TO DEBUG EXCEPTION HANDLING CODE (LOWER LEVEL CODE)
	 * YOU HAVE TO INCREASE DEBUG MONITOR HANDLER'S PRIORITY AND LOWER
	 * THE PRIORITY OF THE CODE YOU WANT TO DEBUG
	 */
	asm volatile("MOV %0, sp":"=r"(frame)::);
	frame = (sExceptionFrame *)( (uint32_t)frame + 0x10);

	if( *(uint16_t *)frame->pc == BREAKPOINT_INSTRUCTION(0) )
	{
		RemoveBreakPoint(frame->pc);
	}

	else
	{
		while(1);
	}
  /* USER CODE END HardFault_IRQn 0 */

}



/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

__attribute__((naked)) void DebugMon_Handler(void)
{

	/* Save context to report it to client */
	asm volatile("PUSH {R4-R11}");
	asm volatile("MRS R0, EAPSR");
	asm volatile("PUSH {R0}");

	asm volatile("MOV R0, SP");
	asm volatile("ADD R0, #68");

	/* Check if processor aligned the stack to 8 byte boundary ? */
	asm volatile("TST R0, 0b111");
	asm volatile("IT ne");
	asm volatile("SUBNE R0,#4 ");

	asm volatile("PUSH {R0}");



	asm volatile("PUSH {LR}");
  __asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq r0, msp \n"
      "mrsne r0, psp \n"
	   "add r0, #4 \n"
      "bl debug_monitor_handler_c \n");



  asm volatile("POP {LR}");
  asm volatile("ADD SP ,#40");
  asm volatile("dsb");
  asm volatile("isb");
  asm volatile("BX LR");
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
	sExceptionFrame *frame;
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
  /* If 'h' character is sent from UART, halt the processor by inserting a bkpt
   * to where the exception come from */
  if(willHalt)
  {
	  //asm volatile("mov %0, %1":"=r"():"r"():);
	  asm volatile("mrs r0, msp");
	  asm volatile("mov %0, r0":"=r"(frame)::);
	  frame = (sExceptionFrame *)( (uint32_t)frame + 16);
	  InsertBreakPoint(frame->pc);

  }

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(USER_Btn_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
