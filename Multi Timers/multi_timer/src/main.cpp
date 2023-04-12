#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include<math.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define upButton 6
#define downButton 7
#define enterButon 8
#define motorStop 9
#define motorOn 10
#define gasValveOutput A0
#define sparkOutput A1
#define motorOutput A2

int screenIndex = 0;
int settingsIndex = 0;
unsigned long oldTime = 0;
unsigned long remainingTime = 0;
const unsigned long buttonDelay = 300;
const int itemsListLength = 8;
const int settingsListLenght = 4;

unsigned long sparkTime = 1000;
unsigned long gasValveTime = 2000;
unsigned long motorDelayOffTime = 60000;

String itemsList[8] = {
    "Zeri3a",
    "Big Kawkaw",
    "Kawkaw",
    "Lger3a",
    "Akajou",
    "Louz",
    "L7mess",
    "Settings",
};
String settingsMenuItems[4] = {
    "Fruits",
    "Gas Valve Time",
    "Spark Time",
    "Motor Delay",
};
unsigned long delaytimes[7] = {
    270000, // 5 minutes
    10000,  // 10 minutes
    20000,  // 15 minutes
    30000,  // 20 minutes
    40000,  // 25 minutes
    500000, // 30 minutes
    60000,  // 35 minutes
};
enum state
{
  on,
  off,
};

void shutDown()
{
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(gasValveOutput, off);
  digitalWrite(sparkOutput, off);
}
void setMotorState(int state)
{
  digitalWrite(motorOutput, state);
}
void motorSequence(int state)
{
  if (state == on)
  {
    setMotorState(on);
  }
  else
  {
    setMotorState(off);
  }
}
void turnOffEverything(){
    shutDown();
          motorSequence(off);
          lcd.clear();
          lcd.print("Stopping");
          delay(2000);
}
void wait(unsigned long period, unsigned long t_old = 0)
{
  t_old = millis();
  unsigned long t_new = millis();
  while (t_new - t_old < period)
  {
    if (digitalRead(motorStop) == LOW)
    {
     turnOffEverything();
      return;
    }
    t_new = millis();
  }
}

void StartSequence(uint8_t state)
{
  if (state == on)
  {
    lcd.clear();
    digitalWrite(LED_BUILTIN, !state);
    lcd.print("Gas Valve On");
    digitalWrite(gasValveOutput, on);
    wait(gasValveTime);
    digitalWrite(sparkOutput, on);
    lcd.setCursor(0, 1);
    lcd.print("Spark ON");
    wait(sparkTime);
    digitalWrite(sparkOutput, off);
    lcd.setCursor(0, 1);
    lcd.print("Spark OFF");
    wait(500);
  }
  else if (state == off)
  {
    digitalWrite(gasValveOutput, state);
  }
}

