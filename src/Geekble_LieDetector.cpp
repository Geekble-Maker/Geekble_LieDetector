/*
    Geekble_LieDetector.h - Library for Geekble LieDetector Module.
    Created by SooDragon @ Geekble Circuit Maker, July 20, 2021.
    Special Thanks to C8H10N4O2.
*/
#include <Geekble_LieDetector.h>
#include "Arduino.h"

uint8_t Pin_V_Check;
uint8_t Pin_Shock;
uint8_t Pin_R_Test;
uint8_t Pin_R_Check;
uint8_t Pin_Buzzer;
uint8_t Pin_SW_IO;
uint8_t Pin_SW_GND;

uint16_t Calc_mVin = 0;

Geekble_LieDetector::Geekble_LieDetector()
{
  ;
}

void Geekble_LieDetector::attach(uint8_t _V_Check, uint8_t _Shock, uint8_t _R_Test, uint8_t _R_Check, uint8_t _Buzzer, uint8_t _SW_IO, uint8_t _SW_GND)
{
  Serial.begin(9600);
  Serial.println("Welcome to Geekble Lie Detector System!");
  Serial.println("Do Not Touch Electrode Until System Setup is Completed.");
  Serial.println("Performing System Setup...");
  System_Setup_Start_Point:
  
  Pin_V_Check = _V_Check;
  Pin_Shock =  _Shock;
  Pin_R_Test = _R_Test;
  Pin_R_Check = _R_Check;
  Pin_Buzzer = _Buzzer;
  Pin_SW_IO = _SW_IO;
  Pin_SW_GND = _SW_GND;

  /*  Setup for LEDs  */
  DDRB |= (1 << PINB3);   // DIN pin for WS2812 (D11)

  
  /*  Setup for Shock */
  pinMode(Pin_Shock, OUTPUT);
  digitalWrite(Pin_Shock, LOW);
  pinMode(Pin_V_Check, INPUT);

  DDRB |= (1 << PINB2);   // PWM pin (D10), Occupies Timer Counter 1(OC1B).
  PORTB &= ~(1 << PINB2); // Default LOW status
    // Timer Counter 0 is reserved for function "millis"
    // Timer Counter 2 is reserved for function "tone"
    
    /*	Timer Counter Setting	*/
		  /*
		  TCCR1A - Timer/Counter Control Register A
			  COM1A - Compare Match Output A Mode (OC1A Pin behavior)
			  COM1B - Compare Match Output B Mode (OC1B Pin behavior)
			  WGM	- Waveform Generation Mode
		  */
  TCCR1A = 0;             // Clear Register
	TCCR1A |= (1 << WGM11);	// Mode 14: Fast PWM, TOP on ICR1, Update on Bottom
		/*
		TCCR0B - Timer/Counter Control Register B
			FOC	- Force Output Compare - Only for Non-PWM Mode.
			WGM	- Waveform Generation Mode
			CS	- Clock Select ( 0:T/C Stop, 1:clk/1, 2:clk/8, 3:clk/64, 4:clk/256, 5:clk/1024, 6:exclk,falling edge, 7:exclk,Rising edge )
		*/
  TCCR1B = 0;             // Clear Register
	TCCR1B |= ((1 << WGM13) | (1 << WGM12));	// clk/8, 500nSec/clock
		/*
		TIMSK0 - Timer/Counter Interrupt Mask Register
			OCIE - Timer/Counter Output Compare Match A,B Interrupt Enable
			TOIE - Timer/Counter Overflow Interrupt Enable
		*/

  TCNT1 = 0;
  TIMSK1 = 0;               // Clear Register
	
    /*  30KHz, 85% Duty
        a Cycle: 33.333uSec, Positive: 4.999uSec
        a Clock: 62.5nSec
        a Cycle: 533 clock, Positive: 80 clock
     */
  ICR1 = 1500;
	OCR1B = 1050;

  /*  SetUp for SW  */
  pinMode(Pin_SW_GND, OUTPUT);
  digitalWrite(Pin_SW_GND, LOW);
  pinMode(Pin_SW_IO, INPUT_PULLUP);

  /*  Setup for Buzzer  */
  pinMode(Pin_Buzzer, OUTPUT);
  digitalWrite(Pin_Buzzer, LOW);

  /*  Setup for Resistance Read   */
  pinMode(Pin_R_Test, OUTPUT);
  digitalWrite (Pin_R_Test, HIGH);
  for(uint8_t temp = 0; temp < ResistanceCheck_Repeat; temp ++)
  {
    delay(1);
    uint16_t Temp_R_Check_ADC = analogRead(Pin_R_Check);
    if (900 < Temp_R_Check_ADC)
    {
      Calc_mVin += analogRead(Pin_R_Check);
    }
    else
    {
      Serial.println("ERROR!");
      delay(600);
      Serial.println("Do Not Touch Electrode Until System Setup is Completed.");
      delay(600);
      Serial.println("Retry System Setup...");
      delay(1200);
      goto System_Setup_Start_Point;
    }
  }
  Calc_mVin = (Calc_mVin * Calc_Coefficent / ResistanceCheck_Repeat);
  Serial.print("Open Status Vin: ");
  Serial.print(Calc_mVin);
  Serial.println("mV");
  digitalWrite (Pin_R_Test, LOW);

	/*  Setup Complete  */
  Serial.println("System Setup Complete!");
}

