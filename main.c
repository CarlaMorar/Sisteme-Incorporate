#include <hidef.h>      /* common defines and macros */
#include "mc9s12dg256.h"      /* derivative-specific definitions */
#include <string.h>
#include<stdio.h>


#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02

void COMWRT4(unsigned char);
void DATWRT4(unsigned char);
void MSDelay(unsigned int);
void PRSTR(unsigned char *);
void LUM(void);
void TEMP(void);

void main(void)
{

DDRK = 0xFF; //port K declared as output
DDRB=0xFF; //port  B declared as output

DDRM=0xFF; //port M declared as output
PTM&=0xFF;

DDRJ=0xFF; //port J declared as output
PTJ=0;
DDRP=0xFF; //port P declared as output
PTP=0;

COMWRT4(0x33);   //reset sequence provided by data sheet
MSDelay(1);
COMWRT4(0x32);   //reset sequence provided by data sheet
MSDelay(1);
COMWRT4(0x28);   //Function set to four bit data length
//2 line, 5 x 7 dot format
MSDelay(1);
COMWRT4(0x06);  //entry mode set, increment, no shift
MSDelay(1);
COMWRT4(0x0E);  //Display set, disp on, cursor on, blink off
MSDelay(1);
COMWRT4(0x01);  //Clear display
MSDelay(1);
COMWRT4(0x80);  //set start posistion, home position
MSDelay(1);
       
        for(;;){   //infinite loop for alternatively measuring temperature and luminosity
        
        MSDelay(500);
        TEMP();
        MSDelay(500);
        LUM();
        }
}
  void LUM(void){
      int i;
      unsigned char display[20]; //the string that will be printed on the LCD display
      ATD0CTL2=0x80;   //Normal ATD functionality
      for(i=0;i<1000;i++);
      ATD0CTL3=0x08; //one conversion per sequence
      ATD0CTL4=0xEB;
// 8 bit resolution; 16 A/D conversion clock periods; PRS=11 =>ATDclock=1MHz
      ATD0CTL5=0xA4;
 //right justified data in the result registers; unsigned data; 
// continuous conversion  sequences
     	//sample only one channel
//AN4 is the selected input channel whose signals are sampled and converted to digital //codes
      MSDelay(1000);
      while(!(ATD0STAT0&0x80)); //wait until the conversion sequence has completed
       if(ATD0STAT0&0x80){
       sprintf(display,"LUM:%3d",ATD0DR0L); 
 //store the converted value(stored in ATD0DR0L)
        display[strlen(display)]='\0';
        COMWRT4(0xC0); //set start position (second row)
        MSDelay(1);
        PRSTR(display); //display on the LCD
        if(ATD0DR0L < 50){
        unsigned int i;
        PTM&=0xFF;//our attempt to stop the blue led
           for(i=0;i<5;i++){
           PTM&=0xFF; // our attempt to stop the blue led
            PTP=0x20;
            MSDelay(100);
            PTP=0x00;
            MSDelay(50);
            PTP=0x20;
            MSDelay(130);
            PTP=0x00;
            MSDelay(50);
           }
        }
        ATD0STAT0|=0x80; //reset
        }
     // }
      
  }                   //DAC //PT5
  
  void TEMP(void){
      int i;
      float aux;
      unsigned char display[20];
      ATD0CTL2=0x80;   //Normal ATD functionality
      for(i=0;i<1000;i++);
      ATD0CTL3=0x08; //one conversion per sequence
      ATD0CTL4=0xEB;
// 8 bit resolution; 16 A/D conversion clock periods; PRS=11 =>ATDclock=1MHz
      ATD0CTL5=0xA5;
//right justified data in the result registers; unsigned data; 
// continuous conversion  sequences
     	//sample only one channel
//AN5 is the selected input channel whose signals are sampled and converted to digital //codes
 
      MSDelay(1000);
            while(!(ATD0STAT0&0x80)); //wait until the conversion sequence has completed
        if(ATD0STAT0&0x80){
        aux= (ATD0DR0L*0.019)*100; //converting the result to Celsius
        if(aux>30) 
  PORTB=0xAA; 
  //chosen configuration for the leds when the temperature exceeds the value 30
        else PORTB=0x00; //leds are off
        sprintf(display,"TEMP:%0.2f",aux);
        display[strlen(display)]='\0';
        COMWRT4(0x80); //set start position //first row
       
        MSDelay(1);
        PRSTR(display); //display the temperature
        ATD0STAT0|=0x80; //reset
        }
      
  }
  
void COMWRT4(unsigned char command)
  {
        unsigned char x;
        
        x = (command & 0xF0) >> 2;         //shift high nibble to center of byte for Pk5-Pk2
      LCD_DATA =LCD_DATA & ~0x3C;          //clear bits Pk5-Pk2
        LCD_DATA = LCD_DATA | x;          //sends high nibble to PORTK
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~RS;         //set RS to command (RS=0)
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //Drop enable to capture command
        MSDelay(15);                       //wait
        x = (command & 0x0F)<< 2;          // shift low nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;         //clear bits Pk5-Pk2
        LCD_DATA =LCD_DATA | x;             //send low nibble to PORTK
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //drop enable to capture command
        MSDelay(15);
  }

         void DATWRT4(unsigned char data)
  {
  unsigned char x;
       
        
        
        x = (data & 0xF0) >> 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | RS;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(5);
       
        x = (data & 0x0F)<< 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(15);
  }

  void PRSTR(unsigned char *message){
        unsigned int index,length;
        length=strlen(message);
        for(index=0;index<length;index++)
          DATWRT4(*(message+index));
  }


 void MSDelay(unsigned int itime)
  {
    unsigned int i; unsigned int j;
    for(i=0;i<itime;i++)
      for(j=0;j<4000;j++);
 }
