//Маленькая комната
#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
//#include <ir_Lego_PF_BitStreamEncoder.h>

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>


const uint8_t dev_id = 1; // 1 - м.к. 2 - б.к. 3 - кор-р 4 - настроечный 5 - реле
const uint8_t channel = 119;
//const uint64_t pipe01 = 0xE8E8F0F0E2LL;
//const uint64_t pipe02 = 0xE8E8F0F0A2LL;
//const uint64_t pipe03 = 0xE8E8F0F0D1LL;
const uint64_t pipe04 = 0xE8E8F0F0C3LL;
const uint64_t pipe05 = 0xE8E8F0F0E7LL;

RF24 radio(9,10);

//const uint8_t pir_pin = 5;
const uint8_t ir_pin = 8;
IRrecv irrecv(ir_pin);
decode_results results;

struct command {
  uint8_t dev_id;
  uint8_t pir;
  unsigned long ir_command;
};

struct command cmd;

struct switch_ {
  uint8_t switch_id;
  uint8_t digital_id;
  uint16_t analog_value;
  uint16_t analog_timeout;
  unsigned long ir_command;
  uint16_t ir_timeout;
  char name[3];
};
//switch_ switches[20];

struct setting_command {
  uint8_t cmd_id;
  uint8_t switch_count;
  uint8_t array_index;
  switch_ switch_value;
};

setting_command setting_cmd;

enum menuposition {MAIN, SET_SWITCH_COUNT, GET_SWITCH_DATA,
SET_SWITCH_DATA, CHANGE_SW_DATA, ON_PIN, OFF_PIN };
menuposition menu = MAIN;

int changing_position;
void setup() {
  cmd.dev_id = dev_id;
  Serial.begin(9600);
  radio.begin();
  printf_begin();
  radio.setChannel(channel);
  radio.setAutoAck(false);
  radio.openReadingPipe(1,pipe05);
  radio.openWritingPipe(pipe04);
  radio.startListening();
  radio.printDetails();
  radio.stopListening();
//  setting_cmd.cmd_id = 0;
//  radio.write(&setting_cmd, sizeof(setting_cmd));
//  radio.startListening();
  irrecv.enableIRIn();
  printMenu();
}

void loop() {
  mainMenu();
//  uint8_t t;
//  if (radio.available()) {
//    Serial.println("available");
////    while(radio.available()){
////      Serial.println("Here");
//      radio.read(&cmd, sizeof(cmd) ); 
////    }
////    Serial.println(t);
//    get_command();
////  } else {
////    radio.stopListening();
////    radio.startListening();
//  }
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

void printMenu() {
  Serial.println("------------");
  if (menu == MAIN) {
    Serial.println("1. Get switch's count");
    Serial.println("2. Set switch's count");
    Serial.println("3. Get n-switch data");
    Serial.println("4. Set n-switch data");
    Serial.println("5. On pin");
    Serial.println("6. Off pin");
  }    
  if ((menu == ON_PIN) || (menu == OFF_PIN)) {
    Serial.println("Enter pin to on|off (0-15)");
    Serial.println("99. Return to main menu");
  }
  if (menu == SET_SWITCH_COUNT) {
    Serial.println("Ener new switch's count (0-20)");
    Serial.println("99. Return to main menu");
  }
  if (menu == GET_SWITCH_DATA) {
    Serial.println("Enter switch's array num (0-19)");
    Serial.println("99. Return to main menu");
  }
  if (menu == SET_SWITCH_DATA) {
    Serial.println("Enter switch's array num (0-19)");
    Serial.println("99. Return to main menu");
  }
  if (menu == CHANGE_SW_DATA) {
    Serial.println("Current values: ");
    printSwitch();
    Serial.println("Enter new value");
    Serial.println("1. switch_id");
    Serial.println("2. digital_id");
    Serial.println("3. analog_value");
    Serial.println("4. analog_timeout");
    Serial.println("5. ir_command");
    Serial.println("6. ir_timeout");
    Serial.println("7. name");
    Serial.println("0. Write");
    Serial.println("99. Return to main menu");
  }
}

void mainMenu() {
  while (menu == MAIN) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      switch (cmd) {
      case 1: 
        setting_cmd.cmd_id = 0;
        sendCmd();
        Serial.print("Switch's count = ");
        Serial.println(setting_cmd.switch_count);
//        printMenu();
        break;
      case 2:
        menu = SET_SWITCH_COUNT;
        printMenu();
        break;
      case 3:
        menu = GET_SWITCH_DATA;
        printMenu();
        break;
      case 4:
        menu = SET_SWITCH_DATA;
        printMenu();
        break;
      case 5:
        menu = ON_PIN;
        printMenu();
        break;
      case 6:
        menu = OFF_PIN;
        printMenu();
        break;
      default:
        break;
    }
  }
  }
  while (menu == SET_SWITCH_COUNT) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      if (cmd == 99) {
        menu = MAIN;
        printMenu();
      } else {
        if ((cmd >= 0) && (cmd <=20)) {
          setting_cmd.cmd_id = 1;
          setting_cmd.switch_count = cmd;
          sendCmd();
          menu = MAIN;
          printMenu();
        } else {
          Serial.println("Enter value 0-20 or 99");
        }
      }
    }
  }
  while (menu == GET_SWITCH_DATA) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      if (cmd == 99) {
        menu = MAIN;
        printMenu();
      } else {
        if ((cmd >= 0) && (cmd <=19)) {
          setting_cmd.cmd_id = 2;
          setting_cmd.array_index = cmd;
          sendCmd();
          printSwitch();
          menu = MAIN;
          printMenu();
        } else {
          Serial.println("Enter value 0-19 or 99");
        }
      }
    }
  }
  while (menu == SET_SWITCH_DATA) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      if (cmd == 99) {
        menu = MAIN;
        printMenu();
      } else {
        if ((cmd >= 0) && (cmd <=19)) {
          setting_cmd.cmd_id = 2;
          setting_cmd.array_index = cmd;
          sendCmd();
          menu = CHANGE_SW_DATA;
          printMenu();
        } else {
          Serial.println("Enter value 0-19 or 99");
        }
      }
    }
  }
  while ((menu == ON_PIN) || (menu == OFF_PIN)) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      if (cmd == 99) {
        menu = MAIN;
        printMenu();
      } else {
        if ((cmd >= 0) && (cmd < 16)) {
          setting_cmd.switch_count = cmd;
          if (menu == ON_PIN) {
            setting_cmd.cmd_id = 4;
          } else {
            setting_cmd.cmd_id = 5;
          }
          sendCmd();
          menu = MAIN;
          printMenu();
        }
      }
    }
  }
  while (menu == CHANGE_SW_DATA) {
    if (Serial.available()) {
      int cmd = Serial.parseInt();
      if (cmd == 99) {
        menu = MAIN;
        printMenu();
      } else {
        if (cmd == 0) {
          setting_cmd.cmd_id = 3;
          setting_cmd.switch_count = cmd;
          sendCmd();
          menu = MAIN;
          printMenu();
        } else {
          if ((cmd > 0) && (cmd <=7)) {
            Serial.println("Changing..");
            changing_position = cmd;
            changeValue();
            printMenu();
          } else {
            Serial.println("Enter value 0-7 or 99");
          }
        }
      }
    }
  }
}

