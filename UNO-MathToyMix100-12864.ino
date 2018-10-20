#include <avr/wdt.h>
#include <U8g2lib.h>

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

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clo  ck=*/ 15 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 14 /* A3 */, /* reset=*/ 17); // 17, U8X8_PIN_NONE

unsigned long keypadDebounceTime = 200; // 200 milliseconds debounce time
unsigned long keypadLastDebounce = 0;
bool keypadDebounce = false;
String strReturnNumber = "";
int intReturnNumber = -1;

int questionCount = 0;
const int questionTotal = 100;
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
  shortBeep(ALARM_PIN);
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
          u8g2.setFont(u8g2_font_helvB10_tf); // u8g2_font_helvB08_tf, u8g2_font_6x13_tn
          u8g2.setCursor(1, 40);
          u8g2.print(strReturnNumber);
          if (intReturnNumber == currentAnswer.toInt())
          {
            u8g2.print("V");
            shortBeep(ALARM_PIN);
          }
          else
          {
            u8g2.print("X");
            longBeep(ALARM_PIN);
          }
          currentMode = 1;
        }
        else
        {
          currentMode = 0;
          currentQuestion = generateMathQuestion(currentAnswer);
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
  u8g2.begin();
  setupKeypad();
  currentMode = 0;
  currentQuestion = generateMathQuestion(currentAnswer);
}

void loop(void) {
  scanKeypad();
  u8g2.firstPage();
  do {
    drawMath();
  } while (u8g2.nextPage());
}

void drawMath(void) {
  u8g2.setFont(u8g2_font_helvB10_tf); // u8g2_font_helvB08_tf, u8g2_font_6x13_tn
  u8g2.setCursor(1, 1);
  u8g2.print(questionCount);
  u8g2.print("/");
  u8g2.print(questionTotal);

  u8g2.setFont(u8g2_font_helvB12_tf); // u8g2_font_helvB08_tf, u8g2_font_10x20_tf
  int stringWidth = u8g2.getStrWidth(string2char(currentAnswer));
  u8g2.setCursor((128 - stringWidth) / 2, 28);
  if (currentMode == 0)
  {
    u8g2.print(currentQuestion);
  }
  else
  {
    u8g2.print(currentAnswer);
  }
}

void software_Reboot() {
  wdt_enable(WDTO_15MS);
  while (1)
  {
  }
}

void shortBeep(int Alarm_PIN) {
  digitalWrite(Alarm_PIN, HIGH);
  delay(200);
  digitalWrite(Alarm_PIN, LOW);
}

void longBeep(int Alarm_PIN) {
  digitalWrite(Alarm_PIN, HIGH);
  delay(2000);
  digitalWrite(Alarm_PIN, LOW);
}

void noBeep(int Alarm_PIN) {
  digitalWrite(Alarm_PIN, LOW);
}

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

String generateMathQuestion(String &Answer) {
  const String strPlusSign = "+";
  const String strMinusSign = "-";
  const String strMultiplySign = "X";
  const String strDivideySign = String((char)247);
  const String strEqualSign = "=";
  String MathQuestion = "";
  int intFirstOperationType = random(1, 4); // 1 - plus, 2 - minus, 3 - multiply
  int intSecondOperationType = 1;
  int intFirstNumber = 1;
  int intSecondNumber = 1;
  int intThirdNumber = 1;

  if (intFirstOperationType == 3)
  {
    intSecondOperationType = random(1, 3);
    intFirstNumber = random(1, 10);
    intSecondNumber = random(1, 10);
    if (intSecondOperationType == 2)
    {
      intThirdNumber = random(1, intFirstNumber * intSecondNumber);
      MathQuestion = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber * intSecondNumber - intThirdNumber);
    }
    else
    {
      intThirdNumber = random(1, 100);
      MathQuestion = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber * intSecondNumber + intThirdNumber);
    }
  }
  else if (intFirstOperationType == 2)
  {
    intSecondOperationType = random(1, 4);
    intFirstNumber = random(50, 100);
    if (intSecondOperationType == 1)
    {
      intSecondNumber = random(0, intFirstNumber);
      intThirdNumber = random(1, 100);
      MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - intSecondNumber + intThirdNumber);
    }
    else if (intSecondOperationType == 2)
    {
      intSecondNumber = random(30, intFirstNumber);
      intThirdNumber = random(1, (intFirstNumber - intSecondNumber));
      MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - intSecondNumber - intThirdNumber);
    }
    else // multiply
    {
      intSecondNumber = random(1, 10);
      intThirdNumber = random(1, 10);
      intFirstNumber = random(intSecondNumber * intThirdNumber, 100);
      MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - (intSecondNumber * intThirdNumber));
    }
  }
  else // first operation is plus
  {
    intSecondOperationType = random(1, 4);
    intFirstNumber = random(1, 100);
    if (intSecondOperationType == 1)
    {
      intSecondNumber = random(1, 100);
      intThirdNumber = random(1, 100);
      MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + intSecondNumber + intThirdNumber);
    }
    else if (intSecondOperationType == 2)
    {
      intSecondNumber = random(1, 100);
      intThirdNumber = random(1, (intFirstNumber - intSecondNumber));
      MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + intSecondNumber - intThirdNumber);
    }
    else
    {
      intSecondNumber = random(1, 10);
      intThirdNumber = random(1, 10);
      MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + "?";
      Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + (intSecondNumber * intThirdNumber));
    }
  }
  return MathQuestion;
}

