#include <SoftwareSerial.h>

//Программа для пульта
//ЗДЕСЬ НИЧЕГО ТРОГАТЬ НЕ НАДО кроме выводимых названий/имён команд

//команды прописываются в монитор порта и имеют следующую форму "НАЗВАНИЕКОМАНДЫ_ЗНАЧЕНИЕ"
//ПРИМЕР: "zero_34" - присваивает 0 элементу отправляемого массива значение 34
//ВАЖНО: значение должно быть в диапозоне от -128 до 128

//tx_dic содержит названия команд для формирования отправляемого массива
//индекс команды в этом массиве - индекс элемента в отправляемом массиве который она меняет
String tx_dic[] = {"mode", "setT", "led", "co2", "four"};

//rx_dic содержит названия значений, получаемых со станции
//в названии всех кроме первого должно вначале стоять "\t" (так будет красиво)
String rx_dic[] = {"hm=", "\ttemperature=", "\tpressure=", "", "", "\tco2=", "", "   tout="};

String analyse_dic[] = {"max", "\ttimer", "\tdif"};

//*******************************************!!!ACHTUNG!!!***************************************************
//*                                                                                                         *
//* ДАЛЬШЕ НИЧЕГО ТРОГАТЬ НЕ НАДО, ЕСЛИ СВЯЗИ НЕТ, ТО СНАЧАЛА СЕМЬ РАЗ ПРОВЕРЬ НЕ ЗАПОРОЛ ЛИ ТЫ КОД СТАНЦИИ *
//*     ЕСЛИ ПРИ ЗАПУСКЕ ПЛАТЫ ПЕРЕД СТРОКОЙ О СТАРТЕ ПИШЕТ "status=0" ТО ПРОВЕРЬ ПОДКЛЮЧЕНИЕ ДАТЧИКА       *
//*                                                                                                         *
//***********************************************************************************************************

//**************************************************


//объявляем массивы:
byte rx_buf[] = {0, 0, 0, 0, 0, 0, 0}; //принимаемый
byte message[] = {0, 0, 28, 0}; //служебный для отправки
unsigned long analyse[] = {99000, 0, 0}, prev_pr = 99000, timer;
byte time_step = 100;
unsigned long = start;

SoftwareSerial radio(10, 11); // RX TX

//***************************************************
void setup()
{
  Serial.begin(9600);      //Связь с ноутом
  radio.begin(9600);    // подключение к радиомодулю
}
void loop()
{
  start = millis()
  if (Serial.available()) // если была введена команда, то обрабатываем (просто не трогай и оно не сломается)
  {
    String com = Serial.readString();  //считываем команду
    int endcom = com.indexOf('_');     //выделяем имя команды
    bool def_com = false;              //переменная на случай нераспознанной команды
    for (int i = 0; i < 5; i++)       // ищем команду в словаре команд и присваемаем заданное значение соответсвующему элементу отправляемого массива
    {
      if (com.substring(0, endcom) == tx_dic[i])
      {
        message[i] = com.substring(endcom + 1).toInt();
        def_com = true;
      }
    }
    if (!def_com) //если команда не распознана, выводим сообщение
    {
      Serial.println("Command not defined!");
    }
  }
  
  int long pr = rx_buf[2] * 10000L + rx_buf[3] * 100L + rx_buf[4]; // собираем давление

  if (pr > analyse[0]) {
    timer = millis();
    analyse[0] = pr;
  }
  analyse[1] = (millis() - timer) / 1000L;
  analyse[2] = prev_pr - pr;
  prev_pr = pr;


  Serial.print("Recieved:");  //Выводим принятые значения
  for (byte i = 0; i < (sizeof(rx_buf) / sizeof(rx_buf[0])); i++)
  {
    Serial.print(rx_dic[i]);
    if (rx_buf[i] < 10) Serial.print(0);
    if (rx_buf[i] < 128) Serial.print(rx_buf[i]);
    else Serial.print(rx_buf[i] - 256);
  }
  Serial.print("\tTransmitted:"); //Выводим отправленные значения
  for (int i = 0; i < (sizeof(message) / sizeof(message[0])); i++)
  {
    Serial.print('\t');
    Serial.print(tx_dic[i]);
    Serial.print('=');
    if (message[i] < 128) Serial.print(message[i]);
    else Serial.print(message[i] - 256);
  }

  Serial.print("\tAnalysed:"); //Выводим отправленные значения
  for (int i = 0; i < (sizeof(analyse) / sizeof(analyse[0])); i++)
  {
    Serial.print(analyse_dic[i]);
    Serial.print('=');
    Serial.print(analyse[i]);
  }
  Serial.println();

  transmition();  //отправляем массив, изменяемый командами из командами
  delay(time_step - (millis() - start));
}

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
