//аналогово-цифровые датчики
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

//#define DEBUG_

//const uint8_t dev_id = 3; // 1 - м.к. 2 - б.к. 3 - кор-р 4 - настроечный 5 - реле
const uint8_t channel = 119;
//const uint64_t pipe01 = 0xE8E8F0F0E2LL;
//const uint64_t pipe02 = 0xE8E8F0F0A2LL;
const uint64_t pipe03 = 0xE8E8F0F0D1LL;
//const uint64_t pipe04 = 0xE8E8F0F0C3LL;
const uint64_t pipe05 = 0xE8E8F0F0E7LL;
RF24 radio(9,10);
struct ad_command {
  uint8_t dev_id;
  uint16_t analog_value;
} ad_cmd;
struct ad_pin {
  uint8_t dev_id;
  uint8_t d_pin;
  uint8_t a_pin;
} ad_pins[4] = {
  {3, 2, A3},
  {4, 3, A2},
  {5, 4, A1},
  {6, 5, A0}
  };

void setup() {
  //cmd.dev_id = dev_id;
  #if defined(DEBUG_)  
    Serial.begin(9600); 
    printf_begin();
  #endif
  radio.begin();
  radio.setChannel(channel);
  radio.setAutoAck(false);
  radio.openWritingPipe(pipe03);
  radio.startListening();
  
  #if defined(DEBUG_)
    radio.printDetails();
  #endif
  radio.stopListening();

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(ad_pins[i].d_pin, INPUT);
    pinMode(ad_pins[i].a_pin, INPUT);
  }
}

void loop() {
  
  for (uint8_t i = 0; i < 4; i++) {
    if (digitalRead(ad_pins[i].d_pin)) {
      ad_cmd.dev_id = ad_pins[i].dev_id;
      ad_cmd.analog_value = analogRead(ad_pins[i].a_pin);
      send_command();
    }
  }
//  if (digitalRead(pir_pin) == HIGH) {
//    //Serial.write("Motion detected\n");
//    cmd.pir = 1;
//    cmd.ir_command = 0L;
//    send_command();
////    delay(100);
//  }
//  if (irrecv.decode(&results)) {
//    
//    //Serial.println(results.value, HEX);
//    irrecv.resume();
//    cmd.pir = 0;
//    cmd.ir_command = results.value;
//    if (results.value != 4294967295L)
//      send_command();
//  }
}

void send_command() {
  #if defined(DEBUG_) 
    Serial.println("sending command:");
    Serial.print("dev_id: ");
    Serial.println(ad_cmd.dev_id, HEX);
    Serial.print("analog_value: ");
    Serial.println(ad_cmd.analog_value);
  #endif
  radio.write(&ad_cmd,sizeof(ad_cmd));
}
