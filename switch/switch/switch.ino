#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
//#define DEBUG_
const uint8_t dev_id = 1; // 1 - м.к. 2 - б.к. 3 - кор-р 4 - настроечный 5 - реле
const uint8_t channel = 119;
const uint64_t pipe01 = 0xE8E8F0F0E2LL;
const uint64_t pipe02 = 0xE8E8F0F0A2LL;
const uint64_t pipe03 = 0xE8E8F0F0D1LL;
const uint64_t pipe04 = 0xE8E8F0F0C3LL;
const uint64_t pipe05 = 0xE8E8F0F0E7LL;
const uint8_t lp = 3;
const uint8_t cp = 2;
const uint8_t dp = 4;
uint16_t state = 65535;
RF24 radio(9,10);
struct command {
  uint8_t dev_id;
  uint8_t pir;
  unsigned long ir_command;
} cmd;
struct switch_ {
  uint8_t switch_id;
  uint8_t digital_id;
  uint16_t analog_value;
  uint16_t analog_timeout;
  unsigned long ir_command;
  uint16_t ir_timeout;
  char name[3];
} switches[20];
struct switch_state {
  uint8_t switch_id;
  unsigned long on_delay;
  unsigned long start_on;
  bool on;
} switch_states[16];
struct setting_command {
  uint8_t cmd_id;
  uint8_t switch_count;
  uint8_t array_index;
  switch_ switch_value;
} setting_cmd;
struct ad_command {
  uint8_t dev_id;
  uint16_t analog_value;
} ad_cmd;
uint8_t switch_count;
void setup() {
  pinMode(lp, OUTPUT);
  pinMode(dp, OUTPUT);
  pinMode(cp, OUTPUT);
  switch_count = EEPROM.read(1);
//  EEPROM.write(1,10);
//  switch_count = 10;
  readValues();
  radio.begin();
  radio.setChannel(channel);
  radio.setAutoAck(false);
  radio.openWritingPipe(pipe05);
  radio.openReadingPipe(1,pipe01);
  radio.openReadingPipe(2,pipe02);
  radio.openReadingPipe(3,pipe03);
  radio.openReadingPipe(4,pipe04);
  radio.startListening();
  
  #if defined(DEBUG_)
    Serial.begin(57600); 
    Serial.println("Starting...");
    printf_begin();
    radio.printDetails();
  #endif

  for (uint8_t i = 0; i < 16; i++) {
    switch_states[i].switch_id = i;
    switch_states[i].on_delay = 0L;
    switch_states[i].on = false;
  }
  registerWrite(1, HIGH);
}
void loop() {
  // put your main code here, to run repeatedly:
  uint8_t pipe;
  if (radio.available(&pipe)) {
    switch (pipe) {
      case 1:
        pipe1();
        break;
      case 2:
        pipe2();
        break;
      case 3:
        pipe3();
        break;
      case 4:
        pipe4();
        break;
      default:
        break;
    }
  }

  check_switches();
}
void check_switches() {
  for (uint8_t i = 0; i < 16; i++) {
    if ((switch_states[i].on) && (millis() - switch_states[i].start_on > switch_states[i].on_delay)) {
      off(&switch_states[i]);
    }
  }
}
void off(switch_state *sw_state) {
  offPin(sw_state->switch_id);
  sw_state->on = false;
}
void on(switch_state *sw_state, uint16_t seconds) {
  onPin(sw_state->switch_id);
  sw_state->on = true;
  sw_state->on_delay = seconds*1000L;
  sw_state->start_on = millis();
}
void pipe1() {
  // маленькая комната
  radio.read(&cmd, sizeof(cmd));
  if (cmd.pir == 1) { // если есть движение
    for (uint8_t i = 0; i < switch_count; i++) { // перебираем все switch_
      if (switches[i].digital_id == cmd.dev_id) { // если где-то совпадает digital_id и dev_id из команды,
        if (switch_states[switches[i].switch_id].on) { // и если соответсвующее реле включено, то
          switch_states[switches[i].switch_id].start_on = millis(); // устанавливаем время включения на текущее
        }
      }
    }
  } else {
    for (uint8_t i = 0; i < switch_count; i++) { // перебираем все switch_
      if (switches[i].ir_command == cmd.ir_command) { // если где-то прописана полученная ir-команда
        for (uint8_t j = 0; j < 16; j++) {
          if (switch_states[j].switch_id == switches[i].switch_id) {
            if (switch_states[j].on) {
              off(&switch_states[j]);
            } else {
              on(&switch_states[j], switches[i].ir_timeout);
            }
          }
        }
      }
    }
  }
}
void pipe2() {
  pipe1();
}
void pipe3() {
  radio.read(&ad_cmd, sizeof(ad_cmd));
  for (uint8_t i = 0; i < switch_count; i++) { // перебираем все switch_
    if (switches[i].digital_id == ad_cmd.dev_id) { // если где-то совпадает digital_id и dev_id из команды,
      if (switch_states[switches[i].switch_id].on) { // и если соответсвующее реле включено, то
          switch_states[switches[i].switch_id].start_on = millis(); // устанавливаем время включения на текущее
      } else {
        for (uint8_t j = 0; j < 16; j++) {
          if (switch_states[j].switch_id == switches[i].switch_id) {
            if (switches[i].analog_value>ad_cmd.analog_value) {
              on(&switch_states[j], switches[i].analog_timeout);
            }
          }
        }
      }
    }
  }
}
void pipe4() {
  radio.read(&setting_cmd, sizeof(setting_cmd));
        if (setting_cmd.cmd_id == 0) {
          //send switch_count
          radio.stopListening();
          setting_cmd.switch_count = switch_count;
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        if (setting_cmd.cmd_id == 1) {
          //set switch_count
          switch_count = setting_cmd.switch_count;
          writeSwitchCount();
          radio.stopListening();
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        if (setting_cmd.cmd_id == 2) {
          //send switches[array_index]
          radio.stopListening();
          setting_cmd.switch_value = switches[setting_cmd.array_index];
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        if (setting_cmd.cmd_id == 3) {
          //set switches[array_index]
          switches[setting_cmd.array_index] = setting_cmd.switch_value;
          writeValues();
          radio.stopListening();
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        if (setting_cmd.cmd_id == 4) {
          onPin(setting_cmd.switch_count);
          
          radio.stopListening();
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        if (setting_cmd.cmd_id == 5) {
          offPin(setting_cmd.switch_count);
          
          radio.stopListening();
          radio.write(&setting_cmd, sizeof(setting_cmd));
          radio.startListening();
        }
        //len = radio.getDynamicPayloadSize();
        //if len = 
        //byte
}
void readValues() {
  EEPROM.readBlock<switch_>(2, switches, 20);
}
void writeValues() {
  EEPROM.writeBlock<switch_>(2, switches, 20);
}
void writeSwitchCount() {
  EEPROM.write(1, switch_count);
}
void onPin(int whichPin) {
  registerWrite(whichPin, LOW);
}
void offPin(int whichPin) {
  registerWrite(whichPin, HIGH);
}
void registerWrite(int whichPin, int whichState) {
  digitalWrite(lp, LOW);
  bitWrite(state, whichPin, whichState);
 
  byte registerOne = highByte(state);
  byte registerTwo = lowByte(state);
 
  // "проталкиваем" байты в регистры
  shiftOut(dp, cp, MSBFIRST, registerTwo);
  shiftOut(dp, cp, MSBFIRST, registerOne);
  digitalWrite(lp, HIGH);
}