void Geekble_LieDetector::byte_out(uint8_t _byte) 
{
  /* 
   * WS2812B-Mini-V3 Timing
   * T0H = 220ns ~ 380ns
   * T0L = 580ns ~ 1us
   * T1H = 580ns ~ 1us
   * T1L = 580ns ~ 1us
   * RES = 280us ~
   * 
   * nop1 = 62.5ns, nop2 = 125ns, nop3 = 187.5ns, nop4 = 250ns, nop5 = 312.5ns, nop6 = 375ns, nop7 = 437.5nsec, nop8 = 500nsec, nop9 = 562.5nSec, nop10 = 625nSec
   * 220ns ~ 380ns -> 3.52clk ~ 6.08clk
   * for loop escape takes 800us
   */

  for (uint8_t i = 0; i < 8; i++) 
  {
    if ((0 < i) && (i < 7))
    {
      nop2;
    }
    if (_byte & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop5;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      nop1;
      PORTB &= ~(1 << PINB3);
    }
    _byte <<= 1;
  }
}

void Geekble_LieDetector::bytes_out(uint8_t _byte0G, uint8_t _byte0R, uint8_t _byte0B, uint8_t _byte1G, uint8_t _byte1R, uint8_t _byte1B, uint8_t _byte2G, uint8_t _byte2R, uint8_t _byte2B, uint8_t _byte3G, uint8_t _byte3R, uint8_t _byte3B, uint8_t _byte4G, uint8_t _byte4R, uint8_t _byte4B, uint8_t _byte5G, uint8_t _byte5R, uint8_t _byte5B, uint8_t _byte6G, uint8_t _byte6R, uint8_t _byte6B, uint8_t _byte7G, uint8_t _byte7R, uint8_t _byte7B, uint8_t _byte8G, uint8_t _byte8R, uint8_t _byte8B, uint8_t _byte9G, uint8_t _byte9R, uint8_t _byte9B, uint8_t _byte10G, uint8_t _byte10R, uint8_t _byte10B, uint8_t _byte11G, uint8_t _byte11R, uint8_t _byte11B)
{
  asm("cli");
  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte0G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte0G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte0R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte0R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte0B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte0B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte1G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte1G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte1R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte1R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte1B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte1B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte2G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte2G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte2R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte2R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte2B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte2B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte3G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte3G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte3R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte3R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte3B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte3B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte4G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte4G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte4R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte4R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte4B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte4B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte5G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte5G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte5R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte5R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte5B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte5B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte6G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte6G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte6R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte6R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte6B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte6B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte7G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte7G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte7R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte7R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte7B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte7B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte8G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte8G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte8R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte8R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte8B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte8B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte9G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte9G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte9R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte9R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte9B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte9B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte10G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte10G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte10R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte10R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte10B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte10B <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte11G & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte11G <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte11R & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte11R <<= 1;
  }

  for (uint8_t i = 0; i < 8; i++) 
  {
    if (_byte11B & 0x80)     // High
    {
      PORTB |= (1 << PINB3);
      nop6;
      PORTB &= ~(1 << PINB3);
    }
    else                  // Low
    {
      PORTB |= (1 << PINB3);
      // nop1;
      PORTB &= ~(1 << PINB3);
    }
    nop5;
    _byte11B <<= 1;
  }
  asm("sei");
}

