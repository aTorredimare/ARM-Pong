#include "lpc17xx.h"
#include "adc.h"
#include "../GLCD/GLCD.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

unsigned short AD_current;   
unsigned short AD_lastvals[3] = {0x7FF}; //last converted values, assegno il valore di centro scala
extern int paddlex;
volatile static int count = 0;

void ADC_IRQHandler(void) {
	unsigned short AD_current;
	int i = 0;
	
	AD_lastvals[count] = ((LPC_ADC->ADGDR>>4) & 0xFFF);/* Read Conversion Result             */
	count = (count+1) % 3;
	
	for(i=0;i<3;i++)
		AD_current += AD_lastvals[i];
	AD_current = AD_current/3;
	
	if(AD_current < 0x7FF){ //sono nella metà sinistra del potenziometro: muovo il paddle verso sinistra
			if(AD_current < 0x3FF)
				paddlex = LCD_MovePaddle(paddlex,-1,6);
			else
				paddlex = LCD_MovePaddle(paddlex,-1,4);
		}
	if(AD_current > 0x7FF){ //sono nella metà destra del potenziometro: muovo il paddle verso destra
			if(AD_current > 0xBFE)
				paddlex = LCD_MovePaddle(paddlex,1,6);
			else
				paddlex = LCD_MovePaddle(paddlex,1,4);
		}

}
