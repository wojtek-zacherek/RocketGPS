#include <LiquidCrystal.h>
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

uint8_t command[] = {0x7E, 0x00, 0x1E, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x41, 0xD3, 0x60, 0xF2, 0xFF, 0xFE, 0x00, 0x00, 0x49, 0x27, 0x6D, 0x20, 0x53, 0x74, 0x69, 0x6C, 0x6C, 0x20, 0x41, 0x6C, 0x69, 0x76, 0x65, 0x21, 0x9F};
uint8_t checksum = 0;
bool sending = false;

enum receiveState{
  _start,
  _length,
  _ftype,
  _64baddr,
  _16baddr,
  _recvopt,
  _data,
  _checksum,
};
receiveState currentState = _start;


// 1,2,1,8,2,1,-1,1
struct packet {
  uint8_t startDelimeter;
  uint16_t len;
  uint8_t ftype;
  uint8_t srcaddr64[8];
  uint8_t srcaddr16[2];
  uint8_t recvopt;
  uint16_t dataPos;
  uint8_t packetData[1024];
  uint8_t checksum;
  uint8_t bytesReceived;
};
typedef struct packet Packet;
Packet dataPacket;

void printStatus(){
  Serial.print("  Status - ");
  Serial.print(dataPacket.startDelimeter,HEX);
  Serial.print(' ');
  Serial.print(dataPacket.len,HEX);
  Serial.print(' ');
  Serial.print(dataPacket.ftype,HEX);
  Serial.print(' ');
  Serial.print(dataPacket.recvopt,HEX);
  Serial.print(' ');
  Serial.print(dataPacket.dataPos,HEX);
  Serial.print(' ');
  Serial.print(dataPacket.checksum,HEX);
  Serial.println();
}

const int buttonPin = 2;
int disp = 0;
int buttonState = 0;
int buttonStatePrev = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), sw, CHANGE);
  
  if (sending){
    
  }else{
    dataPacket.startDelimeter = 0x7E;
    dataPacket.len = 0x00;
    dataPacket.ftype = 0x90;
    dataPacket.recvopt = 0xC1;
    dataPacket.dataPos = 0;
    dataPacket.checksum = 0;
    printStatus();
  }
  
  lcd.begin(16, 2);
  lcd.print("Hello World!");
  delay(1000);
  
}

void sw(){
  buttonState = digitalRead(buttonPin);
  if(buttonState != buttonStatePrev){
    if(buttonState == HIGH){
      disp++;
      if(disp >= 2){
        disp = 0;
      }
    }
  }
}

void loop() {
  if(sending){
    for (int i = 0; i < (sizeof(command) / sizeof(command[0])); i++){
      Serial.print(command[i]);
    }
    Serial.write(command,(sizeof(command) / sizeof(command[0])));
    delay(1000);
  }else{

//    switch(currentState){
//      case _start:
//        if (Serial.available()){
//          
//        }
//        break;
//    }
//    
//    if (Serial.available()){
//      char inByte = Serial.read();
//      Serial.print(inByte);
//    }
  }
}

void displayData(){
  long lonn = (long)(((long)dataPacket.packetData[0] << 24) | ((long)dataPacket.packetData[1] << 16) | ((long)dataPacket.packetData[2] << 8) | ((long)dataPacket.packetData[3]));
  long latt = (long)(((long)dataPacket.packetData[4] << 24) | ((long)dataPacket.packetData[5] << 16) | ((long)dataPacket.packetData[6] << 8) | ((long)dataPacket.packetData[7]));
  long alt = (long)(((long)dataPacket.packetData[8] << 24) | ((long)dataPacket.packetData[9] << 16) | ((long)dataPacket.packetData[10] << 8) | ((long)dataPacket.packetData[11]));
  long siv = (long)(((long)dataPacket.packetData[12] << 24) | ((long)dataPacket.packetData[13] << 16) | ((long)dataPacket.packetData[14] << 8) | ((long)dataPacket.packetData[15]));

  if(disp == 0){
    lcd.setCursor(0, 0);
    // print the number of seconds since reset:
    lcd.print(abs(latt/10000000));
    lcd.print('.');
    lcd.print(abs((latt%10000000)/10000));
    if(latt > 0){
      lcd.print('N');
    }else{
      lcd.print('S');
    }
    lcd.print(' ');
    lcd.print(abs(lonn/10000000));
    lcd.print('.');
    lcd.print(abs((lonn%10000000)/10000));
    if(lonn > 0){
      lcd.print('E');
    }else{
      lcd.print('W');
    }
    lcd.print("  ");
  
    lcd.setCursor(0,1);
    lcd.print("alt: ");
    lcd.print((alt/1000));
    lcd.print(".");
    lcd.print(abs(alt%1000/10));
    lcd.print("m   ");
  }else{
    lcd.setCursor(0, 0);
    // print the number of seconds since reset:
    lcd.print(abs(latt/10000000));
    lcd.print('.');
    lcd.print(abs((latt%10000000)));
    if(latt > 0){
      lcd.print('N');
    }else{
      lcd.print('S');
    }
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print(abs(lonn/10000000));
    lcd.print('.');
    lcd.print(abs((lonn%10000000)));
    if(lonn > 0){
      lcd.print('E');
    }else{
      lcd.print('W');
    }
    lcd.print("        ");
  }
  
}

