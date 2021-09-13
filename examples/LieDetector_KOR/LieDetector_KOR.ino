
#include <Geekble_LieDetector.h>

/*  거짓말 탐지기 Control Module과 아두이노 나노의 핀이 어느곳에 연결될지 설정합니다.  */
#define Pin_V_Check A0  // Analog Pin 0을 Control Module의 V CHECK에 연결하는것으로 설정합니다.
#define Pin_Shock 9     // Digital Pin 9를 Control Module의 SHOCK에 연결하는것으로 설정합니다.
#define Pin_R_Test 8    // Digital Pin 8을 Control Module의 R TEST에 연결하는것으로 설정합니다.
#define Pin_R_Check A1  // Analog Pin 1을 Control Module의 R CHECK에 연결하는것으로 설정합니다.
#define Pin_Buzzer 7    // Digital Pin 7을 Control Module의 BUZZER에 연결하는것으로 설정합니다.
#define Pin_SW_IO 6     // Digital Pin 6을 스위치 구동을 위한 IO 핀으로 설정합니다.
#define Pin_SW_GND 4    // Digital Pin 4를 스위치 구동을 위한 LOW 핀으로 설정합니다.
// Digital Pin 10은 반드시 Control Module의 PWM과 연결되어야 합니다.
// Digital Pin 11은 반드시 Light Module의 LED DIN과 연결되어야 합니다.

/*  멜로디   */
/*
 * unsigned char <멜로디 이름>[][2] = {{<주파수 1>, <재생시간 msec1}, {<주파수 2>, <재생시간 msec2>}, ... {NOTE_END}}
 * 정의된 요소들
 *  - NOTE_REST : 쉼표
 *  - NOTE_B0 부터 NOTE_DS8 까지의 음계에 해당하는 주파수가 정의되어있습니다.
 *  - NOTE_END : 멜로디의 끝을 알립니다. 멜로디의 끝에 반드시 포함되어야합니다.
 */
unsigned int Music_HelloWorld[][2] = {{NOTE_DS6, 256}, {NOTE_DS5, 128}, {NOTE_AS5, 384}, {NOTE_GS5, 512}, {NOTE_DS6, 256}, {NOTE_AS5, 512}, {NOTE_END}};
unsigned int Music_C_Chord[][2] = {{NOTE_C3, 256}, {NOTE_E3, 256}, {NOTE_G3, 256}, {NOTE_REST, 256}, {NOTE_END}};
unsigned int Music_WatchYourHand[][2] = {{NOTE_G4, 400}, {NOTE_REST, 200}, {NOTE_FS4, 200}, {NOTE_G4, 200}, {NOTE_A4, 400}, {NOTE_REST, 200}, {NOTE_D4, 200}, {NOTE_E4, 200}, {NOTE_F4, 400}, {NOTE_REST, 200}, {NOTE_B3, 200}, {NOTE_C4, 200}, {NOTE_D4, 400}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_G3, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 200}, {NOTE_C4, 200}, {NOTE_REST, 2000}, {NOTE_END}};
unsigned int Music_DingDongDengDong[][2] = {{NOTE_C3, 512}, {NOTE_E3, 512}, {NOTE_G3, 512}, {NOTE_C4, 512}, {NOTE_REST, 1024}, {NOTE_END}};
unsigned int Music_Annoying[][2] = {{1000, 50}, {1100, 50}, {1200, 50}, {1300, 50}, {1400, 50}, {1500, 50}, {NOTE_END}};
unsigned int Music_Beep[][2] = {{NOTE_REST, 300}, {NOTE_G4, 300}, {NOTE_REST, 400}, {NOTE_END}};

unsigned int Music_Mute[][2] = {{NOTE_REST, 1499}, {NOTE_END}};

/*  조명  */
/*
 * unsigned char <조명 이름>[] = {<빨강>, <초록>, <파랑>, <조명 효과 코드>};
 * RGB(빨강, 초록, 파랑)값을 0 ~ 255 사이의 값을 이용하여 색상을 고르세요.
 * 조명 효과 코드 리스트
 *  효과 없음 - Normal_Lighting
 *  깜빡임 효과 - Blink_Lighting
 *  수평 조명 왕복 효과 - Swing_Horizontal_Lighting
 *  수직 조명 왕복 효과 - Swing_Vertical_Lighting
 *  무작위 변형 효과 - Metrix_Lighting
 */
unsigned char GeekbleColor_NormalMode[4] = {40, 200, 30, Normal_Lighting};
unsigned char Red_BlinkMode[4] = {200, 0, 0, Blink_Lighting};
unsigned char Blue_BreathMode[4] = {0, 0, 200, Breath_Lighting};
unsigned char White_SwingHorizontalMode[4] = {100, 100, 100, Swing_Horizontal_Lighting};
unsigned char Green_SwingVerticalMode[4] = {0, 200, 0, Swing_Vertical_Lighting};
unsigned char Purple_MetrixMode[4] = {200, 40, 30, Metrix_Lighting};

