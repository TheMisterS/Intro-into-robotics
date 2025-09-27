// C++ code
//
#include <Keypad.h>


const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; 
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String password = "1234";
String inputCode = "";

int ledPin = 13;
int motorPin = 12;
int photoresistorPin = A0;


void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(photoresistorPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();
  int lightValue = analogRead(photoresistorPin);
  


  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    Serial.print("Light level: ");
  	Serial.println(lightValue);
    Serial.println("------------");

    if (key == '#') { 
      if ((inputCode == password) && (lightValue > 500)) {
        digitalWrite(ledPin, HIGH);
        digitalWrite(motorPin, HIGH);
        Serial.println("Access Granted");
      } else {
        digitalWrite(ledPin, LOW);
        digitalWrite(motorPin, LOW);
        Serial.println("Access Denied");
      }
      inputCode = ""; 
    } 
    else if (key == '*') { 
      inputCode = ""; 
      Serial.println("Input cleared");
      digitalWrite(ledPin, LOW);
      digitalWrite(motorPin, LOW);
    } 
    else {
      inputCode += key;
    }
  }
}