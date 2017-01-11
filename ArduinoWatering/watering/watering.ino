#include <SimpleDHT.h>
#include <LiquidCrystal.h>
#include <RF24.h>
#include <SPI.h>

#define AIR_SENSOR_PIN A3
#define EARTH_SENSOR_PIN A4
#define PUMP_PIN A0
#define PUMP_ACTIVE A1
#define PUMP_ON A2

SimpleDHT11 air_sensor;
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
RF24 radio(9, 10);

unsigned long pump_on_delay;
unsigned long pump_inactive_delay;
unsigned long pump_deactivation_timestamp;
unsigned long pump_switch_on_timestamp;
unsigned long elapsed_from_deactivation;
unsigned long elapsed_from_switch_on;
byte air_temperature;
byte air_humidity;
bool earth_is_humid;
bool pump_active;
bool pump_on;

void setupRadio(){
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(0x76);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.openReadingPipe(1, 0xE8E8F0F0E1LL);
  radio.enableDynamicPayloads();
  radio.powerUp();
}

void print_air_data(){
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(air_temperature);
  lcd.print("*C H: ");
  lcd.print(air_humidity);
  lcd.print("%");
}

void print_earth_data(){
  lcd.setCursor(0, 1);
  lcd.print("Earth: ");
  if(earth_is_humid)
    lcd.print("HUMID");
  else
    lcd.print("DRY  "); //need spaces to cover previous message if earth was humid
}

void read_air_conditions(){
  air_sensor.read(AIR_SENSOR_PIN, &air_temperature, &air_humidity, NULL);
}

void read_earth_conditions(){
  earth_is_humid = !digitalRead(EARTH_SENSOR_PIN);
}

void switch_pump_on(){
  // switches pump on and saves timestamp of this action
  // deactivation to prevent reentering this function
  pump_on = true;
  pump_active = false;
  digitalWrite(PUMP_PIN, pump_on);
  pump_switch_on_timestamp = millis();
  elapsed_from_switch_on = 0;
}

void switch_pump_off_and_deactivate(){
  pump_on = false;
  pump_active = false;
  digitalWrite(PUMP_PIN, pump_on);
  pump_deactivation_timestamp = millis();
  elapsed_from_deactivation = 0;
}

void handle_pump(){
  // switch pump on only if it's active and earth is dry
  if(pump_active){
    if(!earth_is_humid){
      switch_pump_on();
    }
  }
  else{
    // switch off and deactivate pump if pump_on_delay elapsed
    if(pump_on){
      elapsed_from_switch_on = millis() - pump_switch_on_timestamp;
      if(elapsed_from_switch_on > pump_on_delay)
        switch_pump_off_and_deactivate();
    }
    else{
      elapsed_from_deactivation = millis() - pump_deactivation_timestamp;
      if(elapsed_from_deactivation > pump_inactive_delay)
        pump_active = true;
    }
  }
}

void sendState(){
  radio.stopListening();
  if(earth_is_humid){
    char text[] = "HUMID";
    radio.write(text, sizeof(text));
  }
  else{
    char text[] = "DRY";
    radio.write(text, sizeof(text));
  }
}

void handleRadio(){
  radio.startListening();
  char receivedMessage[32] = {0};
  if(radio.available()){
    radio.read(receivedMessage, sizeof(receivedMessage));
    Serial.println(receivedMessage);
    String stringMessage(receivedMessage);
    if (stringMessage == "GETSTATE"){
      sendState();
    }
    else {
      // confirm
      char *ptr;
      radio.stopListening();
      
      char num[4] = {stringMessage[7], stringMessage[8], stringMessage[9], stringMessage[10]};
      pump_on_delay = strtoul(num, &ptr, 10);
      
      num[0] = stringMessage[11];
      num[1] = stringMessage[12];
      num[2] = stringMessage[13];
      num[3] = stringMessage[14];
      pump_inactive_delay = strtoul(num, &ptr, 10);
      
      Serial.print("Pump on delay: ");
      Serial.println(pump_on_delay);
      Serial.print("Pump inactive delay: ");
      Serial.println(pump_inactive_delay);
      Serial.println(stringMessage);
      delay(500);
      radio.write("SETPUMP", sizeof("SETPUMP"));
    }
  }
  radio.startListening();
}

void setup() {
  while(!Serial)
    ;
  Serial.begin(9600);
  Serial.println("Serial ON");
  // radio
  setupRadio();
  // lcd
  lcd.begin(16, 2);
  // air
  pinMode(EARTH_SENSOR_PIN, INPUT);
  air_temperature = 0;
  air_temperature = 0;
  // pump
  pinMode(PUMP_PIN, OUTPUT);
  pump_active = true;
  pump_on = false;
  pump_deactivation_timestamp = 0;
  pump_switch_on_timestamp = 0;
  // debug
  pinMode(PUMP_ACTIVE, OUTPUT);
  pinMode(PUMP_ON, OUTPUT);
}

void loop() {
  handleRadio();
//  read_air_conditions();
//  read_earth_conditions();
//
//  print_air_data();
//  print_earth_data();
//
//  handle_pump();
//  digitalWrite(PUMP_ON, pump_on);
//  digitalWrite(PUMP_ACTIVE, pump_active);
}
