#ifdef esp32

  #define RPM_POT_PIN 34
  #define CRANK_N_PIN 25
  #define CRANK_P_PIN 33

  #define CLK_DEBUG 30

  #define MAX_ADC 4095

// ARDUINO MEGA:
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

  #define RPM_POT_PIN A15
  #define CRANK_N_PIN 49
  #define CRANK_P_PIN 51

  #define CLK_DEBUG 31

  #define MAX_ADC 1024
  
#else

  #define RPM_POT_PIN A15
  #define CRANK_N_PIN 49
  #define CRANK_P_PIN 51

  #define MAX_ADC 1024

#endif

#define MAX_RPM 8000            // in rpm
#define RPM_POT_REFRESH 1000   // in ms

#define CRANK_TEETH 60
#define CRANK_MISSING_TEETH 2

#define N_PIN_STATUS HIGH

PGM_P const FIRMWARE_NAME PROGMEM = "CrankSensEmu";
PGM_P const FIRMWARE_VER PROGMEM = "0.1";

uint32_t rpm_setpoint = 0;
uint32_t rpm_pot_sample_time = 0;

uint32_t tooth_time = 0;
uint32_t crank_sample_time = 0;
uint8_t crank_teeth_i = 0;
bool crank_out = LOW;
bool debug_clk = LOW;

void setup() {
  Serial.begin(115200);                             // USB
  
  Serial.print(F("\n\n\n"));                        //  Print firmware name and version const string
  Serial.print(FIRMWARE_NAME);
  Serial.print(F(" - "));
  Serial.println(FIRMWARE_VER);

  pinMode(CRANK_N_PIN, OUTPUT);
  pinMode(CRANK_P_PIN, OUTPUT);

  pinMode(CLK_DEBUG, OUTPUT);

  digitalWrite(CRANK_N_PIN, N_PIN_STATUS);
  digitalWrite(CRANK_P_PIN, LOW);

  digitalWrite(CLK_DEBUG, debug_clk);

  refreshRpmPot();
  crank_sample_time = micros();
}

void loop() {
  
  if(micros() > crank_sample_time){
    //Serial.println(crank_sample_time);
    if (tooth_time != 0xFFFFFFFF) {
      refreshCrank();
      crank_sample_time = crank_sample_time + tooth_time;
    }
  }
  
  if (millis() > rpm_pot_sample_time){
    refreshRpmPot();
    rpm_pot_sample_time = rpm_pot_sample_time + RPM_POT_REFRESH;
  }
}

void refreshCrank(){

  if(tooth_time == 0) return;

  if (crank_teeth_i < (CRANK_TEETH - CRANK_MISSING_TEETH) * 2){
    crank_out = !crank_out;
    digitalWrite(CRANK_P_PIN, crank_out);
  }

  crank_teeth_i = crank_teeth_i + 1;
  if (crank_teeth_i > CRANK_TEETH * 2) crank_teeth_i = 0;
  
  debug_clk = !debug_clk;
  digitalWrite(CLK_DEBUG, debug_clk);
}

void refreshRpmPot(){
  uint16_t raw_adc = analogRead(RPM_POT_PIN);

  Serial.print(raw_adc);
  Serial.print("\t");
  
  rpm_setpoint = map(raw_adc, 0, MAX_ADC, 0, MAX_RPM);    // map(value, fromLow, fromHigh, toLow, toHigh)

  Serial.print(rpm_setpoint);
  
  tooth_time = calcToothTime(rpm_setpoint);

  Serial.print("\t");
  Serial.println(tooth_time);
}

uint32_t calcToothTime(uint32_t rpm){
  // Each engine revolution has CRANK_TEETH
  // Change rpm (minutes) to us
  return (60.0 * 1000.0 * 1000.0) / (rpm * CRANK_TEETH * 2);
}
