#include <CyberLib.h>
#include <ServoTimer2.h>
#include <AltSoftSerial.h>

// ESP8266 -- Arduino
// ESP TX - 10 (Arduino SoftwareSerial RX)
// ESP RX - 9 (Arduino SoftwareSerial TX)

ServoTimer2 lr; //горизонтальный сервопривод
ServoTimer2 ud; //вертикальный сервопривод

int lrpin = 4; //порт горизонтального сервопривода
int udpin = 5; //порт вертикального сервопривода

int clr; // координаты горизонтального сервопривода (up-down servo)
int cud; // координаты вертикального сервопривода (up-down servo)

int single_step_h = 30; //Величина шага вертиклаьного привода
int single_step_v = 60; //Величина шага горизонтального привода

AltSoftSerial espSerial;

const int COM_BAUD = 9600;
 
void setup() {
  espSerial.begin(COM_BAUD);
  Serial.begin(COM_BAUD);

  clr = 1600; 
  cud = 1800;

  ud.attach(udpin); //Порты сервоприводов
  lr.attach(lrpin); 

  pinMode(4, OUTPUT);
  pinMode(5,OUTPUT);

  ud.write(cud); //Переместить сервоприводы в домашнюю позицию
  lr.write(clr);  
}
 
void loop() {
//Получаем команды от QT
if (Serial.available() > 0){
char specifier = Serial.read();
serial_deal(specifier);
 
}
}

//Команды, принимаемые от QT
void serial_deal(char spec){
  switch(spec){
    case 't':
        espSerial.println(command("AT+CWLAP",3000));
    break;
    
    case 'h':
      ud.write(1800); //Переместить сервоприводы в домашнюю позицию
      lr.write(1600);  
    break;
    
    case 'l':
      clr = clr + single_step_h; //Шаг влево
      lr.write(clr);
    break;
    
    case 'r':
      clr = clr - single_step_h; //Шаг вправо
      lr.write(clr);
    break;
    
    case 'u':
      cud = cud - single_step_v; //Шаг вверх
      ud.write(cud);
    break;
    
    case 'd':
      cud = cud + single_step_v; //Шаг вниз
      ud.write(cud);
    break;  
}
}  

  
String command(const char *toSend, unsigned long milliseconds) {
  String result;
 // Serial.write(toSend);
  espSerial.flushInput();
  espSerial.flush();
  espSerial.println(toSend);
  unsigned long startTime = millis();
  unsigned long lastCharTime = startTime;
// Serial.print("Received: ");
  while (millis() - startTime < milliseconds) {
    if (espSerial.available()) {
      char c = espSerial.read();
      lastCharTime = millis();
      Serial.write(c);
      result += c;  // append to the result string
    }
  }  // End timeout
  return result;
}