void displayList(String arr[], int itemsNumber, int listIndex)
{
  lcd.clear();

  if (listIndex != itemsNumber - 1)
  {
    lcd.setCursor(2, 0);
    lcd.print(arr[listIndex]);
    lcd.setCursor(2, 1);
    lcd.print(arr[listIndex + 1]);
  }
  else
  {
    lcd.setCursor(2, 0);
    lcd.print(arr[itemsNumber - 1]);
    lcd.setCursor(2, 1);
    lcd.print(arr[0]);
  }
  lcd.setCursor(0, 0);
  lcd.print("->");
  delay(500);
}
int handleScrolling(int wantedScreenIndex, int listLength, String mList[])
{

  if (digitalRead(upButton) == LOW)
  {
    if (wantedScreenIndex < listLength - 1)
    {
      wantedScreenIndex += 1;
      delay(buttonDelay);
    }
    else
    {
      wantedScreenIndex = 0;
      delay(buttonDelay);
    }
    displayList(mList, listLength, wantedScreenIndex);
  }
  if (digitalRead(downButton) == LOW)
  {
    if (wantedScreenIndex > 0)
    {
      wantedScreenIndex -= 1;
      delay(buttonDelay);
    }
    else
    {
      wantedScreenIndex = listLength - 1;
      delay(buttonDelay);
    }
    displayList(mList, listLength, wantedScreenIndex);
  }
  return wantedScreenIndex;
}
void settingsMenu()
{
  lcd.clear();
  lcd.print("Settings Menu");
  wait(2000);
  lcd.clear();
  displayList(settingsMenuItems, 4, settingsIndex);
  while (digitalRead(enterButon) == HIGH)
  {
    settingsIndex = handleScrolling(settingsIndex, 4, settingsMenuItems);
  }
}
void setRemTime(unsigned long elapsed , unsigned long compVar)
{
  unsigned long elapsedVal = elapsed / 1000;
  remainingTime = (compVar / 1000) - elapsedVal;
}
bool isCountingFor(unsigned long val)
{
  bool isCounting = true;
  unsigned long elapsedTime = millis() - oldTime;

  if (elapsedTime >= val)
  {
    isCounting = false;
  }
  if (elapsedTime >= 1000)
  {
    setRemTime(elapsedTime , val);
  }
  return isCounting;
}
int parseTime(unsigned long ti)
{
  int parsedTime = 0;
  if (ti >= 60)
  {
    Serial.println(ti);
    float eq = float(ti / 60.);
    Serial.println(eq) ; 
    eq = eq * 100 ;
    Serial.println(eq);
   float eqInt = 0 ;
   eqInt = fmodf(eq , 100.);
   //eqInt % 100;
    Serial.println(eqInt);
    unsigned long mul = 60 * eqInt;
    Serial.println(mul);
    if (mul % 100 >= 50)
    {
      mul += 100;
    }
    mul = mul / 100;
    Serial.println(mul);
    parsedTime = mul ; 
  }
  else
  {
    parsedTime = ti;
  }

  return parsedTime;
}
void displayRunScreen(int index, unsigned long variable, String Header, unsigned long compVar)
{
  unsigned long minutes = 0;
  int seconds = 0;
    if(compVar >= 60000){
      minutes = variable /60 ; 
      seconds = parseTime(variable)  ;
      Serial.println(seconds);
    }else{
      minutes = 0;
      seconds = parseTime(variable) ; 
      
    }
   

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(Header);
  lcd.setCursor(0, 1);
  lcd.print("Time Rem:");
  lcd.setCursor(11, 1);
  if (minutes < 10)
  {
    lcd.print("0");
  }
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10)
  {
    lcd.print("0");
    lcd.print(seconds);
  }
  else
  {
    lcd.print(seconds);
  }
}

void setup()
{
  // put your setup code here, to run once:
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(enterButon, INPUT_PULLUP);
  pinMode(motorOn, INPUT_PULLUP);
  pinMode(motorStop, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(gasValveOutput, OUTPUT);
  pinMode(sparkOutput, OUTPUT);

  StartSequence(off);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Welcome");
  delay(3500);
  lcd.clear();
  displayList(itemsList, itemsListLength, screenIndex);
}

void loop()
{
  // put your main code here, to run repeatedly:
  screenIndex = handleScrolling(screenIndex, itemsListLength, itemsList);
  if (digitalRead(enterButon) == LOW && screenIndex == itemsListLength - 1)
  {
    settingsMenu();
    return;
  }
  if (digitalRead(enterButon) == LOW)
  {
    if (screenIndex != itemsListLength - 1)
    {
      remainingTime = delaytimes[screenIndex] / 1000;
      StartSequence(on);
      lcd.clear();
      lcd.print("Waiting For Motor Start Signal");
      delay(500);

      do
      {
        unsigned long y_old = millis();
        for (int i = 0; i < 16; i++)
        {
          if (digitalRead(motorStop) == LOW)
          {
           turnOffEverything();
            remainingTime = 0;
            displayList(itemsList, itemsListLength, screenIndex);
            return;
          }
          if (digitalRead(motorOn) == LOW)
          {
            break;
          }
          lcd.scrollDisplayLeft();
          wait(350, y_old);
        }

      } while (digitalRead(motorOn) == HIGH);
      motorSequence(on);
      oldTime = millis();
      while (isCountingFor(delaytimes[screenIndex]))
      {
        if (digitalRead(motorStop) == LOW)
        {
          turnOffEverything() ; 
          break;
        }
        displayRunScreen(screenIndex, remainingTime, itemsList[screenIndex] , delaytimes[screenIndex]);
      }
      shutDown();
      remainingTime = motorDelayOffTime / 1000;
      oldTime = millis();
      while (isCountingFor(motorDelayOffTime))
      {
        if (digitalRead(motorStop) == LOW)
        {
        turnOffEverything();
          break;
        }
        displayRunScreen(screenIndex, remainingTime, "Motor Delay Off" , motorDelayOffTime);
      }
      displayList(itemsList, itemsListLength, screenIndex);
    }
  }
}