/*  전기충격  */
/*
 * *** 경고 ***
 * 아래에 주어진 2개의 예시 모드 이외의 전기충격 모드를 만드는 경우 각별히 유의하세요.
 * 처음부터 가장 강한 강도를 설정하게 되면 매우 놀랄 수 있습니다.
 * 이는 예기치 못한 상해로 이어질 수 있으니 반드시 약한 강도부터 천천히 강도를 올려 테스트를 진행하십시오.
 * 
 * unsigned char <전기충격 이름>[] = {<전기충격 전압>, <전기충격 반복자극 횟수>};
 * 전기 충격 전압 범위 : 20V ~ 50V
 * 반복자극 횟수 범위 : 1 ~ 50
 */
unsigned char LongSoft_ShockMode[] = {25, 30};
unsigned char ShortHard_ShockMode[] = {40, 1};

Geekble_LieDetector LieDetector;    // Geekble_LieDetector 클래스의 LieDetector 인스턴스를 생성합니다.

unsigned int Calibration_Resistance = 0;    // 캘리브레이션에서 읽어들인 손의 저항값을 저장하기 위한 변수를 생성합니다.
unsigned int Tested_Resistance = 0;         // 거짓말 판별에서 읽어들인 손의 저항값을 저장하기 위한 변수를 생성합니다.

void setup()        // 설정을 진행합니다.
{
    /*  attach 명령어   */
    /* attach 명령어는 거짓말 탐지기 동작에 필요한 설정을 진행하는 명령어입니다.
     * 특히 사용자는 attach 명령어를 이용해 거짓말 탐지기 Control Module과 아두이노 나노의 핀이 어느곳에 연결될지 설정하게 됩니다.
     *
     * <인스턴스명>.attach(V CHECK 핀, SHOCK 핀, R TEST 핀, R CHECK핀, BUZZER 핀, SW IO 핀, SW GND 핀);
     */
    LieDetector.attach(Pin_V_Check, Pin_Shock, Pin_R_Test, Pin_R_Check, Pin_Buzzer, Pin_SW_IO, Pin_SW_GND);
    /*  Run 명령어  */
    /* Run ... 명령어는 다른 명령어와는 달리 단일 기능을 정해진 시간동안 수행하는 명령어입니다.
     * 아래와 같은 종류의 Run 명령어들이 있습니다.
     *
     * 음악만을 정해진 시간동안 재생:   RunMusic(재생 시간(초), 재생할 멜로디);
     * 조명을 정해진 시간동안 구동:     RunLights(구동 시간(초), 표시할 조명);
     * 전기충격을 정해진 시간동안 자극:     RunShocks(자극 시간(초), 인가할 전기충격 모드);
     * 저항값을 정해진 시간동안 측정:   RunResistanceCheck(획득 시간(초));
     * 
     * 멜로디와 자극은 정해진 시간이 지나면 자동으로 꺼지지만, 조명은 정해진 시간이 지나면 가장 마지막 상태를 유지합니다.
     */
    LieDetector.RunLights(1, GeekbleColor_NormalMode);      // 거짓말 탐지기 초기화 완료를 알리는 불빛을 켭니다.
    LieDetector.RunMusic(2, Music_HelloWorld);              // 거짓말 탐지기 초기화 완료를 알리는 음악을 재생합니다.
}

