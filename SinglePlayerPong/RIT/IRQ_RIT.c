#include "lpc17xx.h"
#include "RIT.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h"

extern int gamestate;
extern int score;

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern int keyPressed;

extern char startmsg1[20];
extern char startmsg2[30];

char pausemsg[30] = "Game paused!";

volatile int start = 0;

void RIT_IRQHandler (void){			
	static int down=0;
	down++;
	
	if((LPC_GPIO2->FIOPIN & (1<<10)) == 0 && keyPressed == 0){ //int0
		reset_RIT();
		if(down == 1){ //reset game
			score = 0;
			disable_timer(1);
			reset_timer(1);
			LCD_DrawField();
			start = 0;
			gamestate = 0;
			GUI_Text(55,70,(uint8_t *) startmsg1,White,Black);
			GUI_Text(40,90,(uint8_t *) startmsg2,White,Black);				
		}
	}
	else if((LPC_GPIO2->FIOPIN & (1<<11)) == 0 && keyPressed == 1){ //key1
		reset_RIT();
		if(down == 1){ //play game
				if(gamestate == 0){
					if(start == 0){
						start = 1;
						GUI_Text(55,70,(uint8_t *) startmsg1,Black,Black);
						GUI_Text(40,90,(uint8_t *) startmsg2,Black,Black);
						LCD_DrawComponents();
						enable_timer(1);
					}
					if(start == 1){
						GUI_Text(75,70,(uint8_t *) pausemsg,Black,Black);
						enable_timer(1);
					}
				gamestate = 1;
			}
		}
	}
	else if((LPC_GPIO2->FIOPIN & (1<<12)) == 0 && keyPressed == 2){ //key2
		reset_RIT();
		if(down == 1){ //pause game
			if(gamestate == 1){
				disable_timer(1);
				GUI_Text(75,70,(uint8_t *) pausemsg,White,Black);
				gamestate = 0;
			}
		}
	}
	
	else {	/* button released */
		down=0;			
		disable_RIT();
		reset_RIT();
		if(keyPressed == 0) {
			LPC_PINCON->PINSEL4 |= (1 << 20);
			NVIC_EnableIRQ(EINT0_IRQn);	
		}	
		if(keyPressed == 1) {
			NVIC_EnableIRQ(EINT1_IRQn);	
			LPC_PINCON->PINSEL4 |= (1 << 22);
		}
		if(keyPressed == 2){
			NVIC_EnableIRQ(EINT2_IRQn);	
			LPC_PINCON->PINSEL4 |= (1 << 24);
		}
	}
		
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

