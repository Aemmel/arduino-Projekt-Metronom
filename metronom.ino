#include <LiquidCrystal.h>

#define POT A0
#define BUT_1 A1
#define BUT_2 A2
#define LED_RED 7
#define LED_BLU 8
#define BUZZER 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
bool but1toggle = false; //did they get pressed? if so say yes, but only once per press, no continues signal when pressing for a while
bool but2toggle = false;

//to not get confused because digitalRead(BUT_1) == LOW means pressed:
bool pressed(int num)
{
  switch(num) {
    case 1:
      return digitalRead(BUT_1) == LOW;
    case 2:
      return digitalRead(BUT_2) == LOW;
    
    default:
      break;
  }
}

//check if buttons got pressed, send 1 signal only
//probably an easier way to do this, but I dont care
void checkButtons()
{
  static bool locked1 = false;
  static bool locked2 = false;

  //Button 1
  if (locked1 == false && pressed(1) && but1toggle == false) {
    locked1 = true;
    but1toggle = true;
  }
  else {
    but1toggle = false;
  }

  //Button 2
  if (locked2 == false && pressed(2) && but2toggle == false) {
    locked2 = true;
    but2toggle = true;
  }
  else {
    but2toggle = false;
  }
  
  locked1 = pressed(1) ? true : false;
  locked2 = pressed(2) ? true : false;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLU, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(BUT_1, INPUT_PULLUP);
  pinMode(BUT_2, INPUT_PULLUP);

  lcd.begin(16, 2);
  
  Serial.begin(9600);
}

// -----------

//constants
const int freqFirst = 1700;
const int freqFourths = 1500;
const int freqEighths = 1300;

//variables
int potVal = 0;
int interval = 0; //interval between ticks in ms    min value = 250 (240 BPM) ---- max value = 1500 (40 BPM)
unsigned long elapsedInterval = millis();
unsigned long elapsedTick;
unsigned long elapsedLcd = millis();
bool tick = false;
int cnt = 0; //to count the beats in a rythm
int rythm = 4; //rythm in fourths; Standard rythm = 4 -> rythm is 4/4; if rythm = 6 -> rythm is 6/4
bool eighths = false; //show and beep eights
int freq;
int led;

//make a whole function for it to not litter the loop function more than necessary
void handleButtons()
{
  //button 1 changes rythm
  if (but1toggle == true) {
    rythm = rythm > 8 ? 1 : rythm + 1; //reset after rythm = 9 to rythm = 1
  }
  
  //button 2 toggles eighths
  if (but2toggle == true) {
    eighths = !eighths;
  }
}

void showBPM()
{
  //can't do it in one step because of stupid rounding errors
  float i = interval * 2;
  float is = i / 1000;
  int bpm = 60 / is;
  
  lcd.setCursor(0, 0);
  lcd.print("BPM");
  lcd.setCursor(4, 0);
  lcd.print(bpm);
}

void showRythm()
{
  lcd.setCursor(0, 1);
  lcd.print("Rythm  /4");
  lcd.setCursor(6, 1);
  lcd.print(rythm);
}

void showEighths()
{
  lcd.setCursor(11, 1);
  lcd.print("E:");
  lcd.setCursor(13, 1);
  
  if (eighths == true) {
    lcd.print("ON");
  }
  else {
    lcd.print("OFF");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  checkButtons();
  handleButtons();

  //read value of potentionmeter and calculate intervals
  potVal = analogRead(POT);
  interval = map(potVal, 0, 1023, 250, 1500) / 2; //turn the analog read from the potentionmeter into a value for the metronom
  if (millis() - elapsedLcd >= 50) {
    lcd.clear();
    showBPM();
    showRythm();
    showEighths();

    elapsedLcd = millis();
  }

  //timing circuit for ticks
  //
  
  //if enough time passed, give signal for another tick
  if (millis() - elapsedInterval >= interval) {
    elapsedInterval = millis();
    tick = true;
  }

  //turn on Buzzer and LED after tick
  if (tick == true) {
    if (cnt % 2 == 0) {
      if (rythm > 1 && cnt == 0) { //only the first fourth on rythm greater than 1/4
        led = LED_BLU;
        freq = freqFirst;    
      }
      else {
        led = LED_RED;
        freq = freqFourths;
      }
    }
    else if (eighths == true){ //eighths
      led = 0;
      freq = freqEighths;
    }
    else {
      if (eighths == true) {
        led = 0;
        freq = freqEighths;
      }
      else {
        led = 0;
        freq = 0;
      }
    }

    if (led != 0) {
      digitalWrite(led, HIGH);
    }
    if (freq != 0) {
      tone(BUZZER, freq);
    }
    elapsedTick = millis();
    tick = false;
    //increase beat
    cnt = cnt + 1 < rythm * 2 ? cnt + 1 : 0;
    
  }
  
  //turn off after 50mils elapsed after tick
  if (millis() - elapsedTick >= 50) {
    digitalWrite(led, LOW);
    noTone(BUZZER);
  }
}
