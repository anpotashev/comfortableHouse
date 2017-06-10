//Маленькая комната
#include <boarddefs.h>
//#include <IRremote.h>
//#include <IRremoteInt.h>
//#include <ir_Lego_PF_BitStreamEncoder.h>

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>


const uint8_t dev_id = 1; // 1 - м.к. 2 - б.к. 3 - кор-р 4 - настроечный 5 - реле
const uint8_t channel = 119;
const uint64_t pipe01 = 0xE8E8F0F0E2LL;
//const uint64_t pipe02 = 0xE8E8F0F0A2LL;
//const uint64_t pipe03 = 0xE8E8F0F0D1LL;
//const uint64_t pipe04 = 0xE8E8F0F0C3LL;
const uint64_t pipe05 = 0xE8E8F0F0E7LL;

RF24 radio(9,10);

//const uint8_t pir_pin = 5;
//const uint8_t ir_pin = 4;
//IRrecv irrecv(ir_pin);
//decode_results results;

struct command {
  uint8_t dev_id;
  uint8_t pir;
  unsigned long ir_command;
};

struct command cmd;

void setup() {
  cmd.dev_id = dev_id;
  Serial.begin(9600);
  radio.begin();
  printf_begin();
  radio.setChannel(channel);
  radio.setAutoAck(false);
  
//  radio.openWritingPipe(pipe05);
//  radio.startListening();
//  radio.stopListening();
  radio.openReadingPipe(1,pipe01);
  radio.openWritingPipe(pipe05);
  radio.startListening();
  
  radio.printDetails();
//  pinMode(pir_pin, INPUT);
  //irrecv.enableIrIn();
//  irrecv.enableIRIn();
}

void loop() {
  uint8_t t;
  if (radio.available()) {
    Serial.println("available");
//    while(radio.available()){
//      Serial.println("Here");
      radio.read(&cmd, sizeof(cmd) ); 
//    }
//    Serial.println(t);
    get_command();
//  } else {
//    radio.stopListening();
//    radio.startListening();
  }
}

void get_command() {
  Serial.println("getting command:");
  Serial.print("dev_id: ");
  Serial.println(cmd.dev_id, HEX);
  Serial.print("pir: ");
  Serial.println(cmd.pir, HEX);
  Serial.print("ir_command: ");
  Serial.println(cmd.ir_command, HEX);
  //radio.write(&cmd,sizeof(cmd));
}
