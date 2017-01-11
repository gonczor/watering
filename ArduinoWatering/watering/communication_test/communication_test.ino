#include <RF24.h>
#include <SPI.h>

//ce csn
RF24 radio(9, 10);
bool pinState = false;

void setup() {
  while(!Serial)
    ;
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(0x76);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.openReadingPipe(1, 0xE8E8F0F0E1LL);
  radio.enableDynamicPayloads();
  radio.powerUp();
  pinMode(8, OUTPUT);
}

void loop() {
  radio.startListening();
  Serial.println("Serial on. Listening");
  char receivedMessage[32] = {0};
  if(radio.available()){
    radio.read(receivedMessage, sizeof(receivedMessage));
    Serial.println("Received:");
    Serial.println(receivedMessage);
    Serial.println("Turning off radio");
    radio.stopListening();

    String stringMessage(receivedMessage);
    if (stringMessage == "GETSTRING"){
      Serial.println("Someone requested string");
      const char text[] = "Hello, world!";
      radio.write(text, sizeof(text));
      Serial.println("We\'ve sent message");
    }
  }

  delay(100);
}
