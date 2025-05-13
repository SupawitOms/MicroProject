#include <avdweb_Switch.h>
#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// access to other device after doorlock is true
bool access = false;

// LCD address at 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// keypad
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
//Set Pins for keypad
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// setup servo
Servo ServoDoor;
Servo ServoGarage;

// setup digital doorlock
const String password = "1234";
String input = "";
bool showText = true;

// ultrasonic
long duration_G;
int distance_G;
int mode = 0;

// set pin for Buzzer and soil moisture
const int Soilsensor = A1; // set pin Soilsensorto A1
const int buzzer = 12; // set pin buzzer

// กำหนด pin ของ Ultrasonic Garage
const int TRIG_PIN_G = 10; 
const int ECHO_PIN_G = 11; 

// กำหนด pin ของ LED 4 ตัว
const int LED1 = 13; 
const int LED2 = A0; 

void setup() {

//set anything
  Serial.begin(9600);

//Servo_Door
  ServoDoor.attach(A2);  // Set pin servo (Arduino)
  ServoDoor.write(0);    

//LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello Smart Home");
  
//Buzzer
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

// Servo_Garage
  ServoGarage.attach(A3);  //(Arduino)
  ServoGarage.write(0);

// Ultrasonic_Garage
  pinMode(TRIG_PIN_G, OUTPUT);
  pinMode(ECHO_PIN_G, INPUT);
}  

void garageDoor(){

  //Garage ระบบเปิดปิดประตูโรงรถแค่ขับไปใกล้ๆ
  // ส่งคลื่น
  digitalWrite(TRIG_PIN_G, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN_G, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN_G, LOW);

  // อ่านคลื่นสะท้อน
  duration_G = pulseIn(ECHO_PIN_G, HIGH);
  distance_G = duration_G * 0.034 / 2;

  // กำหนดค่า ถ้าน้อยกว่า 10 CM ServoGarage เปิด
  if (distance_G < 10) {
    ServoGarage.write(90);
    delay(10000);
    ServoGarage.write(0);
  }

}

void moisture(){

  //Soilsensor ระบบตรวจวัดความชื้นในดินถ้าน้อยจะดังแจ้งเตือนให้รดน้ำ
  int inValue = analogRead(Soilsensor);
  Serial.println(inValue);
  if (inValue > 750) {
    digitalWrite(buzzer, LOW);
  } else {
    digitalWrite(buzzer, HIGH);
  }
  delay(1000);

}

void Mode(char key){

  // กด * เปลี่ยนMode
  if (key == '*') {
    mode++;
    if(mode > 2) mode = 0;

    lcd.clear();
    if (mode == 0) {
    // Mode : Welcome
      lcd.setCursor(0,0);
      lcd.print("Hello Smart Home");

    } else if(mode == 1){
      lcd.setCursor(0,0);
      lcd.print("Enter Password: ");
      input = "";

      }
  }

}

void digitalDoorlock(char key){

  // Door lock ระบบใส่รหัสก่อนเข้าบ้าน
  if (mode == 1 && key && key != '*') {
    if (key == '#') {
      if (input == password) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Access Granted");
        lcd.setCursor(0,1);
        lcd.print("Welcome Owner ^^");
        ServoDoor.write(90);
        access = true;
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        delay(5000);
        ServoDoor.write(0);

      } else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Access Denied !");
        lcd.setCursor(0,1);
        lcd.print("Wait Who are you?");
        delay(5000);
      }

    input = "";
    lcd.clear();
    lcd.print("Enter Password:");

    } else {
      input += key;
      lcd.setCursor(0,1);
      lcd.print(input);
      }

  }

}

void loop() {
  
  //รับตัวเลข
  char key = keypad.getKey();

  Mode(key);
  digitalDoorlock(key);

  if (access){

    moisture();
    garageDoor();
  }
}
