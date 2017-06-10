//Маленькая комната
#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

//#define DEBUG_

const uint8_t dev_id = 1; // 1 - м.к. 2 - б.к. 3 - кор-р 4 - настроечный 5 - реле
const uint8_t channel = 119;
const uint64_t pipe01 = 0xE8E8F0F0E2LL;
//const uint64_t pipe02 = 0xE8E8F0F0A2LL;
//const uint64_t pipe03 = 0xE8E8F0F0D1LL;
//const uint64_t pipe04 = 0xE8E8F0F0C3LL;
const uint64_t pipe05 = 0xE8E8F0F0E7LL;

RF24 radio(7,8);

const uint8_t pir_pin = 5;
const uint8_t ir_pin = 4;
IRrecv irrecv(ir_pin);
decode_results results;

struct command {
  uint8_t dev_id;
  uint8_t pir;
  unsigned long ir_command;
};

struct command cmd;

void setup() {
  cmd.dev_id = dev_id;
  #if defined(DEBUG_)  
    Serial.begin(9600); 
    printf_begin();
  #endif
  radio.begin();
  radio.setChannel(channel);
  radio.setAutoAck(false);
  radio.openWritingPipe(pipe01);
  radio.startListening();
  
  #if defined(DEBUG_)
    radio.printDetails();
  #endif
  radio.stopListening();
  pinMode(pir_pin, INPUT);
  //irrecv.enableIrIn();
  irrecv.enableIRIn();
}

void loop() {
  if (digitalRead(pir_pin) == HIGH) {
    //Serial.write("Motion detected\n");
    cmd.pir = 1;
    cmd.ir_command = 0L;
    send_command();
//    delay(100);
  }
  if (irrecv.decode(&results)) {
    
    //Serial.println(results.value, HEX);
    irrecv.resume();
    cmd.pir = 0;
    cmd.ir_command = results.value;
    if (results.value != 4294967295L)
      send_command();
  }
}

void send_command() {
  #if defined(DEBUG_) 
    Serial.println("sending command:");
    Serial.print("dev_id: ");
    Serial.println(cmd.dev_id, HEX);
    Serial.print("pir: ");
    Serial.println(cmd.pir, HEX);
    Serial.print("ir_command: ");
    Serial.println(cmd.ir_command, HEX);
  #endif
  radio.write(&cmd,sizeof(cmd));
//  radio.startListening();
//  radio.stopListening();

}
