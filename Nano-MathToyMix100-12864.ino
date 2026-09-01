#include <U8g2lib.h>
#include "StringHelpers.h"
#include "AlarmBeeper.h"
#include "MathQuizGenerator.h"

#define ALARM_PIN 13
#define KEYPAD_0_PIN 4
#define KEYPAD_1_PIN 0
#define KEYPAD_2_PIN 5
#define KEYPAD_3_PIN 9
#define KEYPAD_4_PIN 3
#define KEYPAD_5_PIN 6
#define KEYPAD_6_PIN 2
#define KEYPAD_7_PIN 1
#define KEYPAD_8_PIN 7
#define KEYPAD_9_PIN 10
#define KEYPAD_DOT_PIN 8
#define KEYPAD_A_PIN 11
#define KEYPAD_B_PIN 12

U8G2_ST7920_128X64_F_SW_SPI display(U8G2_R0, /* clo  ck=*/ 15 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 14 /* A3 */, /* reset=*/ 17); // 17, U8X8_PIN_NONE

#define NUMBER_CEILING 10000

unsigned long keypadDebounceTime = 200; // 200 milliseconds debounce time
unsigned long keypadLastDebounce = 0;
bool keypadDebounce = false;
String strReturnNumber = "";
int intReturnNumber = -1;

int questionCount = 0;
const int questionTotal = 20;
int currentMode = 0; // 0 - show question, 1 - show answer
String currentQuestion = "";
String currentAnswer = "";

void setupKeypad() {
  pinMode(KEYPAD_0_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_1_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_2_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_3_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_4_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_5_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_6_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_7_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_8_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_9_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_DOT_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_A_PIN, INPUT_PULLUP);
  pinMode(KEYPAD_B_PIN, INPUT_PULLUP);
  pinMode(ALARM_PIN, OUTPUT);
  beep(ALARM_PIN, true, 5);
}

void scanKeypadSub(int pinNumber, String strNumber) {
  int numberValue = digitalRead(pinNumber);
  if (numberValue == 0)
  {
    if (keypadDebounce == true)
    {
      if ((millis() - keypadLastDebounce)  > keypadDebounceTime )
      {
        keypadLastDebounce = millis();
        strReturnNumber = strReturnNumber + strNumber;
        keypadDebounce = false;
      }
    }
    else
    {
      keypadDebounce = true;
    }
  }
}

void scanKeypadFunctionKeyA(int pinNumber) {
  int numberValue = digitalRead(pinNumber);
  if (numberValue == 0)
  {
    if (keypadDebounce == true)
    {
      if ((millis() - keypadLastDebounce)  > keypadDebounceTime )
      {
        keypadLastDebounce = millis();
        intReturnNumber = strReturnNumber.toInt();
        keypadDebounce = false;
        if (currentMode == 0)
        {
          currentMode = 1;
          if (intReturnNumber == extractResultFromAnswer(currentAnswer))
          {
            beep(ALARM_PIN, true, 5);
          }
          else
          {
            beepPattern(ALARM_PIN, true, 8, 30, 30);
            questionCount = -1;
          }
        }
        else
        {
          currentMode = 0;
          strReturnNumber = "";
          intReturnNumber = -1;
          currentQuestion = generateMathQuestion(currentAnswer, NUMBER_CEILING, true);
          questionCount++;
          if (questionCount >= questionTotal + 1)
          {
            questionCount = 1;
          }
        }
      }
    }
    else
    {
      keypadDebounce = true;
    }
  }
}

void scanKeypadFunctionKeyB(int pinNumber) {
  int numberValue = digitalRead(pinNumber);
  if (numberValue == 0)
  {
    if (keypadDebounce == true)
    {
      if ((millis() - keypadLastDebounce)  > keypadDebounceTime )
      {
        keypadLastDebounce = millis();
        if (strReturnNumber.length() > 0)
        {
          strReturnNumber = strReturnNumber.substring(0, strReturnNumber.length() - 1);
        }
        keypadDebounce = false;
      }
    }
    else
    {
      keypadDebounce = true;
    }
  }
}

void scanKeypad() {
  scanKeypadSub(KEYPAD_0_PIN, "0");
  scanKeypadSub(KEYPAD_1_PIN, "1");
  scanKeypadSub(KEYPAD_2_PIN, "2");
  scanKeypadSub(KEYPAD_3_PIN, "3");
  scanKeypadSub(KEYPAD_4_PIN, "4");
  scanKeypadSub(KEYPAD_5_PIN, "5");
  scanKeypadSub(KEYPAD_6_PIN, "6");
  scanKeypadSub(KEYPAD_7_PIN, "7");
  scanKeypadSub(KEYPAD_8_PIN, "8");
  scanKeypadSub(KEYPAD_9_PIN, "9");
  //  scanKeypadSub(KEYPAD_DOT_PIN, "."); // We do not need to use for now
  scanKeypadFunctionKeyA(KEYPAD_A_PIN);
  scanKeypadFunctionKeyB(KEYPAD_B_PIN);
}

void setup(void) {
  randomSeed(analogRead(5));
  display.begin();
  display.setFontPosTop();

  display.clearBuffer();
  display.setFont(u8g2_font_helvB12_tf); // u8g2_font_helvB08_tf, u8g2_font_6x13_tn
  display.setCursor(5, 25);
  display.print("Math Exercises");
  display.sendBuffer();
  setupKeypad();
  delay(2000);
  currentMode = 0;
  currentQuestion = generateMathQuestion(currentAnswer, NUMBER_CEILING, true);
}

void loop(void) {
  scanKeypad();
  display.firstPage();
  do {
    drawMath();
  } while (display.nextPage());
}

void drawMath(void) {
  display.setFont(u8g2_font_helvB08_tf); // u8g2_font_helvB08_tf, u8g2_font_6x13_tn
  display.setCursor(1, 1);
  if (questionCount > -1)
  {
    display.print(questionCount);
    display.print("/");
    display.print(questionTotal);
  }
  else
  {
    display.print("Wrong!");
  }

  display.setFont(u8g2_font_helvR08_tf); // u8g2_font_helvB08_tf, u8g2_font_10x20_tf
  int stringWidth = display.getStrWidth(string2char(currentAnswer));
  display.setCursor((128 - stringWidth) / 2, 25);
  if (currentMode == 0)
  {
    display.print(currentQuestion);
    if (strReturnNumber.length() > 0)
    {
      display.setCursor(50, 48);
      display.print(strReturnNumber);
    }
  }
  else
  {
    display.print(currentAnswer);
    display.setCursor(50, 48);
    display.print(strReturnNumber);
    display.setFont(u8g2_font_open_iconic_check_2x_t); // u8g2_font_helvB08_tf, u8g2_font_10x20_tf, u8g2_font_helvB12_tf, u8g2_font_open_iconic_check_2x_t
    display.setCursor(85, 46);
    if (intReturnNumber == extractResultFromAnswer(currentAnswer))
    {
      display.print("A");
    }
    else
    {
      display.print("B");
    }
  }
}

int extractResultFromAnswer(String Answer) {
  int endPos = Answer.indexOf("=");
  if (endPos > 0)
  {
    String tempString = Answer.substring(endPos + 1);
    return tempString.toInt();
  }
  else
  {
    return 0;
  }
}
