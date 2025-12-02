#include <Servo.h>
#include <EEPROM.h>
#include <LedControl.h>

const int BUTTON_PIN = 2;
const int LIGHT_INPUT = A0;
const int SERVO_RADAR_PIN  = 7; 
const int SERVO_GATE_PIN = 6;
const int TRIG_PIN =  9;
const int ECHO_PIN = 8;
const int MATRIX_CS_PIN = 11;
const int MATRIX_DIN_PIN = 12;
const int MATRIX_CLK_PIN = 10;
// ------------------------------------------------------------------------------------

enum RadarState {
  RADAR_SWEEP = 0,
  RADAR_ALERT,
  RADAR_SUN 
};
// ------------------------------------------------------------------------------------
const int LIGHT_OPEN_THRESHOLD = 600;
const int LIGHT_SAMPLE_INTERVAL = 50;
int light_sampling_cnt = 0;
int light = 0;

// ------------------------------------------------------------------------------------
volatile bool tick_1ms = false;
volatile long ms_count = 0;
volatile bool button_flag = false;
volatile long last_bnt_ms = 0;

// ------------------------------------------------------------------------------------
const long ALERT_TOGGLE_MS = 500;
int alert_count = 5;
bool alert_showing_exclam = true;
long alert_next_ms = 0;

// ------------------------------------------------------------------------------------
int rotation_dir = 1;
long last_flip_ms = 0;
long servo_next_step_ms = 0;   // when the next step is allowed (ms_count)
const long SWEEP_DURATION_MS = 1500;
// stop is between 89-94 (inclusive)
const int FS90R_STOP       = 91;
const int FS90R_SPEED_FWD  = 7;
const int FS90R_SPEED_BACK = 6;
const int SERVO_STEP_MS = 50;

const int SERVO_GATE_CLOSED_POS = 10;
const int SERVO_GATE_OPEN_POS = 170;
bool servo_gate_open = false;
// ------------------------------------------------------------------------------------
const unsigned int DIST_THRESHOLD_CM = 3;
unsigned int last_distance_cm = 0;

// ------------------------------------------------------------------------------------
bool light_override = false;

// ------------------------------------------------------------------------------------
LedControl lc = LedControl(MATRIX_DIN_PIN, MATRIX_CLK_PIN, MATRIX_CS_PIN, 1);
Servo servo_radar;
Servo servo_gate;
RadarState radar_state = RADAR_SWEEP;
// ------------------------------------------------------------------------------------
int gate_open_counter = 0;   
int light_max_value = 0;

#define OPEN_CNT_ADDR 2
#define LIGHT_MAX_ADDR 4
// ------------------------------------------------------------------------------------

const byte num1[8] = {
  B00011000,
  B00111000,
  B00011000,
  B00011000,
  B00011000,
  B00011000,
  B01111110,
  B01111110
};

const byte num2[8] = {
  B00111100,
  B01100110,
  B00000110,
  B00001100,
  B00011000,
  B00110000,
  B01111110,
  B01111110
};

const byte num3[8] = {
  B00111100,
  B01100110,
  B00000110,
  B00011100,
  B00000110,
  B01100110,
  B00111100,
  B00111100
};

const byte num4[8] = {
  B00001100,
  B00011100,
  B00101100,
  B01001100,
  B11111110,
  B11111110,
  B00001100,
  B00001100
};

const byte num5[8] = {
  B01111110,
  B01111110,
  B01100000,
  B01111100,
  B00000110,
  B01100110,
  B00111100,
  B00111100
};

const byte exclam[8] = {
  B00011000,
  B00011000,
  B00011000,
  B00011000,
  B00011000,
  B00000000,
  B00011000,
  B00011000
};

const byte sun[8] = {
  B00100100,
  B00011000,
  B01111110,
  B00111100,
  B00111100,
  B01111110,
  B00011000,
  B00100100
};

const byte moon[8] = {
  B00011110,
  B00111110,
  B01111100,
  B01111000,
  B01111000,
  B01111100,
  B00111110,
  B00011110
};

const byte* symbols[] = { num1, num2, num3, num4, num5, exclam};

inline void stop_radar_servo() {
  servo_radar.write(FS90R_STOP);
}

void setup() {
  Serial.begin(9600);

// ----------------------------------
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LIGHT_INPUT, INPUT);

// ----------------------------------
  servo_radar.attach(SERVO_RADAR_PIN);
  stop_radar_servo();
  servo_next_step_ms = 0;

  servo_gate.attach(SERVO_GATE_PIN);
  servo_gate.write(SERVO_GATE_CLOSED_POS);

// ----------------------------------
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

// ----------------------------------
  lc.shutdown(0, false);
  lc.setIntensity(0, 4);
  lc.clearDisplay(0);