void stateMachine(){
  uint8_t value;
  value = Serial.read();
  switch(currentState){
    case _start:
//      value = Serial.read();
      if(value == dataPacket.startDelimeter){
        currentState = _length;
        dataPacket.bytesReceived = 0;
        dataPacket.len = 0;
        Serial.print(value,HEX);
        Serial.print(' ');
      }
      break;
      
    case _length:
//      value = Serial.read();
      dataPacket.len = dataPacket.len + ((uint16_t)value) << (8 * (1 - dataPacket.bytesReceived));
      dataPacket.bytesReceived++;
      Serial.print(value,HEX);
      if(dataPacket.bytesReceived == 2){
        Serial.print(' ');
        currentState = _ftype;
        dataPacket.bytesReceived = 0;
        dataPacket.checksum = 0;
      }
      break;
      
    case _ftype:
//      value = Serial.read();
      if(value == dataPacket.ftype){
        Serial.print(value,HEX);
        Serial.print(' ');
        dataPacket.checksum += value;
        currentState = _64baddr;
        dataPacket.bytesReceived = 0;
      }
      break;
      
    case _64baddr:
//      value = Serial.read();
      dataPacket.srcaddr64[dataPacket.bytesReceived] = value;
      dataPacket.bytesReceived++;
      dataPacket.checksum += value;
      Serial.print(value,HEX);
      if(dataPacket.bytesReceived == 8){
        Serial.print(' ');
        currentState = _16baddr;
        dataPacket.bytesReceived = 0;
      }
      break;

    case _16baddr:
//      value = Serial.read();
      dataPacket.srcaddr16[dataPacket.bytesReceived] = value;
      dataPacket.bytesReceived++;
      dataPacket.checksum += value;
      Serial.print(value,HEX);
      if(dataPacket.bytesReceived == 2){
        Serial.print(' ');
        currentState = _recvopt;
      }
      break;

    case _recvopt:
//      value = Serial.read();
      if(value == dataPacket.recvopt ){
        Serial.print(value,HEX);
        Serial.print(' ');
        dataPacket.checksum += value;
        dataPacket.len -= 12;
        currentState = _data;
        dataPacket.bytesReceived = 0;
      }
      break;

    case _data:
      if(dataPacket.len > 0){
//        value = Serial.read();
        dataPacket.packetData[dataPacket.bytesReceived] = value;
        dataPacket.bytesReceived++;
        dataPacket.len--;
        dataPacket.checksum += value;
        Serial.print(value,HEX);
        break;
      }else{
        Serial.print(' ');
        currentState = _checksum;
      }
      
    case _checksum:
      
      Serial.print(value,HEX);
      dataPacket.checksum = 0xFF - dataPacket.checksum;
      Serial.println();
      if(value == dataPacket.checksum){
        currentState = _start;
        Serial.println("Succesfully Received Packet");
        displayData();
      }else{
        Serial.print("Checksum: ");
        Serial.print(value,HEX);
        Serial.print(" vs ");
        Serial.println(dataPacket.checksum,HEX);
        currentState = _start;
      }
      break;
      
    default:
      Serial.print("How the hell did you get here?");
  }
//  printStatus();

}


void serialEvent(){
  while(Serial.available()){
    stateMachine();
  }
}