void sendCmd() {
  radio.stopListening();
  radio.write(&setting_cmd, sizeof(setting_cmd));
  radio.startListening();
  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
  while ( ! radio.available() ){                             // While nothing is received
    if (micros() - started_waiting_at > 2000000 ){            // If waited longer than 2000ms, indicate timeout and exit while loop
      timeout = true;
    break;
    }      
  }
  if ( timeout ){                                             // Describe the results
      Serial.println(F("Failed, response timed out."));
  }else{
    radio.read( &setting_cmd, sizeof(setting_cmd) );
  }
}

void printSwitch() {
  switch_ sw = setting_cmd.switch_value;
  Serial.print("switch_id: ");
  Serial.println(sw.switch_id);
  Serial.print("digital_id: ");
  Serial.println(sw.digital_id);
  Serial.print("analog_value: ");
  Serial.println(sw.analog_value);
  Serial.print("analog_timeout: ");
  Serial.println(sw.analog_timeout);
  Serial.print("ir_command: 0x");
  Serial.println(sw.ir_command, HEX);
  Serial.print("ir_timeout: ");
  Serial.println(sw.ir_timeout);
  Serial.print("name: ");
//  Serial.print(sw.name[0]);
//  Serial.print(sw.name[1]);
  for (uint8_t i = 0; i<3; i++)
    Serial.print(sw.name[i]);
  Serial.println();  
}

void changeValue() {
  Serial.println("Changing value. Enter new value or press key on ir remote");
  bool done_ = false;
  switch (changing_position) {
    
    case 1:
      Serial.println("Enter value switch_id: 0 - 19");
      while (!done_) {
        if (Serial.available()) {
          uint8_t value = Serial.parseInt();
          if (value < 20) {
            done_ = true;
            setting_cmd.switch_value.switch_id = value;
          }
        }
      }
      break;
//    Serial.println("1. switch_id");
    case 2:
      Serial.println("Enter value digital_id: 0 - 255");
      while (!done_) {
        if (Serial.available()) {
          uint8_t value = Serial.parseInt();
          done_ = true;
          setting_cmd.switch_value.digital_id = value;
        }
      }
      break;
//    Serial.println("2. digital_id");
    case 3:
      Serial.println("Enter value analog_value");
      while (!done_) {
        if (Serial.available()) {
          uint16_t value = Serial.parseInt();
          done_ = true;
          setting_cmd.switch_value.analog_value = value;
        }
      }
      break;
//    Serial.println("3. analog_value");
    case 4:
      Serial.println("Enter value analog_timeout");
      while (!done_) {
        if (Serial.available()) {
          uint16_t value = Serial.parseInt();
          done_ = true;
          setting_cmd.switch_value.analog_timeout = value;
        }
      }
      break;
//    Serial.println("4. analog_timeout");
    case 5:
      Serial.println("Press key on ir-remote");
      while (!done_) {
        if (irrecv.decode(&results)) {
          Serial.println(results.value, HEX);
          setting_cmd.switch_value.ir_command = results.value;
    //Serial.println(results.value, HEX);
          irrecv.resume();
          done_ = true;
        }
      }
      break;
//    Serial.println("5. ir_command");
    case 6:
      Serial.println("Enter value ir_timeout");
      while (!done_) {
        if (Serial.available()) {
          uint16_t value = Serial.parseInt();
          done_ = true;
          setting_cmd.switch_value.ir_timeout = value;
        }
      }
      //    Serial.println("6. ir_timeout");
    case 7:
      Serial.println("Enter value name: 3 char");
      while (!done_) {
        if (Serial.available()>=3) {
          for (byte i=0; i<3; i++){
            setting_cmd.switch_value.name[i] = Serial.read();
          }
          done_ = true;
          
        }
      }
      break;
//    Serial.println("7. name");  
    default:
      break;
  }
}

