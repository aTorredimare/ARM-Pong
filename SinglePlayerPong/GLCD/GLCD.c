#include <stdio.h>
#include "GLCD.h" 
#include "HzLib.h"
#include "AsciiLib.h"
#include "../timer/timer.h"

extern volatile int score;
extern volatile int record;
extern int gamestate;
	
volatile int paddlex = 98;
volatile int ballx = 229;
volatile int bally = 160;
volatile int speedx = -6;
volatile int speedy = 6;

char losemsg1[20] = "YOU LOSE!!";
char losemsg2[20] = "Your score: ";
char losemsg3[30] = "Best score: ";
char sc[5];
char rec[5];
//const int freqs[2] = {424,824}; //divido le note per lo scaling factor che imposto nell'emulatore (2120/5 e 4120/5)
const int freqs[2] = {2120,4120};

#define PADDLE_Y 288
#define PADDLE_THICKNESS 10
#define PADDLE_LENGTH 54
#define BALL_DIM 5
#define MIN_USABLE_X 5
#define MAX_USABLE_X 234
#define SCORE_X 8
#define SCORE_Y 160
#define RECORD_X 206
#define RECORD_Y 8


/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;

/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A	10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void){
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCDÐ´Êý¾Ý
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) {
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    /* Interface A->B */
	LCD_EN(0)	                        /* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)							/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}

/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void wait_delay(int count){
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD¶ÁÊý¾Ý
* Input          : - byte: byte to be read
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) {
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   /* delay some times */
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCDÐ´¼Ä´æÆ÷µØÖ·
* Input          : - index: ¼Ä´æÆ÷µØÖ·
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index){
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCDÐ´¼Ä´æÆ÷Êý¾Ý
* Input          : - index: ¼Ä´æÆ÷Êý¾Ý
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data){				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : ¶ÁÈ¡¿ØÖÆÆ÷Êý¾Ý
* Input          : None
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void){ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue){ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg){
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos){
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code ){
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    { 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void){
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* ¶ÁÈ¡ÆÁID	*/	
	/* ²»Í¬ÆÁÇý¶¯IC ³õÊ¼»¯²»Í¬ */
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* ÐÐÊ×Ö·0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* ÁÐÊ×Ö·0 */     
	}  						
    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : ½«ÆÁÄ»Ìî³ä³ÉÖ¸¶¨µÄÑÕÉ«£¬ÈçÇåÆÁ£¬ÔòÌî³ä 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color){
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB ¸ÄÎª BBBBBGGGGGGRRRRR ¸ñÊ½
* Input          : - color: BRG ÑÕÉ«Öµ  
* Output         : None
* Return         : RGB ÑÕÉ«Öµ
* Attention		 : ÄÚ²¿º¯Êýµ÷ÓÃ
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : »ñÈ¡Ö¸¶¨×ù±êµÄÑÕÉ«Öµ
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : ÔÚÖ¸¶¨×ù±ê»­µã
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: AµãÐÐ×ù±ê
*                  - y1: AµãÁÐ×ù±ê 
*				   - x2: BµãÐÐ×ù±ê
*				   - y2: BµãÁÐ×ù±ê 
*				   - color: ÏßÑÕÉ«
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ){
    short dx,dy;      /* ¶¨ÒåX YÖáÉÏÔö¼ÓµÄ±äÁ¿Öµ */
    short temp;       /* Æðµã ÖÕµã´óÐ¡±È½Ï ½»»»Êý¾ÝÊ±µÄÖÐ¼ä±äÁ¿ */

    if( x0 > x1 ){
	    temp = x1;
			x1 = x0;
			x0 = temp;   
    }
    if( y0 > y1 ){
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;       /* XÖá·½ÏòÉÏµÄÔöÁ¿ */
	dy = y1-y0;       /* YÖá·½ÏòÉÏµÄÔöÁ¿ */

    if( dx == 0 )     /* XÖáÉÏÃ»ÓÐÔöÁ¿ »­´¹Ö±Ïß */ 
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ Ãè´¹Ö±Ïß */
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )     /* YÖáÉÏÃ»ÓÐÔöÁ¿ »­Ë®Æ½Ö±Ïß */ 
    {
        do
        {
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ ÃèË®Æ½Ïß */
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }
	/* ²¼À¼É­ººÄ·(Bresenham)Ëã·¨»­Ïß */
    if( dx > dy )                         /* ¿¿½üXÖá */
    {
	    temp = 2 * dy - dx;               /* ¼ÆËãÏÂ¸öµãµÄÎ»ÖÃ */         
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    /* »­Æðµã */ 
	        x0++;                         /* XÖáÉÏ¼Ó1 */
	        if( temp > 0 )                /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */
	        {
	            y0++;                     /* ÎªÓÒÉÏÏàÁÚµã£¬¼´£¨x0+1,y0+1£© */ 
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;           /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */  
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                      /* ¿¿½üYÖá */       
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
}

int LCD_DrawPaddle(uint16_t startx){
	int i = 0;
	for (i=0;i<PADDLE_THICKNESS;i++){
			LCD_DrawLine(startx,PADDLE_Y-i,startx+PADDLE_LENGTH,PADDLE_Y-i,Green);
	}
	return startx;
}

int LCD_MovePaddle(uint16_t oldposx, int dir, int speed){
	int i = 0;
	int newposx = oldposx+(speed*dir); //calcolo la nuova posizione
	
	if(dir == 1) { //dx
		if((newposx + PADDLE_LENGTH) > MAX_USABLE_X) //per non uscire dal campo
			newposx = MAX_USABLE_X - PADDLE_LENGTH;
		
		for(i=0;i<PADDLE_THICKNESS;i++){
			LCD_DrawLine(oldposx,PADDLE_Y-i,newposx,PADDLE_Y-i,Black); //cancello i primi 10 pixel
			LCD_DrawLine(oldposx+PADDLE_LENGTH,PADDLE_Y-i,newposx+PADDLE_LENGTH,PADDLE_Y-i,Green); //aggiungo 10 pixel alla fine
		}
	}
	
	if( dir == -1){ //sx
		if(newposx < MIN_USABLE_X) //per non uscire dal campo
			newposx = MIN_USABLE_X;
		
		for(i=0;i<PADDLE_THICKNESS;i++){
			LCD_DrawLine(newposx,PADDLE_Y-i,oldposx,PADDLE_Y-i,Green); //cancello i primi 10 pixel
			LCD_DrawLine(newposx+PADDLE_LENGTH,PADDLE_Y-i,oldposx+PADDLE_LENGTH,PADDLE_Y-i,Black); //aggiungo 10 pixel alla fine
		}
	}
	return newposx;
}

void LCD_DrawBall(uint16_t startx, uint16_t starty, uint16_t color){
	int i = 0;
	for (i=0; i<BALL_DIM; i++){
			LCD_DrawLine(startx,starty+i,startx+BALL_DIM,starty+i,color);
	}
}

void LCD_MoveBall(){
	int oldx = ballx;
	int oldy = bally;
	//calcolo la posizione successiva della pallina per capire dove andrà
	int nextx = ballx + speedx; 
	int nexty = bally + speedy;
	
	//coordinate utili
	int first_quarter = paddlex + (PADDLE_LENGTH/4);
	int second_quarter = paddlex + (PADDLE_LENGTH/2);
	int third_quarter = paddlex + 3*(PADDLE_LENGTH/4);
	
	//cancello la pallina
	LCD_DrawBall(ballx,bally,Black); 
	
	//cambio la velocità della pallina in base a cosa colpirà
	if(nexty >= (PADDLE_Y-PADDLE_THICKNESS)){ //paddle
		if(nextx <= paddlex || nextx >= paddlex+PADDLE_LENGTH) {//condizione di sconfitta!
			disable_timer(1);
			gamestate = -1;
			if(score > record)
				record = score;
			LCD_DrawBall(nextx+(speedx*2),nexty+(speedy*2),Blue);
			sprintf(sc,"%d",score);
			sprintf(rec,"%d",record);
			GUI_Text(SCORE_X,SCORE_Y,(uint8_t *) sc, Black, Black);
			GUI_Text(RECORD_X,RECORD_Y,(uint8_t *) rec, Black, Black);
			GUI_Text(85,75,(uint8_t *) losemsg1,White,Black);
			sprintf(losemsg2,"Your score: %d",score);
			GUI_Text(72,90,(uint8_t *) losemsg2,White,Black);
			sprintf(losemsg3,"Best score: %d",record);
			GUI_Text(63,105,(uint8_t *) losemsg3,White,Black);
			return;
		}
		else{ // se colpisce paddle, speedy diventa negativa e speedx varia in base a dove colpisce
			disable_timer(0); //emetto la nota
			reset_timer(0);
			init_timer(0,freqs[0]);
			enable_timer(0);
			
			speedy = -6;
			if(nextx > paddlex && nextx <= first_quarter){
				if(speedx<0)
					speedx = -6;
				else
					speedx = 6;
			}
			if(nextx > first_quarter &&  nextx <= second_quarter){
				if(speedx<0)
					speedx = -3;
				else
					speedx = 3;
			}
			if(nextx > second_quarter && nextx <= third_quarter){
				if(speedx>0)
					speedx = 3;
				else
					speedx = -3;
			}
			if(nextx > third_quarter && nextx <= paddlex + PADDLE_LENGTH){
				if(speedx>0)
					speedx = 6;
				else
					speedx = -6;
			}
				
			LCD_DrawBall(ballx,PADDLE_Y-PADDLE_THICKNESS-BALL_DIM,Green); //disegno e cancello la palla in contatto con il paddle
			LCD_DrawBall(ballx,PADDLE_Y-PADDLE_THICKNESS-BALL_DIM,Black);		
			if(score >= 100)
				score += 10;
			else
				score += 5;
			sprintf(sc,"%d",score);
			GUI_Text(SCORE_X,SCORE_Y,(uint8_t *) sc, Black, Black);
			GUI_Text(SCORE_X,SCORE_Y,(uint8_t *) sc, White, Black);
		}
	}
	if(nextx <= 4) {// se colpisce muro sinistra, speedx diventa positiva
		speedx = -speedx;		
		disable_timer(0);
		reset_timer(0);
		init_timer(0,freqs[1]);
		enable_timer(0);
	}
	if(nextx >= 229){ //se colpisce muro destra, speedx diventa negativa
		speedx = -speedx;
		disable_timer(0);
		reset_timer(0);
		init_timer(0,freqs[1]);
		enable_timer(0);
	}
	if(nexty <= 4){ //se colpisce muro alto, speedy diventa positiva
		speedy = -speedy;
		disable_timer(0);
		reset_timer(0);
		init_timer(0,freqs[1]);
		enable_timer(0);
	}
		
	//sposto la pallina nella posizione corretta e disegno
	ballx += speedx;
	bally += speedy;
	LCD_DrawBall(ballx,bally,Green);
	
	//se la pallina è passata su score o su record li ridisegno
	if(ballx >= MIN_USABLE_X && ballx <= MIN_USABLE_X+30 && bally >= SCORE_Y-10 && bally <= SCORE_Y+25)
		GUI_Text(SCORE_X,SCORE_Y,(uint8_t *) sc, White, Black);
	if(ballx >= MAX_USABLE_X-40 && ballx <= MAX_USABLE_X && bally >= RECORD_Y-5 && bally <= RECORD_Y+21)
		GUI_Text(RECORD_X,RECORD_Y,(uint8_t *) rec, White, Black);	
}
	
void LCD_DrawField(){
	int i = 0;
	
	LCD_Clear(Black);
	for(i=0;i<5;i++){
		LCD_DrawLine(i,0,i,288,Red); //left
		LCD_DrawLine(0,i,239,i,Red); //up
		LCD_DrawLine(239-i,0,239-i,288,Red); //right
	}
	return;
}

void LCD_DrawComponents(){	
	paddlex = 98;
	ballx = 229;
	bally = 160;
	speedx = -6;
	speedy = 6;
	
	sprintf(sc,"%d",score);
	sprintf(rec,"%d",record);
	GUI_Text(SCORE_X,SCORE_Y, (uint8_t *) sc, White, Black);
	GUI_Text(RECORD_X,RECORD_Y, (uint8_t *) rec, White, Black);
	
	LCD_DrawPaddle(paddlex);
	LCD_DrawBall(ballx,bally,Green);
	
}


/******************************************************************************
* Function Name  : PutChar
* Description    : ½«LcdÆÁÉÏÈÎÒâÎ»ÖÃÏÔÊ¾Ò»¸ö×Ö·û
* Input          : - Xpos: Ë®Æ½×ø±ê 
*                  - Ypos: ´¹Ö±×ø±ê  
*				   - ASCI: ÏÔÊ¾µÄ×Ö·û
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor ){
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* È¡×ÖÄ£Êý¾Ý */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* ×Ö·ûÑÕÉ« */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* ±³¾°ÑÕÉ« */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : ÔÚÖ¸¶¨×ù±êÏÔÊ¾×Ö·û´®
* Input          : - Xpos: ÐÐ×ù±ê
*                  - Ypos: ÁÐ×ù±ê 
*				   - str: ×Ö·û´®
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}



/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