// ----------------------------------
  setupTimer2_1kHz();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonISR, FALLING);
  
// ----------------------------------
  light_sampling_cnt = 0;

// ----------------------------------
  // read_from_eeprom();

}

void loop() {
  if (!tick_1ms) return;
  tick_1ms = false;

  long now = ms_count;

  if (button_flag) {
    button_flag = false;

    radar_state = RADAR_ALERT;
    stop_radar_servo();
    display_array_on_matrix(symbols[5]);
  }

  if (light_sampling_cnt > 0) {
    light_sampling_cnt--;
  } else {
    light_sampling_cnt = LIGHT_SAMPLE_INTERVAL;
    light = analogRead(LIGHT_INPUT);

    if (light > light_max_value) {
      light_max_value = light;
    }

    light_override = (light >= LIGHT_OPEN_THRESHOLD);

    Serial.print("last_distance:");
    Serial.println(last_distance_cm);
    Serial.print("light reading:");
    Serial.println(light);
    Serial.print("LIGHT MAX VALUE: ");
    Serial.println(light_max_value);
    Serial.print("GATE OPEN COUNTER: ");  
    Serial.println(gate_open_counter);
    Serial.println("********************");
  }
  
  if (light_override) {
    radar_state = RADAR_SUN;
  } else if (radar_state == RADAR_SUN) {
    radar_state = RADAR_SWEEP;
  }

  if (radar_state == RADAR_SUN) {
    stop_radar_servo();
    display_array_on_matrix(sun);
    if (!servo_gate_open){
      servo_gate.write(SERVO_GATE_OPEN_POS);
      servo_gate_open = true;
      gate_open_counter++;
    };

    return;
  }

  if (radar_state == RADAR_ALERT) {
    if (!servo_gate_open) {
      servo_gate.write(SERVO_GATE_OPEN_POS);
      servo_gate_open = true;
      gate_open_counter++;
    };

    handle_alert(now);
    update_servo_and_radar(now);
    return;
  }

  if (servo_gate_open) {
    servo_gate.write(SERVO_GATE_CLOSED_POS);
    servo_gate_open = false;
  }

  display_array_on_matrix(moon);
  update_servo_and_radar(now);
  save_to_eeprom(); 

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

void save_to_eeprom() {
  EEPROM.put(OPEN_CNT_ADDR,  gate_open_counter);
  EEPROM.put(LIGHT_MAX_ADDR, light_max_value);
}

void read_from_eeprom() {
  EEPROM.get(OPEN_CNT_ADDR,  gate_open_counter);
  EEPROM.get(LIGHT_MAX_ADDR, light_max_value);
}

void display_array_on_matrix(const byte pattern[8]) 
{
  for (byte i = 0; i < 8; i++) 
  {
    lc.setRow(0, i, pattern[i]);
  }
}

unsigned int measure_distance_cm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 20000UL);
  if (duration == 0) {
    return 500;
  }

  unsigned int distance_cm = (unsigned int)(duration / 58);
  return distance_cm;
}

void update_servo_and_radar(long now_ms) {
  if (radar_state == RADAR_ALERT || radar_state == RADAR_SUN) {
    stop_radar_servo();
    return;
  }

  if (now_ms - last_flip_ms >= SWEEP_DURATION_MS) {
    last_flip_ms = now_ms;
    rotation_dir = -rotation_dir;
  }

  int cmd;
  if (rotation_dir > 0) {
    cmd = FS90R_STOP + FS90R_SPEED_FWD;
  } else {
    cmd = FS90R_STOP - FS90R_SPEED_BACK;
  }
  servo_radar.write(cmd);

  if (now_ms >= servo_next_step_ms) {
    servo_next_step_ms = now_ms + SERVO_STEP_MS;
    last_distance_cm = measure_distance_cm();

    if (last_distance_cm <= DIST_THRESHOLD_CM) {
      radar_state = RADAR_ALERT;
      stop_radar_servo();

      alert_count = 5;
      alert_showing_exclam = true;
      alert_next_ms = now_ms;

      display_array_on_matrix(symbols[5]);
    }
  }
}

void handle_alert(long now_ms) {

  if (now_ms < alert_next_ms) return;
  alert_next_ms = now_ms + ALERT_TOGGLE_MS;

  if (alert_showing_exclam) {

    if (alert_count >= 1 && alert_count <= 5) {
      display_array_on_matrix(symbols[alert_count - 1]);
    }
    alert_showing_exclam = false;
  } else {

    display_array_on_matrix(symbols[5]);
    alert_showing_exclam = true;
    alert_count--;

    if (alert_count == 0) {
      radar_state = RADAR_SWEEP;
      alert_count = 5;
      rotation_dir = 1;
      last_flip_ms = now_ms;
      servo_next_step_ms = now_ms;
    }
  }
}