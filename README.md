# Welcome to Geekble_LieDetector<br>
<br>
Geekble_Oscillator generates pulse from 8MHz to 31.25KHz with fixed 50% duty ratio.<br>
<br>
This Library occupies Timer Counter 1, which will affect the functions or library depend on Timer Counter 1.<br>
<br>
<br>


***
## Instruction<br>
<br>
 
        Geekble_LieDetector();          			// Construct Class, Initialize function.
        uint8_t Geekble_LieDetector::Read_SW();		// Read and return SW Status, 0 for idle or Noise, 1 for Short Press, 2 for Long Press 
        void attach(uint8_t _V_Check, uint8_t _Shock, uint8_t _R_Test, uint8_t _R_Check, uint8_t _Buzzer, uint8_t _SW_IO, uint8_t _SW_GND);
        void Geekble_LieDetector::RunMusic(uint8_t Time_sec, uint16_t Notes[][2]);
        void Geekble_LieDetector::RunLights(uint8_t Time_sec, uint8_t Lights[]);
        void Geekble_LieDetector::RunShocks(uint8_t Time_sec, uint8_t Shocks[]);
        uint16_t Geekble_LieDetector::RunResistanceCheck(uint8_t Time_sec);
        uint16_t Geekble_LieDetector::GetResistance(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[]);
        void Geekble_LieDetector::ReturnResult_Truth(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[]);
        void Geekble_LieDetector::ReturnResult_Lier(uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[]);

        private:
        void byte_out(uint8_t _byte) ;
        void Geekble_LieDetector::Shock(uint8_t ShockVoltage);
        uint16_t Geekble_LieDetector::ResistanceCheck_ADC2Kohm(uint16_t ADC_Value);
        uint16_t Geekble_LieDetector::ServiceEngine(uint8_t FunctionSelect, uint8_t Time_sec, uint16_t Notes[][2], uint8_t Lights[], uint8_t Shocks[]);
        void Program_Lighting (uint8_t GRB_R, uint8_t GRB_G, uint8_t GRB_B, uint8_t Lighting_Mode, uint32_t Time_Spent, uint32_t *Update_Time_Lighting);

