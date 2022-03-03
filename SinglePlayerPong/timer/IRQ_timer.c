#include "lpc17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h"
#include "../ADC/adc.h"

#define N 10

extern int gamestate;

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

uint16_t SinTable[45] = {
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};



void TIMER0_IRQHandler (void){ //timer per i suoni
	static int ticks = 0;
	
	//DAC Management
	LPC_DAC->DACR = SinTable[ticks] << 6;
	ticks++;
	if(ticks == 45){
		ticks = 0;
		disable_timer(0);
	}
	
	LPC_TIM0->IR = 1;			/* clear interrupt flag */	
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER1_IRQHandler (void){ //timer per paddle e pallina
	
	//ball movement
	LCD_MoveBall();
	
	/* ADC management */
	ADC_start_conversion();
	LPC_RIT->RICTRL |= 0x1;
	
	LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
**                            End Of File
******************************************************************************/