void Geekble_LieDetector::Shock(uint8_t ShockVoltage)
{
  if (Shock_Voltage_Maximum < ShockVoltage)
  {
    ShockVoltage = Shock_Voltage_Maximum;
  }
  else if (ShockVoltage < Shock_Voltage_Minimum)
  {
    ShockVoltage = Shock_Voltage_Minimum;
  }

  uint8_t Weighted_ShockVoltage = (ShockVoltage - ((Shock_Voltage_Maximum - ShockVoltage) >> 2));

  uint16_t Shock_ADC = (Weighted_ShockVoltage * Shock_Voltage2ADC_Coefficent);
  TCNT1 = 0;
  TCCR1A |= (3 << COM1B0);  // Compare Output Mode, Fast PWM, 3: Set on Compare Match, Clear at Bottom
  TCCR1B |= (1 << CS10);    // Run PWM
  
  while (analogRead(Pin_V_Check) <= Shock_ADC)
  {
    // Wait Boost
  }

  TCCR1B &= ~(1 << CS10);
  TCCR1A &= ~(3 << COM1B0);
  PORTB &= ~(1 << PINB2);

  /*  Shock */
  digitalWrite(Pin_Shock, HIGH);
  delay(1);
  digitalWrite(Pin_Shock, LOW);
}

uint8_t Geekble_LieDetector::Read_SW()
{
  uint32_t Time_Start = millis();
  while(digitalRead(Pin_SW_IO) == 0)
  {
    if ((Time_Start + 800) < millis())
    {
      return 2;
    }
  }

  if (millis() < (Time_Start + 50))
  {
    return 0;
  }
  else 
  {
    return 1;
  }
}

uint16_t Geekble_LieDetector::ResistanceCheck_ADC2Kohm(uint16_t ADC_Value)
{
  
  /*
    Resistance Calculation
    - (((mVs*ADC/1024) - mVf) * 1000) / (mVin - (mVs*ADC/1024)) = KR_Hand
      - mVs = Voltage Source, milli volt (5000mV)
      - mVin = Voltage from R-Test, milli volt (5000mV)
      - mVf = Diode's Forward Voltage, milli volt (330mV)
      - ADC = ADC Value
      - KR_Hand = Hand's Impedance, Killo Ohm.
    Clac_Coefficent = ADC * mVs / 1024 / Sampling = 4.8828
   */
  uint32_t Calc_R_Test_Pin_mVoltage = (ADC_Value * Calc_Coefficent / ResistanceCheck_Repeat);
  if ((Calc_mVin - 10) < Calc_R_Test_Pin_mVoltage)
  {
    /* Open Status  */
    return 0;
  }
  
  //return (1000 * ((ADC_Value * Calc_Coefficent / ResistanceCheck_Repeat) - Calc_mVf) / (Calc_mVin - (ADC_Value * Calc_Coefficent / ResistanceCheck_Repeat)));
  return (1000 * (Calc_R_Test_Pin_mVoltage - Calc_mVf) / (Calc_mVin - Calc_R_Test_Pin_mVoltage));
}

