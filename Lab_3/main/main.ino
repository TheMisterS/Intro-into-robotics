#include <Servo.h>
#include <EEPROM.h>

// A, B, C, D, E, F, G, DP
const int SEGMENT_PINS[8] = {5, 6, 9, 8, 7, 4, 3, 10};

const int BUTTON_PIN = 2;
const int LIGHT_INPUT = A0;
const int SERVO_PIN  = 13;

int starting_angle = 10;
int light = 0;

volatile bool tick_1ms = false;
volatile long ms_count = 0;

volatile bool button_flag = false;
volatile long last_bnt_ms = 0;

const int LIGHT_OPEN_THRESHOLD = 600;
const int SAMPLE_MS = 400;

int remaining_Ms = 0;
int forced_open = 0;
int intervals_to_keep_open = 3;

#define OPEN_CNT_ADDR 2
#define LIGHT_MAX_ADDR 4

int light_max_value = 0;
int gate_open_counter = 0;
bool servo_is_open = false;

Servo servo;

const int DIGITS[12][8] = {
//  A B C D E F G DP
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}, // 9
  {1,0,0,0,0,0,0,0}, // off
  {1,0,0,1,0,0,1,1} // special
};

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(SEGMENT_PINS[i], OUTPUT);
    digitalWrite(SEGMENT_PINS[i], HIGH);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LIGHT_INPUT, INPUT);

  Serial.begin(9600);

  servo.attach(SERVO_PIN);
  servo.write(starting_angle);

  setupTimer2_1kHz();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonISR, FALLING);

   read_from_eeprom();

  remaining_Ms = 0;
}

void loop() {
  if (!tick_1ms) return;
  tick_1ms = false;

  if (button_flag) {
    button_flag = false;
    servo.write(180);

    if (!servo_is_open) {
      gate_open_counter++;
      servo_is_open = true;
    }

    forced_open = intervals_to_keep_open;
    remaining_Ms = SAMPLE_MS;
    display_digit(DIGITS[11]);
    return;
  }

  if (remaining_Ms > 0) {
    remaining_Ms--;
    return;
  }

  if (forced_open) {
    forced_open--;
  } else {
    light = analogRead(LIGHT_INPUT);

    if (light > light_max_value) {
      light_max_value = light;
    }

    int digit = constrain(light / 102, 0, 9);
    display_digit(DIGITS[digit]);

    if (light >= LIGHT_OPEN_THRESHOLD) {
      servo.write(180);
      if (!servo_is_open) {
        gate_open_counter++;
        servo_is_open = true;
      }
    } else {
      servo.write(starting_angle);
      servo_is_open = false;
    }
    Serial.print("light level =");
    Serial.println(light);
    Serial.print(" digit shown ="); 
    Serial.println(digit);
    Serial.print(" max light level =");
    Serial.println(light_max_value);
    Serial.print(" gate open count =");
    Serial.println(gate_open_counter);
    Serial.println("********************");
    
    save_to_eeprom();
  }

  remaining_Ms = SAMPLE_MS;
}


void setupTimer2_1kHz() {
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;

  OCR2A = 249;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22);
  TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect) {
  ms_count++;
  tick_1ms = true;
}

void onButtonISR() {
  long now = ms_count;
  if (now - last_bnt_ms < 50) return;
  last_bnt_ms = now;
  button_flag = true;
}

void display_digit(const int bits[8]) {
  for (int i = 0; i < 8; i++) {
    if (bits[i] == 1) {
      digitalWrite(SEGMENT_PINS[i], LOW);
    } else {
      digitalWrite(SEGMENT_PINS[i], HIGH);
    }
  }
}

void save_to_eeprom() {
  EEPROM.put(OPEN_CNT_ADDR,  gate_open_counter);
  EEPROM.put(LIGHT_MAX_ADDR, light_max_value);
}

void read_from_eeprom() {
  EEPROM.get(OPEN_CNT_ADDR,  gate_open_counter);
  EEPROM.get(LIGHT_MAX_ADDR, light_max_value);
}