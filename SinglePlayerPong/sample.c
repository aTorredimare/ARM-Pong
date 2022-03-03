#include <stdio.h>
#include "LPC17xx.H"                    /* LPC17xx definitions                */
#include "led/led.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "adc/adc.h"
#include "GLCD/GLCD.h"

volatile int	score = 0;
volatile int record = 100;

volatile int gamestate = 0; //0-->paused, 1-->playing

char startmsg1[20] = "Welcome to Pong!";
char startmsg2[30] = "Press key1 to start..";

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
	SystemInit();  												/* System Initialization (i.e., PLL)  */
	BUTTON_init();												/*Button Initialization */
 	init_RIT(0x4C4B40);									/* RIT Initialization 10 msec       	*/
	init_timer(1,0xF4240);							//TIM1 inizializatyion 1 ms
	ADC_init();
	LCD_Initialization();
	
	LCD_DrawField();
	
	GUI_Text(55,70,(uint8_t *) startmsg1,White,Black);
	GUI_Text(40,90,(uint8_t *) startmsg2,White,Black);
												
	
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);			

	while (1) {                           /* Loop forever                       */	
		__ASM("wfi");
  }

}