void loop()
{
    /*  10msec마다 스위치의 상태를 읽어들이고, 읽은 값을 SW_TEMP에 저장합니다.  */
    delay(10);
    char SW_TEMP = LieDetector.Read_SW();

    if (SW_TEMP == 0)       // 스위치가 눌리지 않았거나, 너무 짧게 눌려 노이즈로 판단되는 경우입니다.
    {
        ;
    }

    else if (SW_TEMP == 1)  // 스위치가 짧게 눌린 경우입니다. 거짓말 판별을 시작합니다.
    {
        if (Calibration_Resistance !=0) // 캘리브레이션에서 읽어들인 손의 저항값이 0이 아닌지 확인합니다. 0인 경우 켈리브레이션을 하지 않은 것입니다.
        {
            /*  GetResistance 명령어 */
            /*
             * GetResistance 명령어는 손의 저항값을 측정하여 Kohm 단위로 반환하게 됩니다. 
             * 측정중에는 1초마다 현재 측정되고 있는 값을 시리얼 통신으로 출력합니다.
             * 20KR ~ 800KR범위의 저항값을 측정할 수 있습니다.
             * 측정중 손을 전극에서 떼게 되면 측정이 중단되며, "0"을 반환하게 됩니다.
             * GetResistance 명령어를 사용하면 손의 저항값을 측정하는 "시간", 측정중 재생할 "멜로디", 측정중 표시할 "조명" 을 함께 설정할 수 있습니다.
             *
             * 반환될 값을 저장하기 위한 변수 = <인스턴스명>.GetResistance(측정 시간(초), 재생할 멜로디, 표시할 조명);
             */
            Tested_Resistance = LieDetector.GetResistance(7, Music_WatchYourHand, White_SwingHorizontalMode);
            
            // 측정된 값을 시리얼 모니터에 출력합니다.
            Serial.println("Test Complete!");
            Serial.print("Current Hand Resistance is ");
            Serial.print(Tested_Resistance);
            Serial.println("KR.");
            if (Tested_Resistance == 0) // 측정값 0 : 측정 중 손을 전극에서 뗀 경우입니다.
            {
                Serial.println("Hand detached! Test again");    // 손을 떼었으니 다시 시도하라는 메시지를 출력합니다.
                LieDetector.RunMusic(1, Music_Beep);            // 오류음을 재생합니다.
            }
            else if (Tested_Resistance <= Calibration_Resistance)   // 손의 저항값이 켈리브레이션 때 보다 낮아진 경우입니다.
            {
                /* 손의 저항값이 캘리브레이션 때 보다 낮아짐    */
                /*
                 * 손의 저항값에 영향을 미치는 요소는 아주 많습니다.
                 * 그 중 손에 땀의 양 또한 영향을 미치게 됩니다.
                 * 손에 땀이 많이 나면 전자를 이동시킬 이온이 많아져 저항값이 낮아지게 됩니다.
                 * 마치 소금물이 전기가 잘 통하는것과 비슷합니다.
                 * 거짓말을 하여 긴장하게 되면 땀이 나는것에 착안하여, 거짓말이라고 판별합니다.
                 */
                Serial.println("Result: Lier!");                    // 거짓말로 판별되었다는 메시지를 시리얼 통신으로 출력합니다.

                /*  ReturnResult_Lier 명령어    */
                /*
                 * ReturnResult_Lier 명령어는 정해진 시간동안 거짓말이라는 피드백을 사용자에게 전달하기 위한 명령어입니다.
                 * ReturnResult_Lier 명령어를 사용하면 피드백을 줄 "시간", 피드백 중 재생할 "멜로디", 피드백 중 표시할 "조명", 피드백 중 인가할 "전기충격" 을 함께 설정할 수 있습니다.
                 * 
                 * <인스턴스명>.RetrunResult_Lier(피드백 시간(초), 재생할 멜로디, 표시할 조명, 인가할 전기충격)
                 */
                LieDetector.ReturnResult_Lier(5, Music_Annoying, Red_BlinkMode, LongSoft_ShockMode);
            }
            else                                                    // 손의 저항값이 켈리브레이션 때 보다 높아진 경우입니다.
            {
                /*  손의 저항값이 캘리브레이션 때 보다 높아짐   */
                /* 이 경우는 반대로 손의 저항값이 낮아졌기 때문에 적어도 땀을 더 흘리지는 않았도고 판단하여 진실이라고 판별합니다.
                 */
                Serial.println("Result: PASS!");                    // 진실로 판별되었다는 메시지를 시리얼 통신으로 출력합니다.

                /*   ReturnResult_Truth 명령어  */
                /* 
                 * ReturnResult_Truth 명령어는 정해진 시간동안 진실이라는 피드백을 사용자에게 전달하기 위한 명령어입니다.
                 * ReturnResult_Truth 명령어를 사용하면 피드백을 줄 "시간", 피드백 중 재생할 "멜로디", 피드백 중 표시할 "조명" 을 함께 설정할 수 있습니다.
                 *
                 * <인스턴스명>.RetrunResult_Truth(피드백 시간(초), 재생할 멜로디, 표시할 조명)
                 */
                LieDetector.ReturnResult_Truth(3, Music_DingDongDengDong, Green_SwingVerticalMode);
            }

            LieDetector.RunLights(1, GeekbleColor_NormalMode);  // 기본 조명 색상으로 돌아가 다음 거짓말 탐지가 준비되었음을 표시합니다.
        }
        else        // 캘리브레이션에서 읽어들인 손의 저항값이 0인 경우입니다. 0인 경우 켈리브레이션을 하지 않은 것입니다.
        {
            Serial.println("Run Calibration First!");   // 먼저 켈리브레이션을 하라는 메시지를 시리얼 통신으로 출력합니다.
            LieDetector.RunMusic(1, Music_Beep);        // 오류음을 재생합니다.
        }
    }

    else if (SW_TEMP == 2)  // 스위치가 길게 눌린 경우입니다. 켈리브레이션을 시작합니다.
    {
        Calibration_Resistance = LieDetector.GetResistance(6, Music_C_Chord, Blue_BreathMode);  // GetResistance 명령어를 이용해 Calibration_Resistance 변수에 현재 손의 저항값을 저장합니다.
        // 캘리브레이션 결과값을 시리얼 통신으로 출력합니다.
        Serial.println("Calibration Complete!");
        Serial.print("Current Hand Resistance is ");
        Serial.print(Calibration_Resistance);
        Serial.println("KR.");
    }

}
