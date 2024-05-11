#include <SPI.h>
#include "Arduino.h"
#include "LCD_2004.h"


void LCD2004_Init(){
  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);  
  //Set RS to Low (Reset) -> Data to the display is considered Instruction
  digitalWrite(LCD_RS,LOW);
  //Set RW to Low (Reset)--> Write data to LCD
  digitalWrite(LCD_RW,LOW);

  //Start LCD initialization
  //LCD2004_Test(hspi1);
  //Function Set
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
  LCD2004_FunctionSet();

  //Clear Display
  //0x01
  LCD2004_ClearScreen();

  //Return Home
  //0x02
  LCD2004_ReturnHome();

  //DisplayOnOffControl
  //0 0 0 0 1 D C B :: D=1=Display ON; C=1=CursorON ; B=1=CursorBlink ::0x0F
  LCD2004_DisplayOnOffControl();
}

void LCD2004_FunctionSet(){
  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);

  //Set RS to Low (Reset) -> Data to the display is considered Instruction
  digitalWrite(LCD_RS,LOW);

  //FunctionSet
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
  SPI.transfer(0x38);

  digitalWrite(LCD_Ena,LOW);

  delay(1);
}

void LCD2004_ClearScreen(){
  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);

  //Set RS to Low (Reset) -> Data to the display is considered Instruction
  digitalWrite(LCD_RS,LOW);

  //FunctionSet
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
  SPI.transfer(0x01);

  digitalWrite(LCD_Ena,LOW);

  delay(1);
}

void LCD2004_ReturnHome(){
  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);

  //Set RS to Low (Reset) -> Data to the display is considered Instruction
  digitalWrite(LCD_RS,LOW);

  //FunctionSet
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
  SPI.transfer(0x02);

  digitalWrite(LCD_Ena,LOW);

  delay(1);
}


void LCD2004_DisplayOnOffControl(){
  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);

  //Set RS to Low (Reset) -> Data to the display is considered Instruction
  digitalWrite(LCD_RS,LOW);

  //FunctionSet
  //0 0 1 DL N F X X :: DL=1= 8bit; N=1=2LineDisplay; F=0=5x8dots ::0x38
  //SPI.transfer(0x0F);
  SPI.transfer(0x0C);

  digitalWrite(LCD_Ena,LOW);

  delay(1);
}

/*
 *rPos needs to be within below range
 *0x00 -> Line1/Row1 :: 0x4F -> Line4/Row20
 */
void LCD2004_SetCursor(uint8_t rPos){

  if (rPos>=0 && rPos <=79){
    //Set Enable to High (Set) --> Data is latched at Enable falling edge
    digitalWrite(LCD_Ena,HIGH);
    //Set RS to Low (Reset) -> Data to the display is considered Instruction
    digitalWrite(LCD_RS,LOW);
    //Set RW to Low (Reset)--> Write data to LCD
    digitalWrite(LCD_RW,LOW);

    //Return Home
    SPI.transfer(0x02);

    digitalWrite(LCD_Ena,LOW);
    delay(1);

    for (uint8_t i = 0; i<rPos; i++){
      digitalWrite(LCD_Ena,HIGH);
      //0 0 0 1 S/N R/L X X :: S/C=DisplayShift/CursorMove=0=CursorMove; R/L=ShiftToRight/ShiftToLeft=1 ::0x14
      SPI.transfer(0x14);

      digitalWrite(LCD_Ena,LOW);
      delay(1);      
    }
  }
}

void LCD2004_WriteChar(uint8_t sChar){

  //Set Enable to High (Set) --> Data is latched at Enable falling edge
  digitalWrite(LCD_Ena,HIGH);

  ///Set RS to High (Set) -> Data to the display is considered Data
  digitalWrite(LCD_RS,HIGH);

  SPI.transfer(sChar);
  
  digitalWrite(LCD_Ena,LOW);
  delay(1);
}

void LCD2004_WriteString(uint8_t * sString){
  int i=0;
  while(sString[i]!='\0'){
    LCD2004_WriteChar(sString[i]);
    i++;
  }
}

void LCD2004_WriteTittle(uint8_t * sString){
  uint8_t rPos = 0; //Primera línea
  LCD2004_CleanLine(rPos);
  LCD2004_SetCursor(rPos);
  LCD2004_WriteString(sString);

  //Limpia las líneas inferiores
  LCD2004_CleanLine(40);
  LCD2004_CleanLine(20);
  LCD2004_CleanLine(60);
}

void LCD2004_WriteMainLabel(uint8_t * sString){
  uint8_t rPos = 40; //Segunda línea
  LCD2004_CleanLine(rPos);
  LCD2004_SetCursor(rPos);
  LCD2004_WriteString(sString);

  //Limpia las líneas inferiores
  LCD2004_CleanLine(20);
  LCD2004_CleanLine(60);
}

void LCD2004_WriteSubLabel(uint8_t * sString){
  uint8_t rPos = 20; //Tercera línea
  LCD2004_CleanLine(rPos);
  LCD2004_SetCursor(rPos);
  LCD2004_WriteString(sString);

  //Limpia las líneas inferiores
  //LCD2004_CleanLine(60);
}

void LCD2004_WriteMessage(uint8_t * sString){
  uint8_t rPos = 60; //Cuarta línea
  LCD2004_CleanLine(rPos);
  LCD2004_SetCursor(rPos);
  LCD2004_WriteString(sString);
}

void LCD2004_CleanLine(uint8_t rPos){
  char sLine[] = "                    ";
  LCD2004_SetCursor(rPos);
  LCD2004_WriteString((uint8_t *)&sLine);

}

void LCD2004_WelcomeScreen(){
  char sTittle[] = "SISTEMA RIEGO WIFI  ";
  char sMainLabel[] = "                    ";
  char sSubLabel[] = "FRANK RAMIREZ       ";
  char sMessage[] = "2023 - V01          ";
  LCD2004_WriteTittle((uint8_t *)&sTittle); 
  LCD2004_WriteMainLabel((uint8_t *)&sMainLabel);
  LCD2004_WriteSubLabel((uint8_t *)&sSubLabel);
  LCD2004_WriteMessage((uint8_t *)&sMessage);
  
  //LCD2004_ReturnHome();
}