uint16_t Geekble_LieDetector::ServiceEngine(uint8_t FunctionSelect, uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[])
{
  uint32_t Time_Start = millis();
  uint32_t Time_End = (Time_Start + (Time_sec * 1000));

  uint16_t Resistance_ADC = 0;
  uint16_t ResistanceCheck_Count = 0;
  uint32_t Update_Time_ResistanceCheck = 0;
  uint32_t Resistance_Average = 0;
  uint16_t Temp_Hand_R ;

  uint8_t Length_Notes = 0;
  for(Length_Notes; ; Length_Notes ++)
  {
    if (Notes[Length_Notes][0] == NULL)
    {
      break;
    }
  }
  uint8_t Sequence_Notes = 0;
  uint32_t Update_Time_Notes = 0;

  uint32_t Update_Time_Lighting = 0;

  uint32_t Update_Time_Shock = 0;
  uint8_t Shock_Count = 0;
  uint8_t _Shock_Repeat = Shocks[Shock_Repeat];
  if (Shock_Repeat_Maximum < _Shock_Repeat)
  {
    _Shock_Repeat = Shock_Repeat_Maximum;
  }
  

  while (millis() < Time_End)
  {
    uint32_t Time_Spent = (millis() - Time_Start);

    /*  Music Sequence   */
    if (((FunctionSelect & (1 << Function_Music)) != 0) && (Update_Time_Notes <= Time_Spent))
    {
      if (Notes[Sequence_Notes][0] != NOTE_REST)
      {
        tone(Pin_Buzzer, Notes[Sequence_Notes][0]);
      }
      else
      {
        noTone(Pin_Buzzer);
      }
      Update_Time_Notes += Notes[Sequence_Notes][1];
      Sequence_Notes ++;
      if (Length_Notes <= Sequence_Notes)
      {
        Sequence_Notes = 0;
      }
    }

    /*  Lighting Sequence   */
    if (((FunctionSelect & (1 << Function_Lights)) != 0) && (Update_Time_Lighting <= Time_Spent))
    {
      Program_Lighting(Lights[0], Lights[1], Lights[2], Lights[3], Time_Spent, &Update_Time_Lighting);
    }

    /*  Resistance Check Sequence */
    if (((FunctionSelect & (1 << Function_R_Check)) != 0) && (Update_Time_ResistanceCheck <= Time_Spent))
    {
      digitalWrite(Pin_R_Test, HIGH);
      delay(1);
      ResistanceCheck_Count ++;
      Resistance_ADC += analogRead(Pin_R_Check);

      if ((ResistanceCheck_Count % ResistanceCheck_Repeat) == 0)
      {
        Update_Time_ResistanceCheck += ResistanceCheck_Interval;
        Temp_Hand_R = ResistanceCheck_ADC2Kohm(Resistance_ADC);
        if (Temp_Hand_R != 0)
        {
          Serial.print("Current Hand Resistance : ");
          Serial.print(Temp_Hand_R);
          Serial.println("KR");
          Resistance_Average += ResistanceCheck_ADC2Kohm(Resistance_ADC);
          Resistance_ADC = 0;
        }
        else
        {
          Serial.println("Hand Detached!");
          Resistance_Average = 0;
          Time_End = 0;
        }
      }
      
      digitalWrite(Pin_R_Test, LOW);
    }
    

    /*  Shock Sequence    */
    if (((FunctionSelect & (1 << Function_Shock)) != 0) && (Update_Time_Shock <= Time_Spent))
    {
      Shock(Shocks[Shock_Intensity]);
      Shock_Count ++;
      
      if (_Shock_Repeat <= Shock_Count)
      {
        Update_Time_Shock += Shock_Interval;
        Shock_Count = 0;
      }
    }
  }
  
  noTone(Pin_Buzzer);
  return (Resistance_Average / (ResistanceCheck_Count / ResistanceCheck_Repeat));
}

