#include "button.h"
#include "lpc17xx.h"

#include "../RIT/RIT.h"		  			/* you now need RIT library 			 */

volatile int keyPressed;

void EINT0_IRQHandler (void){	  	/* INT0														 */
	enable_RIT();										/* enable RIT to count 10ms				 */
	NVIC_DisableIRQ(EINT0_IRQn);
	keyPressed = 0;
	LPC_PINCON->PINSEL4    &= ~(1 << 20);
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void){			/* KEY1														 */
	enable_RIT();										/* enable RIT to count 10ms				 */
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
	keyPressed = 1;
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void){	  	/* KEY2														 */
	enable_RIT();										/* enable RIT to count 10ms				 */
	NVIC_DisableIRQ(EINT2_IRQn);
	keyPressed = 2;
	LPC_PINCON->PINSEL4    &= ~(1 << 24);
	
	LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


