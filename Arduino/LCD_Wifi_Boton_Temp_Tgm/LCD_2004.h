#ifndef LCD_2004_h
#define LCD_2004_h

#include "Arduino.h"

#define LCD_Data D7
#define LCD_Clk D5
#define LCD_Ena D3
#define LCD_RW D2
#define LCD_RS D1

// the #include statement and code go here...
void LCD2004_Init();

  //Start LCD initialization
  //LCD2004_Test(hspi1);
  //Function Set
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
void LCD2004_FunctionSet();

  //Clear Display
  //0x01
void LCD2004_ClearScreen();

  //Return Home
  //0x02
void LCD2004_ReturnHome();

  //DisplayOnOffControl
  //0 0 0 0 1 D C B :: D=1=Display ON; C=1=CursorON ; B=1=CursorBlink ::0x0F
void LCD2004_DisplayOnOffControl();

/*
 *rPos needs to be within below range
 *0x00 -> Line1/Row1 :: 0x4F -> Line4/Row20
 */
void LCD2004_SetCursor(uint8_t rPos);

void LCD2004_WriteChar(uint8_t sChar);

void LCD2004_WriteString(uint8_t * sString);

void LCD2004_WriteTittle(uint8_t * sString);

void LCD2004_WriteMainLabel(uint8_t * sString);

void LCD2004_WriteSubLabel(uint8_t * sString);

void LCD2004_WriteMessage(uint8_t * sString);

void LCD2004_CleanLine(uint8_t rPos);

void LCD2004_WelcomeScreen();
#endif
