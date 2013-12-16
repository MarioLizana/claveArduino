#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "Keypad.h"

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {6,7,8,9}; //connect to row pinouts 
byte colPins[COLS] = {10,11,12,13}; //connect to column pinouts

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



int error;
int clave1234[4]={1,2,3,4};
int claveUsuario[4];
int i;
int claveEscrita[4];
char tecla;


LiquidCrystal lcd(5, 4, 3, 2, 1, 0);

void despierta(void) {
}

void setup(){
   Serial.begin(9600);
   delay(3000);
  lcd.begin(16, 2);
   //  EEPROM.write(0, 0xF);      //pARA SIMULAR PRIMERA VEZ
   if(EEPROM.read(0)==0xF){                              //Si la posicion 0 es F, nunca se ha encendido el PIC. La clave es 1 2 3 4.
      for(i=0; i<4; i++){
         claveUsuario[i]=clave1234[i];
         //printf(lcd_putc,"%d ", claveUsuario[i]);
      }
   }
   else{                                                  //Si la posicion 0 es A, se ha cambiado la clave por defecto 1 2 3 4 por otra.
                                                          //Cargo la contraseña de la eeprom.                                                     
      for(i=0; i<4; i++){  
         claveUsuario[i]=EEPROM.read(i+1);
         delay(100);
         
      }
   }

  
}


void loop(){
  //lcd_send_byte(0, 1); //Borra LCD
  error=0;
  lcd.print("Pass: ");
  
  for(i=0; i<4; i++){                                //Pido el pass, si se pulsa / salgo del bucle.
    tecla=0;
    while (tecla==0){
      tecla = keypad.getKey();
    }
  
    if(tecla==47){
      break;
    }
    
    claveEscrita[i]=tecla-48;
    lcd.print(tecla);
    
  }

  if(tecla==47){                                     //Pido el pass anterior antes de cambiar la clave.
    lcd.setCursor(0, 0);
    lcd.print("PASS ATERIOR?:\n");
    for(i=0; i<4; i++){
      tecla=0;
      
      while (tecla==0){
        tecla = keypad.getKey();
      }
      
      claveEscrita[i]=tecla-48;
      lcd.print('tecla');
    }
    error=0;
    
    for (i=0; i<4; i++){
      if(claveUsuario[i] != claveEscrita[i]){
        error=1;
      }
    }
    if(!error){                                     //Si no hay errores en lo pass cambio 
    //lcd_send_byte(0, 1); //Borra LCD
    
    lcd.print("PASS ATERIOR?:\n");
    lcd.print('tecla');
    delay(1000);
    //lcd_send_byte(0, 1); //Borra LCD
    lcd.print("Nuevo PASS: \n");
    EEPROM.write(0, 0xA);                          //Se ha introducido una clave por primera vez.
    for(i=1; i<5; i++){
      tecla=0;
      while (tecla==0){
        tecla = keypad.getKey();
      }
      EEPROM.write(i, tecla-48);
      lcd.print('tecla');
    }
    for(i=0; i<4; i++){
      claveUsuario[i]=EEPROM.read(i+1);
      delay(100);
      //printf(lcd_putc,"%d ", claveUsuario[i]);
      }
    }
    else{                                           //Si hay errores no hago nada digo que la clave es incorrecta.
      //lcd_send_byte(0, 1); //Borra LCD
      lcd.print("CLAVE INCORRECTA: \n");
      delay(1000);
    }
  }
  else{                                              //Compruebo la clave, muestro en la LCD si lo es o no.
    error=0;
    for (i=0; i<4; i++){
      if(claveUsuario[i] != claveEscrita[i]){
        error=1;
      }
    }
    if(!error){
      lcd.setCursor(0, 1);
      lcd.print("CLAVE CORRECTA      ");
      //digitalWrite(13, HIGH);
      delay(1000);
      //digitalWrite(13, LOW);
      //lcd_send_byte(0, 1); //Borra LCD
      lcd.print("zZzZzZzZZZ");
      //sleep();
    }
    else{
      lcd.setCursor(0, 1);
      lcd.print("CLAVE INCORRECTA");
      delay(1000);
    }
  }
}  
  




