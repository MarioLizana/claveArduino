/******************************************************************************/
/**                   Proyecto final curso de Arduino                        **/
/**                                                                          **/
/**       Sistema de alarma con:     Clave en teclado matricial de 4x4       **/
/**                                  Menu en display LCD                     **/ 
/**                                  Detector de movimiento por infrarojos   **/
/**                                                                          **/
/**                                                                          **/
/**       Creador:         MARIO LIZANA CALVO                                **/
/**                                                                          **/
/**       Licencias:     http://www.opensource.org/licenses/mit-license.php  **/
/**                      http://www.tapr.org/OHL                             **/
/******************************************************************************/

/*----------------------------------------------------------------------------*/
/*-                     Librerias usadas y variables                         -*/
/*----------------------------------------------------------------------------*/

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
byte rowPins[ROWS] = {6,7,8,9};                 //Pines de las columnas
byte colPins[COLS] = {10,11,12,13};             //Pines de las filas


int error;
int clave1234[4]={1,2,3,4};
int claveUsuario[4];
int i;
int claveEscrita[4];
char tecla;
int enciende=1;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal lcd(5, 4, 3, 2, 1, 0);


/*----------------------------------------------------------------------------*/
/*-                Funcion que borra la pantalla del LCD                     -*/
/*----------------------------------------------------------------------------*/
void borraPantalla(){
  lcd.setCursor(0, 0);
  lcd.print("                ");                          //16 espacios en blanco.
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);
}

/*----------------------------------------------------------------------------*/
/*-                          Setup del arduino                               -*/
/*----------------------------------------------------------------------------*/
/*En el setup cargo el vector claveUsuario, ademas de configurar la interrupcion 
/*software que comprueba si el detector de movimiento por infrarojos se ha activado*/


void setup(){
  Serial.begin(9600);
  lcd.begin(16, 2);
  //EEPROM.write(0, 0xF);                                 //PARA SIMULAR PRIMERA VEZ, ya que si laposicion uno tiene F nunca se ha encendido.
  if(EEPROM.read(0)==0xF){                                //Si la posicion 0 es F, nunca se ha encendido el PIC. La clave es 1 2 3 4.
    for(i=0; i<4; i++){
      claveUsuario[i]=clave1234[i];
    }
  }
  else{                                                   //Si la posicion 0 es A, se ha cambiado la clave por defecto 1 2 3 4 por otra.
                                                          //Cargo la contraseña de la eeprom.                                                     
    for(i=0; i<4; i++){  
      claveUsuario[i]=EEPROM.read(i+1);
      delay(100);
    }
  }
  
    
                                                          // Inicializo el timer1
  noInterrupts();                                         // desactivo las interrupciones.
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 34286;                                          //Preload timer 65536-16MHz/256/2Hz
  TCCR1B = TCCR1B & 0b11111000 | 0x04;          
  TIMSK1 |= (1 << TOIE1);                                 // habilito la interrupcion por overflow del timer1
  interrupts();                                           // habilito todas las interrupciones.
  
}


ISR(TIMER1_OVF_vect){                                     //La interrupcion asociada al timer1 ejecuta compruebaPIR();
  compruebaPIR();
}




/*----------------------------------------------------------------------------*/
/*-                          compruebaPIR                               -*/
/*----------------------------------------------------------------------------*/
/*La patilla A0 tiene una resistencia que fuerza a baja el voltaje, si se 
/*detecta un ladron el PIR pone u voltaje de 0.3 (V), por lo que 0.3*255 es
/*igual a 76.5. Por seguridad he puesto que se active si existe una entrada
/*analogica de mas de 60*/

void compruebaPIR(){
  if(enciende){
    digitalWrite(A5, HIGH);
    enciende=0;
  }
  else{
    digitalWrite(A5, LOW);
    enciende=1;
  }
  if(analogRead(A0)>60){
    borraPantalla();
    lcd.print("LADRON DETECTADO");
    while(true){
      digitalWrite(A5, HIGH);
      delay(5000);
      digitalWrite(A5, LOW);
      delay(5000);
    }
  }
}




/*----------------------------------------------------------------------------*/
/*-                          Loop del Arduino                                -*/
/*----------------------------------------------------------------------------*/
/*Funcion principal que ejecuta el sistema de alarma, la tecla A se interpreta 
/*como una peticion de cambio de pass*/

void loop(){
  error=0;
  lcd.setCursor(0, 0);
  lcd.print("Pass:           ");
  lcd.setCursor(7, 0);
  
  for(i=0; i<4; i++){                                //Pido el pass, si se pulsa / salgo del bucle.
    tecla=0;
    while (tecla==0){
      tecla = keypad.getKey();
    }
  
    if(tecla=='A'){
      break;
    }
    
    claveEscrita[i]=tecla-48;
    lcd.print(tecla);
    
  }

  if(tecla=='A'){                                     //Pido el pass anterior antes de cambiar la clave.
    lcd.setCursor(0, 0);
    lcd.print("PASS ANTERIOR?:");
    lcd.setCursor(0, 1);
    
    for(i=0; i<4; i++){
      tecla=0;
      while (tecla==0){
        tecla = keypad.getKey();
      }
      
      claveEscrita[i]=tecla-48;
      lcd.print(tecla);
    }
    delay(1000);
    error=0;
    
    for (i=0; i<4; i++){
      if(claveUsuario[i] != claveEscrita[i]){
        error=1;
      }
    }
    
    if(!error){                                     //Si no hay errores en lo pass cambio 
      borraPantalla();
      lcd.print("CLAVE CORRECTA ! ");
      lcd.print(tecla);
      delay(1000);
      
      borraPantalla();
      lcd.print("Nuevo PASS: ");
      EEPROM.write(0, 0xA);                          //Se ha introducido una clave por primera vez.
      lcd.setCursor(0, 1);
      
      for(i=1; i<5; i++){
        tecla=0;
        while (tecla==0){
          tecla = keypad.getKey();
        }
        
        EEPROM.write(i, tecla-48);
        lcd.print(tecla);
      }
      
      delay(1000);
      borraPantalla();
      lcd.print("OK, nueva clave");
      lcd.setCursor(0, 1);
      lcd.print("definida");
      delay(2000);
      borraPantalla();

      for(i=0; i<4; i++){
        claveUsuario[i]=EEPROM.read(i+1);
        delay(100);
      }
    }
    else{                                           //Si hay errores no hago nada digo que la clave es incorrecta.
      borraPantalla();
      lcd.print("CLAVE INCORRECTA: ");
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
      borraPantalla();
      lcd.print("CLAVE CORRECTA ! ");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("DESACTIVANDO...");
      delay(1000);
      noInterrupts();
      borraPantalla();
      lcd.print("DESACTIVADA :)");
      while(true);
    }
    else{
      borraPantalla();
      lcd.print("CLAVE INCORRECTA");
      delay(1000);
    }
  }
}  
  

