/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           lib_timer.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        atomic functions to be used by higher sw levels
** Correlated files:    lib_timer.c, funct_timer.c, IRQ_timer.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void enable_timer( uint8_t timer_num ){
  if ( timer_num == 0 ){
		LPC_TIM0->TCR = 1;
  }
  else if ( timer_num == 1 ){
		LPC_TIM1->TCR = 1;
  }
  return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 ){
		LPC_TIM0->TCR = 0;
  }
  else if ( timer_num == 1 ){
		LPC_TIM1->TCR = 0;
  }
  return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer( uint8_t timer_num ){
  uint32_t regVal;

  if ( timer_num == 0 ){
		regVal = LPC_TIM0->TCR;
		regVal |= 0x02;
		LPC_TIM0->TCR = regVal;
  }
  else if ( timer_num == 1 ){
		regVal = LPC_TIM1->TCR;
		regVal |= 0x02;
		LPC_TIM1->TCR = regVal;
  }
  return;
}

uint32_t init_timer ( uint8_t timer_num, uint32_t TimerInterval ){
  if ( timer_num == 0 ){
		LPC_TIM0->MR0 = TimerInterval;
		LPC_TIM0->MCR = 3;

		NVIC_EnableIRQ(TIMER0_IRQn);
		return (1);
  }
  else if ( timer_num == 1 ){
		LPC_TIM1->MR0 = TimerInterval;
		LPC_TIM1->MCR = 3;				/* Interrupt and Reset on MR1 */

		NVIC_EnableIRQ(TIMER1_IRQn);
		return (1);
  }
	
  return (0);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
