//код станции  ИСПРАВИТЬ

#include <Wire.h>
#include <Adafruit_Sensor.h> // барометр
#include <Adafruit_BMP280.h>
#include "DHT.h"     //для датчика влажности/температуры
//***************************************************
// основной кулер
// насос и пельтье
// малый кулер
// LED освещение

const byte SCG = 8, HEATER = 9, COOLER = 10, LED = 11, STR = 12 ;
//***************************************************
DHT dht(12, DHT11);        //датчик влажности/температуры внутренний
Adafruit_BMP280 bmp;       //барометр

String rx_dic[] = {"mode=", "\tset_t=", "\tlight=", "\tgas="};
String tx_dic[] = {"hm=", "\tt=", "\tpr=", "", "", "\tco2=", ""}; //массив с назвниями передаваемых значений, перед каждым кроме первого нужно "\t"
byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; //команда запроса показаний для датчика со2
unsigned char response[9] = {0}; //массив для ответа от датчика со2

// ОБЪЯВЛЯЕМ МАССИВЫ ПРИЁМА И ПЕРЕДАЧИ
byte rx_buf[] = {0, 28, 0, 0, }; //принимаемый
byte message[] = {0, 0, 0, 0, 0, 0, 0}; //служебный для отправки

//********************************************
void setup() {
  Serial.begin(9600); // связь с компьютером
  dht.begin();        //датчик влажности/температуры
  bmp.begin(0x76);    //барометр
  pinMode(SCG, OUTPUT);
  pinMode(HEATER, OUTPUT);
  pinMode(COOLER, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(STR, OUTPUT)
  Serial2.begin(9600);  //соединение с датчиком со2
  //  ГДЕ РАДИО? ?  ?
}


void loop() {
  //**************************ДЕБАГ*******************************************
  Serial.print("recieved:");   //для отладки выводим принятый и отправленный массивы
  for (byte i = 0; i < RX_PLOAD_WIDTH; i++) {  // ОПРЕДЕЛЕНИЕ РАЗМЕРА  ? 
    Serial.print(rx_dic[i]);
    if (rx_buf[i] < 128) Serial.print(rx_buf[i]);
    else Serial.print(rx_buf[i] - 256);
  }
  Serial.print('\t');

  Serial.print("transmitted:");
  for (int i = 0; i < TX_PLOAD_WIDTH; i++) {   // ОПРЕДЕЛЕНИЕ РАЗМЕРА  ? 
    Serial.print(tx_dic[i]);
    if (message[i] < 128) Serial.print(message[i]);
    else Serial.print(message[i]);
  }
  Serial.println();

  //****************************ОПРОС ДАТЧИКОВ***********************************
  int hm = dht.readHumidity();
  int t = dht.readTemperature();
  int long pr = bmp.readPressure();
 
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
  bool cool = (mode == 3) or (mode == 1 and t < set_t); // mode=0: off; mode=1: auto; mode=2: heat; mode=3: cool;
  bool heat = !cool and mode;
  digitalWrite(LED, light);
  digitalWrite(HEATER, heat);
  digitalWrite(COOLER, cool);
  digitalWrite(SCG, );  //???

  //*********************************СВЯЗЬ***************************************
  message[0] = hm;
  message[1] = t;
  message[2] = int(pr / 10000);
  message[3] = int(pr / 100 % 100);
  message[4] = int(pr % 100);
  message[5] = byte(ppm / 100);
  message[6] = byte(ppm % 100);


  //********************************РАДИО*****************************************   // ПЕРЕПИСАТЬ ЧЕРЕЗ СЕРИАЛ1
  void transmition() {
    radio.write(0xFF);
    byte checksum = 0;
    for (byte i = 0; i < (sizeof(message) / sizeof(message[0])); i++) {
      checksum += message[i];
      radio.write(message[i]);
    }
    radio.write((0xFF - checksum) + 1);
  }

  bool reception() { // приём массива значений
    byte checksum = 0;
    byte buf[(sizeof(rx_buf) / sizeof(rx_buf[0]))] = {};
    if (radio.available() and (radio.read() == 0xFF)) {
      for (int i = 0; i < (sizeof(rx_buf) / sizeof(rx_buf[0])); i++) {
        byte val = radio.read();
        buf[i] = val;
        checksum += val;
      }
    }
    if (((0xFF - checksum) + 1) == radio.read()) {
      for (int i = 0; i < (sizeof(rx_buf) / sizeof(rx_buf[0])); i++) rx_buf[i] = buf[i];
    }
  }
