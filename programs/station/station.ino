//код станции  ИСПРАВИТЬ, ВЫПИЛИТЬ СЕРВУ

#include <Wire.h>
#include <Adafruit_Sensor.h> // барометр
#include <Adafruit_BMP280.h>
#include "DHT.h"     //для датчика влажности/температуры
#include "Servo.h"
//***************************************************
//#define FAN 8         // основной кулер  // ПЕРЕПИСАТЬ ЧЕРЕЗ КОНСТАНТЫ
//#define COOLER 10     // насос и пельтье
//#define HEATER 9          // малый кулер
//#define LED 11              // освещение

const byte FAN = 8;  // ВОТ ТАК
//***************************************************
DHT dht(12, DHT11);        //датчик влажности/температуры внутренний

Adafruit_BMP280 bmp;       //барометр
Servo servo;

String rx_dic[] = {"mode=", "\tset_t=", "\tlight=", "\tgas="};
String tx_dic[] = {"hm=", "\tt=", "\tpr=", "", "", "\tco2=", "", "\ttout="}; //массив с назвниями передаваемых значений, перед каждым кроме первого нужно "\t"
byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; //команда запроса показаний для датчика со2
unsigned char response[9] = {0}; //массив для ответа от датчика со2

// ОБЪЯВТИТЬ МАССИВЫ ПРИЁМА И ПЕРЕДАЧИ

void setup() {
  Serial.begin(9600); // связь с компьютером
  dht.begin();        //датчик влажности/температуры
  bmp.begin(0x76);
  servo.attach(13);
  pinMode(8, OUTPUT); // ПЕРЕПИСАТЬ ЧЕРЕЗ КОНСТАНТЫ
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  //pinMode(13, OUTPUT);
  Serial2.begin(9600);  //соединение с датчиком со2
}


void loop() {
  //**************************ДЕБАГ*******************************************
  Serial.print("recieved:");   //для отладки выводим принятый и отправленный массивы
  for (byte i = 0; i < RX_PLOAD_WIDTH; i++) {
    Serial.print(rx_dic[i]);
    if (rx_buf[i] < 128) Serial.print(rx_buf[i]);
    else Serial.print(rx_buf[i] - 256);
  }
  Serial.print('\t');

  Serial.print("transmitted:");
  for (int i = 0; i < TX_PLOAD_WIDTH; i++) {
    Serial.print(tx_dic[i]);
    if (message[i] < 128) Serial.print(message[i]);
    else Serial.print(message[i]);
  }
  Serial.println();

  //****************************ОПРОС ДАТЧИКОВ***********************************
  int hm = dht.readHumidity();
  int t = dht.readTemperature();
  int long pr = bmp.readPressure();
  int tout = dhtOUT.readTemperature();

  // процедура опроса датчика со2
  Serial1.write(cmd, 9); 
  Serial1.readBytes(response, 9);
  byte crc = 0;
  for (int i = 1; i < 8; i++) crc += response[i];
  crc = 255 - crc;
  crc++;
  int ppm;
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    //Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
    while (Serial1.available()) Serial1.read();
    ppm = 0;
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    ppm = (256 * responseHigh) + responseLow;
    //Serial.println(ppm);
  }


  //*********АНАЛИЗ ПРИНЯТЫХ ЗНАЧЕНИЙ И УПРАВЛЕНИЕ БОРТОВЫМИ СИСТЕМАМИ***********
  mode = rx_buf[0];
  set_t = rx_buf[1];
  light = rx_buf[2];
  bool cool = (mode == 3) or (mode == 1 and t < set_t);
  bool heat = !cool and mode;
  digitalWrite(LED, light);
  digitalWrite(HEATER, heat);
  digitalWrite(COOLER, cool);
  digitalWrite(FAN, !(cool or heat));
  if (rx_buf[3]) servo.write(180); // DHFJGKHVGHCJKLK
  else servo.write(90);

  //*********************************СВЯЗЬ***************************************
  message[0] = hm;
  message[1] = t;
  message[2] = int(pr / 10000);
  message[3] = int(pr / 100 % 100);
  message[4] = int(pr % 100);
  message[7] = tout; //УБРАТЬ
  message[5] = byte(ppm / 100);
  message[6] = byte(ppm % 100);

}