void Geekble_LieDetector::Program_Lighting(uint8_t GRB_R, uint8_t GRB_G, uint8_t GRB_B, uint8_t Lighting_Mode, uint32_t Time_Spent, uint32_t *Update_Time_Lighting)
{
  switch (Lighting_Mode)
  {
    case Normal_Lighting:
    {
      bytes_out(GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B);
      *Update_Time_Lighting += 9897857;
      break;
    }

    case Blink_Lighting:
    {
      uint8_t UpdateInterval = 197;
      if (((Time_Spent / UpdateInterval) % 2) == 0)
      {
        bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      }
      else
      {
        bytes_out(GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B);
      }

      *Update_Time_Lighting += UpdateInterval;
      break;
    }

    case Breath_Lighting:
    {
      uint8_t UpdateInterval = 47;
      uint16_t Sequence_Lights = (Time_Spent / UpdateInterval);
      
      if ((Sequence_Lights % 40) < 20)
      {
        uint8_t Program_GRB_G = GRB_G;
        uint8_t Program_GRB_R = GRB_R;
        uint8_t Program_GRB_B = GRB_B;

        for (uint8_t Calc_Temp = 0; Calc_Temp < ((Sequence_Lights % 40) + 1); Calc_Temp++)
        {
          Program_GRB_G = (Program_GRB_G * 0.92);
          Program_GRB_R = (Program_GRB_R * 0.92);
          Program_GRB_B = (Program_GRB_B * 0.92);
        }
        bytes_out(Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B);
      }
      else
      {      
        uint8_t Program_GRB_G = (GRB_G * 0.1887);
        uint8_t Program_GRB_R = (GRB_R * 0.1887);
        uint8_t Program_GRB_B = (GRB_B * 0.1887);

        for (uint8_t Calc_Temp = 0; Calc_Temp < (Sequence_Lights % 20); Calc_Temp++)
        {
          Program_GRB_G = (Program_GRB_G * 100 / 92);
          Program_GRB_R = (Program_GRB_R * 100 / 92);
          Program_GRB_B = (Program_GRB_B * 100 / 92);
        }
        bytes_out(Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B, Program_GRB_G, Program_GRB_R, Program_GRB_B);
      }

      *Update_Time_Lighting += UpdateInterval;
      break;
    }
  
    case Swing_Horizontal_Lighting:
    {
      uint8_t UpdateInterval = 149;    
      // 140 bpm -> 430msec for 1 cycle
      // row2, row3, row4, row3, row2, row1 -> 6 step
      // 73 for each step
      uint16_t Sequence_Lights = ((Time_Spent / UpdateInterval) % 6);

      switch (Sequence_Lights)
      {
        case 0:
          bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          break;
        case 1:
          bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          break;
        case 2:
          bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B);
          break;
        case 3:
          bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          break;
        case 4:
          bytes_out(0, 0, 0, 0, 0, 0, 0, 0, 0, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          break;
        case 5:
          bytes_out(GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, GRB_G, GRB_R, GRB_B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          break;
        default:
          break;
      }

      *Update_Time_Lighting += UpdateInterval;
      break;
    }

    case Swing_Vertical_Lighting:
    {
      uint8_t UpdateInterval = 149;    
      // 140 bpm -> 430msec for 1 cycle
      // row2, row3, row4, row3, row2, row1 -> 6 step
      // 73 for each step
      uint16_t Sequence_Lights = ((Time_Spent / UpdateInterval) % 4);

      switch (Sequence_Lights)
      {
        case 0:
          bytes_out(0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0);
          break;
        case 1:
          bytes_out(0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0);
          break;
        case 2:
          bytes_out(0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, 0,0,0);
          break;
        case 3:
          bytes_out(GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B, GRB_G,GRB_R,GRB_B, 0,0,0, 0,0,0, 0,0,0, 0,0,0, GRB_G,GRB_R,GRB_B);
          break;
        default:
          break;
      }

      *Update_Time_Lighting += UpdateInterval;
      break;
    }

    case Metrix_Lighting:
    {
      uint8_t UpdateInterval = 379;
      uint8_t GRB_G0 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R0 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B0 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G1 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R1 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B1 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G2 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R2 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B2 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G3 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R3 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B3 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G4 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R4 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B4 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G5 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R5 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B5 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G6 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R6 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B6 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G7 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R7 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B7 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G8 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R8 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B8 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G9 = GRB_G / (1.1 * random(1, 10));   uint8_t GRB_R9 = GRB_R / (1.1 * random(1, 10));   uint8_t GRB_B9 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G10 = GRB_G / (1.1 * random(1, 10));  uint8_t GRB_R10 = GRB_R / (1.1 * random(1, 10));  uint8_t GRB_B10 = GRB_B / (1.1 * random(1, 10));
      uint8_t GRB_G11 = GRB_G / (1.1 * random(1, 10));  uint8_t GRB_R11 = GRB_R / (1.1 * random(1, 10));  uint8_t GRB_B11 = GRB_B / (1.1 * random(1, 10));
      bytes_out(GRB_G0, GRB_R0, GRB_B0, GRB_G1, GRB_R1, GRB_B1, GRB_G2, GRB_R2, GRB_B2, GRB_G3, GRB_R3, GRB_B3, GRB_G4, GRB_R4, GRB_B4, GRB_G5, GRB_R5, GRB_B5, GRB_G6, GRB_R6, GRB_B6, GRB_G7, GRB_R7, GRB_B7, GRB_G8, GRB_R8, GRB_B8, GRB_G9, GRB_R9, GRB_B9, GRB_G10, GRB_R10, GRB_B10, GRB_G11, GRB_R11, GRB_B11);
    
      *Update_Time_Lighting += UpdateInterval;
      break;
    }

    default:
    {
      *Update_Time_Lighting += 9897857;
      break;
    }
  }
}

void Geekble_LieDetector::RunMusic(uint8_t Time_sec, uint16_t Notes[][2])
{
  Serial.println("*** Run Music ***");
  uint8_t FunctionSelect = (1 << Function_Music);
  ServiceEngine(FunctionSelect, Time_sec, Notes, NULL, NULL);
}

void Geekble_LieDetector::RunLights(uint8_t Time_sec, uint8_t Lights[])
{
  Serial.println("*** Run Lights ***");
  uint8_t FunctionSelect = (1 << Function_Lights);
  ServiceEngine(FunctionSelect, Time_sec, NULL, Lights, NULL);
}

void Geekble_LieDetector::RunShocks(uint8_t Time_sec, uint8_t Shocks[])
{
  Serial.println("*** Run Shocks ***");
  uint8_t FunctionSelect = (1 << Function_Shock);
  ServiceEngine(FunctionSelect, Time_sec, NULL, NULL, Shocks);
}

uint16_t Geekble_LieDetector::RunResistanceCheck(uint8_t Time_sec)
{
  Serial.println("*** Run Resistance Check ***");
  uint8_t FunctionSelect = (1 << Function_R_Check);
  return ServiceEngine(FunctionSelect, Time_sec, NULL, NULL, NULL);
}

uint16_t Geekble_LieDetector::GetResistance(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[])
{
  Serial.println("*** Get Resistance ***");
  uint8_t FunctionSelect = ((1 << Function_Music) | (1 << Function_Lights) | (1 << Function_R_Check));
  return ServiceEngine(FunctionSelect, Time_sec, Notes, Lights, NULL);
}

void Geekble_LieDetector::ReturnResult_Truth(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[])
{
  Serial.println("*** Return Result - Truth ***");
  uint8_t FunctionSelect = ((1 << Function_Music) | (1 << Function_Lights));
  ServiceEngine(FunctionSelect, Time_sec, Notes, Lights, NULL);
}

void Geekble_LieDetector::ReturnResult_Lier(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[])
{
  Serial.println("*** Return Result - Lier ***");
  uint8_t FunctionSelect = ((1 << Function_Music) | (1 << Function_Lights) | (1 << Function_Shock));
  ServiceEngine(FunctionSelect, Time_sec, Notes, Lights, Shocks);
}