#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "base64.hpp"

RF24 radio(4, 10);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001"; // Address

struct Data_Package {
  byte LjoyX;
  byte LjoyY;
  byte RjoyX;
  byte RjoyY;
  bool Button1;
  bool Button2;
  bool Button3;
  bool Button4;
  bool Button5;
  bool Button6;
  bool Button7;
  bool Button8;
  bool Button9;
  bool Button10;
  bool Button11;
  bool Button12;
  bool Button13;
  bool Button14;
  bool Button15;
  bool Button16;
};
Data_Package data;
Data_Package previous_data;

int keyIn = 0;
int Button_vals[7]  = {112, 197, 279, 372, 496, 683, 1023};
byte Button_num0[7] = { 12,  2,  3,  9,  8, 11,  10};
byte Button_num1[7] = {  6,  4,  7,  5,  1,  0,  13};
int range = 15;
int CRC = 0xa5;
byte val = 0;
uint8_t output_buffer[128];

uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

void setup()
{
  Serial.begin(115200);

  // Define the radio communication
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(true);
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(sizeof(data));
  radio.stopListening();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  resetData();
}

void loop()
{
  data.RjoyX = map(analogRead(A0), 0, 1024, 255, 0);
  delay(10);
  data.RjoyY = map(analogRead(A1), 0, 1024, 255, 0);
  delay(10);
  data.LjoyX = map(analogRead(A2), 0, 1024, 255, 0);
  delay(10);
  data.LjoyY = map(analogRead(A3), 0, 1024, 255, 0);
  delay(10);
  readButtons();
  radio.write(&data, sizeof(Data_Package));

  CRC = 0;
  Serial.write((byte)'*');  //Send start byte
  CRC = crc_xmodem_update(CRC, '*');
  encode_base64(&data.LjoyX, sizeof(Data_Package), output_buffer);
  for (int i = 0; i < strlen(output_buffer); i++) {
    val = output_buffer[i];
    Serial.write((byte)val);
    CRC = crc_xmodem_update(CRC, val);
  }
  Serial.write((byte)'*');  //Send stop byte
  CRC = crc_xmodem_update(CRC, '*');
  Serial.write((byte)((CRC & 0xFF00) >> 8));  //Send CRC
  Serial.write((byte)CRC & 0xFF);
  Serial.write('\n');
  delay(20);
}

void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.LjoyX = 127;
  data.LjoyY = 127;
  data.RjoyX = 127;
  data.RjoyY = 127;
  data.Button1 = false;
  data.Button2 = false;
  data.Button3 = false;
  data.Button4 = false;
  data.Button5 = false;
  data.Button6 = false;
  data.Button7 = false;
  data.Button8 = false;
  data.Button9 = false;
  data.Button10 = false;
  data.Button11 = false;
  data.Button12 = false;
  data.Button13 = false;
  data.Button14 = false;
}
void readButtons() {
  keyIn =  analogRead(A7);      //Read in keypad input
  for (int i = 0; i < 7; i++)   //Run through the array of button values
  {
    if (keyIn >= Button_vals[i] - range && keyIn <= Button_vals[i] + range) //If the measured value is in the range of one key
    {
      (*(&data.Button1 + Button_num0[i])) = true;
    }
    else {
      (*(&data.Button1 + Button_num0[i])) = false;
    }
  }
  keyIn =  analogRead(A6);      //Read in keypad input
  for (int i = 0; i < 7; i++)   //Run through the array of button values
  {
    if (keyIn >= Button_vals[i] - range && keyIn <= Button_vals[i] + range) //If the measured value is in the range of one key
    {
      (*(&data.Button1 + Button_num1[i])) = true;
    }
    else {
      (*(&data.Button1 + Button_num1[i])) = false;
    }
  }
}