***
## Example<br>
<br>

        #include <Geekble_LieDetector.h>

        /*  Pin Definition  */
        #define Pin_V_Check A0
        #define Pin_Shock 9
        #define Pin_R_Test 8
        #define Pin_R_Check A1
        #define Pin_Buzzer 7
        #define Pin_SW_IO 6
        #define Pin_SW_GND 4
        // PIN10 Reserved for PWM
        // PIN11 Reserved for LED DIN
        
        /*  Music   */
        /*
         * unsigned char <Music Name>[][2] = {{<Frequency1>, <duration msec1}, {<Frequency2>, <duration msec2>}, ... {NOTE_END}}
         * PreDefined Notes
         *  - NOTE_REST : REST
         *  - NOTE_B0 ~ NOTE_DS8 : Each Note's Frequency is Defined.
         *  - NOTE_END : Defines the End of a Notes.
         */
        unsigned int Music_HelloWorld[][2] = {{NOTE_DS6, 256}, {NOTE_DS5, 128}, {NOTE_AS5, 384}, {NOTE_GS5, 512}, {NOTE_DS6, 256}, {NOTE_AS5, 512}, {NOTE_END}};
        unsigned int Music_C_Chord[][2] = {{NOTE_C3, 256}, {NOTE_E3, 256}, {NOTE_G3, 256}, {NOTE_REST, 256}, {NOTE_END}};
        unsigned int Music_WatchYourHand[][2] = {{NOTE_G4, 400}, {NOTE_REST, 200}, {NOTE_FS4, 200}, {NOTE_G4, 200}, {NOTE_A4, 400}, {NOTE_REST, 200}, {NOTE_D4, 200}, {NOTE_E4, 200}, {NOTE_F4, 400}, {NOTE_REST, 200}, {NOTE_B3, 200}, {NOTE_C4, 200}, {NOTE_D4, 400}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 2000}, {NOTE_END}};
        unsigned int Music_DingDongDengDong[][2] = {{NOTE_C3, 512}, {NOTE_E3, 512}, {NOTE_G3, 512}, {NOTE_C4, 512}, {NOTE_REST, 1024}, {NOTE_END}};
        unsigned int Music_Annoying[][2] = {{1000, 50}, {1100, 50}, {1200, 50}, {1300, 50}, {1400, 50}, {1500, 50}, {NOTE_END}};
        unsigned int Music_Beep[][2] = {{NOTE_G4, 300}, {NOTE_REST, 1200}, {NOTE_END}};

        unsigned int Music_Mute[][2] = {{NOTE_REST, 1499}, {NOTE_END}};

        /*  Lights  */
        /*
         * unsigned char <Light Mode Name>[] = {<Red>, <Green>, <Blue>, <Effect Code>};
         * Use the RGB Color code to select a color.
         * Effect Code
         *  - Normal_Lighting
         *  - Blink_Lighting
         *  - Swing_Horizontal_Lighting
         *  - Swing_Vertical_Lighting
         *  - Metrix_Lighting
         */
        unsigned char GeekbleColor_NormalMode[4] = {40, 200, 30, Normal_Lighting};
        unsigned char Red_BlinkMode[4] = {200, 0, 0, Blink_Lighting};
        unsigned char Blue_BreathMode[4] = {0, 0, 200, Breath_Lighting};
        unsigned char White_SwingHorizontalMode[4] = {100, 100, 100, Swing_Horizontal_Lighting};
        unsigned char Green_SwingVerticalMode[4] = {0, 200, 0, Swing_Vertical_Lighting};
        unsigned char Purple_MetrixMode[4] = {200, 40, 30, Metrix_Lighting};

        /*  Shocks  */
        /*
         * unsigned char <Shock Mode Name>[] = {<Shock Voltage>, <Number of Iterations>};
         * Shock Voltage Range : From 20V to 50V
         * Iterations Range : From 1 to 50
         */
        unsigned char LongSoft_ShockMode[] = {25, 30};
        unsigned char ShortHard_ShockMode[] = {40, 1};

        Geekble_LieDetector LieDetector;

        unsigned int Calibration_Resistance = 0;
        unsigned int Tested_Resistance = 0;

        void setup()
        {
            LieDetector.attach(Pin_V_Check, Pin_Shock, Pin_R_Test, Pin_R_Check, Pin_Buzzer, Pin_SW_IO, Pin_SW_GND);
            LieDetector.RunLights(1, GeekbleColor_NormalMode);
            LieDetector.RunMusic(2, Music_HelloWorld);
        }

        void loop()
        {
            delay(10);
            char SW_TEMP = LieDetector.Read_SW();

            if (SW_TEMP == 0)       // Idle Status or Wrong Switch Function
            {
                ;
            }

            else if (SW_TEMP == 1)  // Short Press Function -> Test Mode
            {
                if (Calibration_Resistance !=0)
                {
                    Tested_Resistance = LieDetector.GetResistance(7, Music_WatchYourHand, White_SwingHorizontalMode);
                    Serial.println("Test Complete!");
                    Serial.print("Current Hand Resistance is ");
                    Serial.print(Tested_Resistance);
                    Serial.println("KR.");
                    if (Tested_Resistance == 0)
                    {
                        Serial.println("Hand detached! Test again");
                        LieDetector.RunMusic(1, Music_Beep);
                    }
                    else if (Tested_Resistance <= Calibration_Resistance)
                    {
                        Serial.println("Result: Lier!");
                        LieDetector.ReturnResult_Lier(5, Music_Annoying, Red_BlinkMode, LongSoft_ShockMode);
                    }
                    else
                    {
                        Serial.println("Result: PASS!");
                        LieDetector.ReturnResult_Truth(3, Music_DingDongDengDong, Green_SwingVerticalMode);
                    }

                    LieDetector.RunLights(1, GeekbleColor_NormalMode);
                }
                else
                {
                    Serial.println("Run Calibration First!");
                    LieDetector.RunMusic(1, Music_Beep);
                }
            }

            else if (SW_TEMP == 2)  // Long Press Function  -> Calibration Mode
            {
                Calibration_Resistance = LieDetector.GetResistance(6, Music_C_Chord, Blue_BreathMode);
                Serial.println("Calibration Complete!");
                Serial.print("Current Hand Resistance is ");
                Serial.print(Calibration_Resistance);
                Serial.println("KR.");
            }

        }
