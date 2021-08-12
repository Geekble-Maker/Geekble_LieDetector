/*
    Geekble_LieDetector.h - Library for Geekble LieDetector Module.
    Created by SooDragon @ Geekble Circuit Maker, July 20, 2021.
    Special Thanks to C8H10N4O2.
*/
#include "Arduino.h"

// WS2812 data transfer delay time
#define LED_Qtt 12
#define nop1 {asm volatile("nop");}                                             // 62.5nSec
#define nop2 {asm volatile("nop"); asm volatile("nop");}                        // 125nSec
#define nop3 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}   // 187.5nSec
#define nop4 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}          // 250nSec
#define nop5 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}          // 312.5nsec
#define nop6 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}          // 375nSec
#define nop7 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}          // 437.5nSec
#define nop8 {asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}          // 500nSec

#define ResistanceCheck_Repeat 53       // Max 60
#define ResistanceCheck_Interval 991

#define Shock_Voltage_Maximum 50
#define Shock_Voltage_Minimum 20
#define Shock_Repeat_Maximum 50
#define Shock_Intensity 0
#define Shock_Repeat 1
#define Shock_Interval 293
#define Shock_Voltage2ADC_Coefficent 15

#define Normal_Lighting 0
#define Blink_Lighting 1
#define Breath_Lighting 2
#define Swing_Horizontal_Lighting 3
#define Swing_Vertical_Lighting 4
#define Metrix_Lighting 5

#define Function_Music 1
#define Function_Lights 2
#define Function_R_Check 3
#define Function_Shock 4

#define Calc_mVf 330        // 330mV
#define Calc_Coefficent 4.8828   // 5000/1024 = 4.8828125

#ifndef Geekble_LieDetector_h
    #define Geekble_LieDetector_h
    
    class Geekble_LieDetector
    {
        public:
        Geekble_LieDetector();          // Construct Class, Initialize function.
        uint8_t Geekble_LieDetector::Read_SW();
        void Geekble_LieDetector::attach(uint8_t _V_Check, uint8_t _Shock, uint8_t _R_Test, uint8_t _R_Check, uint8_t _Buzzer, uint8_t _SW_IO, uint8_t _SW_GND);
        void Geekble_LieDetector::RunMusic(uint8_t Time_sec, uint16_t Notes[][2]);
        void Geekble_LieDetector::RunLights(uint8_t Time_sec, uint8_t Lights[]);
        void Geekble_LieDetector::RunShocks(uint8_t Time_sec, uint8_t Shocks[]);
        uint16_t Geekble_LieDetector::RunResistanceCheck(uint8_t Time_sec);
        uint16_t Geekble_LieDetector::GetResistance(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[]);
        void Geekble_LieDetector::ReturnResult_Truth(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[]);
        void Geekble_LieDetector::ReturnResult_Lier(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[]);

        private:
        void Geekble_LieDetector::byte_out(uint8_t _byte);
        void Geekble_LieDetector::bytes_out(uint8_t _byte0G, uint8_t _byte0R, uint8_t _byte0B, uint8_t _byte1G, uint8_t _byte1R, uint8_t _byte1B, uint8_t _byte2G, uint8_t _byte2R, uint8_t _byte2B, uint8_t _byte3G, uint8_t _byte3R, uint8_t _byte3B, uint8_t _byte4G, uint8_t _byte4R, uint8_t _byte4B, uint8_t _byte5G, uint8_t _byte5R, uint8_t _byte5B, uint8_t _byte6G, uint8_t _byte6R, uint8_t _byte6B, uint8_t _byte7G, uint8_t _byte7R, uint8_t _byte7B, uint8_t _byte8G, uint8_t _byte8R, uint8_t _byte8B, uint8_t _byte9G, uint8_t _byte9R, uint8_t _byte9B, uint8_t _byte10G, uint8_t _byte10R, uint8_t _byte10B, uint8_t _byte11G, uint8_t _byte11R, uint8_t _byte11B);
        void Geekble_LieDetector::Shock(uint8_t ShockVoltage);
        uint16_t Geekble_LieDetector::ResistanceCheck_ADC2Kohm(uint16_t ADC_Value);
        uint16_t Geekble_LieDetector::ServiceEngine(uint8_t FunctionSelect, uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[]);
        void Geekble_LieDetector::Program_Lighting (uint8_t GRB_R, uint8_t GRB_G, uint8_t GRB_B, uint8_t Lighting_Mode, uint32_t Time_Spent, uint32_t *Update_Time_Lighting);
    };

#endif

#define NOTE_END NULL
#define NOTE_REST 1
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978