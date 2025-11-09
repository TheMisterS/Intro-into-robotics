#include <EEPROM.h>
#include <LedControl.h>

const int SPEAKER_PIN = 12;
const int BUTTON_PIN = 2;
const int DIN_PIN = 8;
const int CS_PIN  = 7;
const int CLK_PIN = 13;

volatile bool tick_1ms = false;
volatile long ms_count = 0;

int notes_Hz[4] = {262, 294, 330, 600};  
int note_LenMs[4] = {400, 400, 400, 1000}; 
int note_cnts[4] = {0, 0, 0, 0};
int gap_ms = 300;                    

int note_index = 0;      
int remaining_Ms = 0;    
int in_gap = 0;

volatile bool button_flag = false;
volatile long last_bnt_ms = 0; 

#define NOTE_ONE_ADDR 2
#define NOTE_TWO_ADDR 4
#define NOTE_THREE_ADDR 6
#define NOTE_FOUR_ADDR 8

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

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

const byte letterG[8] = {
  B00111100,
  B01100110,
  B11000000,
  B11011110,
  B11000110,
  B01100110,
  B00111100,
  B00111100
};

const byte* symbols[] = { num1, num2, num3, num4, letterG };

void setup() {
  Serial.begin(9600);
  
   read_cnts_from_eeprom();

  lc.shutdown(0, false);
  lc.setIntensity(0, 2);
  lc.clearDisplay(0);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setupTimer1_1kHz();

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonISR, FALLING);
  startNote(0);
}

void loop() {
  if (!tick_1ms) return;
  tick_1ms = false;

  if (button_flag) 
  {
    button_flag = false;

    display_array_on_matrix(symbols[3]);
    note_cnts[3]++;
    tone(SPEAKER_PIN, notes_Hz[3]); 
    remaining_Ms = note_LenMs[3];
    in_gap = 0;
  }

  if (remaining_Ms > 0) 
  {
    remaining_Ms--;     
    return;
  }

  if (in_gap == 0)
  {
    startGap();       
  } else {
    note_index++;         
    if (note_index >= 3) 
    {
      note_index = 0;     
      save_cnts_to_eeprom();
    }
    startNote(note_index);
  }
}

ISR(TIMER1_COMPA_vect)
{
  ms_count++;
  tick_1ms = true;
}


void setupTimer1_1kHz() {

  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 0;

  OCR1A = 249;

  TCCR1B |= (1 << WGM12);
  
  TCCR1B |= (1 << CS11) | (1 << CS10);

  TIMSK1 |= (1 << OCIE1A);
}

void startNote(int idx)
{
  display_array_on_matrix(symbols[idx]);
  note_cnts[idx]++;
  tone(SPEAKER_PIN, notes_Hz[idx]);
  remaining_Ms = note_LenMs[idx];  
  in_gap = 0;                      
}

void startGap() 
{
  noTone(SPEAKER_PIN);                 
  remaining_Ms = gap_ms;               
  in_gap = 1;                       
}

void onButtonISR() 
{
  long now = ms_count;        
  if (now - last_bnt_ms < 50) return;
  last_bnt_ms = now;
  button_flag = true;                
}

void display_array_on_matrix(const byte pattern[8]) 
{
  for (byte i = 0; i < 8; i++) 
  {
    lc.setRow(0, i, pattern[i]);
  }
}

void save_cnts_to_eeprom() 
{
  int addr = 2;

  for (int i = 0; i < 4; i++) 
  {
    EEPROM.put(addr, note_cnts[i]);
    Serial.print("Saved note ");
    Serial.print(i + 1);
    Serial.print(" count ");
    Serial.println(note_cnts[i]);
    addr += 2;
  }

  Serial.println("************************************");
}

void read_cnts_from_eeprom() 
{
  int addr = 2;

  for (int i = 0; i < 4; i++) 
  {
    EEPROM.get(addr, note_cnts[i]);
    addr += 2;
  }

  Serial.println("Note counts succesfully read from EEPROM!");
}