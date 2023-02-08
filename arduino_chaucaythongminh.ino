#include <Arduino_FreeRTOS.h>

#include <Wire.h>
#include "RTClib.h"
#include <semphr.h>
#include <timers.h>
#include "DFRobot_DHT11.h"


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0X27, 16, 2);  //SCL A5 SDA A4



DFRobot_DHT11 DHT;

SemaphoreHandle_t xMutex;
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;
TaskHandle_t Task3Handle;

int nhiet_do_den_min = 0;
int nhiet_do_den_max = 0;
int nhiet_do_bom_min = 0;
int nhiet_do_bom_max = 0;
int do_am_dat_den_min = 0;
int do_am_dat_den_max = 0;
int do_am_dat_bom_min = 0;
int do_am_dat_bom_max = 0;
int do_am_khong_khi_den_min = 0;
int do_am_khong_khi_den_max = 0;
int do_am_khong_khi_bom_min = 0;
int do_am_khong_khi_bom_max = 0;

// Setup DHT
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 60000;

// Setup SOIL MOIST
#define SOIL_MOIST_1_PIN A0

int doamkhongkhi;
int nhietdo;
int doamdat;

int btnBom = 9;
int btnDen = 10;
int btnConfig = 11;

// Setup Relay
int relayBom = 8;
int relayDen = 7;

// Setup esp8266

// tinh trang bom va den 0 ->  off, 1 -> on
bool status_bom = 0;
bool status_den = 0;
int status_config = 2;

bool status_auto = 0;


void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);

// do so lieu cam bien
void readSensors() {


  DHT.read(3);
  nhietdo = DHT.temperature;
  doamkhongkhi = DHT.humidity;
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  //
  {
    anaValue += analogRead(SOIL_MOIST_1_PIN);  // Đọc giá trị cảm biến độ ẩm đất
  }
  anaValue = anaValue / 10;
  doamdat = map(anaValue, 1023, 0, 0, 100);  // Ít nước:0%  ==> Nhiều nước 100%
}

void getData() {
  if (!digitalRead(btnBom)) {
    digitalWrite(relayBom, !digitalRead(relayBom));
  }

  if (!digitalRead(btnDen)) {
    digitalWrite(relayDen, !digitalRead(relayDen));
  }
  if (!digitalRead(btnConfig)) {
    status_config = 0;
  }
  while (Serial.available() > 0) {
    byte c = Serial.read();
    if (c == '+') {
      digitalWrite(relayBom, LOW);
    }
    if (c == '(') {
      digitalWrite(relayBom, HIGH);
    }
    if (c == '$') {
      digitalWrite(relayDen, LOW);
    }
    if (c == '^') {
      digitalWrite(relayDen, HIGH);
    }
    if (c == '[') {
      status_config = 2;
    }
    if (c == '@') {
      status_auto = 0;
    }
    if (c == '<') {
      status_config = 1;
    }
    if (c == ')') {
      String data_arduino = Serial.readStringUntil('\n');
      Serial.println(data_arduino);
      int count = 0;
      int indx = 0;
      if (data_arduino[0] != 'x')
        continue;
      char tem[50];
      char temp1[50];
      char temp2[50];
      char temp3[50];
      char temp4[50];
      char temp5[50];
      char temp6[50];
      char temp7[50];
      char temp8[50];
      char temp9[50];
      char temp10[50];
      char temp11[50];
      char temp12[50];
      for (int i = 0; i < data_arduino.length(); i++) {

        if (data_arduino[i] == 'x') {
          indx = 0;
          continue;
        }

        if (count == 0 && data_arduino[i] != 'a') {
          temp1[indx++] = data_arduino[i];
        }
        if (count == 0 && data_arduino[i] == 'a') {
          nhiet_do_den_min = atof(temp1);
          count++;
          indx = 0;
          continue;
        }

        if (count == 1 && data_arduino[i] != 'b') {
          temp2[indx++] = data_arduino[i];
        }
        if (count == 1 && data_arduino[i] == 'b') {
          nhiet_do_den_max = atof(temp2);
          count++;
          indx = 0;
          continue;
        }

        if (count == 2 && data_arduino[i] != 'c') {
          temp3[indx++] = data_arduino[i];
        }
        if (count == 2 && data_arduino[i] == 'c') {
          nhiet_do_bom_min = atof(temp3);
          count++;
          indx = 0;
          continue;
        }

        if (count == 3 && data_arduino[i] != 'd') {
          temp4[indx++] = data_arduino[i];
        }
        if (count == 3 && data_arduino[i] == 'd') {
          nhiet_do_bom_max = atof(temp4);
          count++;
          indx = 0;
          continue;
        }

        if (count == 4 && data_arduino[i] != 'e') {
          temp5[indx++] = data_arduino[i];
        }
        if (count == 4 && data_arduino[i] == 'e') {
          do_am_dat_den_min = atof(temp5);
          count++;
          indx = 0;
          continue;
        }

        if (count == 5 && data_arduino[i] != 'f') {
          temp6[indx++] = data_arduino[i];
        }
        if (count == 5 && data_arduino[i] == 'f') {
          do_am_dat_bom_min = atof(temp6);
          count++;
          indx = 0;
          continue;
        }

        if (count == 6 && data_arduino[i] != 'g') {
          temp7[indx++] = data_arduino[i];
        }
        if (count == 6 && data_arduino[i] == 'g') {
          do_am_dat_den_max = atof(temp7);
          count++;
          indx = 0;
          continue;
        }

        if (count == 7 && data_arduino[i] != 'h') {
          temp8[indx++] = data_arduino[i];
        }
        if (count == 7 && data_arduino[i] == 'h') {
          do_am_dat_bom_max = atof(temp8);
          count++;
          indx = 0;
          continue;
        }

        if (count == 8 && data_arduino[i] != 'i') {
          temp9[indx++] = data_arduino[i];
        }
        if (count == 8 && data_arduino[i] == 'i') {
          do_am_khong_khi_den_min = atof(temp9);
          count++;
          indx = 0;
          continue;
        }

        if (count == 9 && data_arduino[i] != 'k') {
          temp10[indx++] = data_arduino[i];
        }
        if (count == 9 && data_arduino[i] == 'k') {
          do_am_khong_khi_den_max = atof(temp10);
          count++;
          indx = 0;
          continue;
        }

        if (count == 10 && data_arduino[i] != 'm') {
          temp11[indx++] = data_arduino[i];
        }
        if (count == 10 && data_arduino[i] == 'm') {
          do_am_khong_khi_bom_min = atof(temp11);
          count++;
          indx = 0;
          continue;
        }

        if (count == 11 && data_arduino[i] != 'n') {
          temp12[indx++] = data_arduino[i];
        }
        if (count == 11 && data_arduino[i] == 'n') {
          do_am_dat_den_max = atof(temp12);
          count = 0;
          indx = 0;
          continue;
        }
      }
      status_auto = 1;
    }
  }
}



void Task1(void* pvParameters) {
  while (1) {

    readSensors();
    if (nhietdo < 100 && doamkhongkhi < 100) {
      sendData();

      // display.clearDisplay();
      // display.setTextSize(1);
      // display.setTextColor(WHITE);
      // display.setCursor(0, 10);
      // display.print("DO AM: ");
      // display.print(doamkhongkhi);
      // display.print("%");

      // display.setCursor(0, 20);
      // display.print("NHIET DO: ");
      // display.print(nhietdo);
      // display.print("'C");

      // display.setCursor(0, 30);
      // display.print("DO AM DAT: ");
      // display.print(doamdat);
      // display.print("%");

      // display.display();
    }
    if (status_config == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Dang Cau Hinh ");
    }
     else {
      lcd.clear();

      lcd.setCursor(2, 0);
      lcd.print("T");
      lcd.setCursor(1, 1);
      lcd.print(nhietdo);
      lcd.print("C");

      lcd.setCursor(5, 0);
      lcd.print("AmDat");
      lcd.setCursor(6, 1);
      lcd.print(doamdat);
      lcd.print("%");

      lcd.setCursor(11, 0);
      lcd.print("AmKK");
      lcd.setCursor(12, 1);
      lcd.print(doamkhongkhi);
      lcd.print("%");

    }
    delay(500);
  }
}



void Task2(void* pvParameters) {
  while (1) {
    getData();

    if (status_auto) {
      if ((nhietdo > nhiet_do_bom_max) || doamdat < do_am_dat_bom_min || doamkhongkhi < do_am_khong_khi_bom_min) {
        digitalWrite(relayBom, LOW);
      }
      if ((nhietdo > nhiet_do_den_max) || doamdat < do_am_dat_den_min || doamkhongkhi < do_am_khong_khi_den_min) {
        digitalWrite(relayDen, LOW);
      }
      if ((nhietdo < nhiet_do_bom_min) || doamdat > do_am_dat_bom_max || doamkhongkhi > do_am_khong_khi_bom_max) {
        digitalWrite(relayBom, HIGH);
      }
      if ((nhietdo < nhiet_do_den_min) || doamdat > do_am_dat_den_max || doamkhongkhi > do_am_khong_khi_den_max) {
        digitalWrite(relayDen, HIGH);
      }
    }
    delay(500);
  }
}


void sendData() {
  Serial.print('x');
  Serial.print(nhietdo);
  Serial.print('a');
  Serial.print(doamdat);
  Serial.print('b');
  Serial.print(doamkhongkhi);
  Serial.print('c');
  Serial.print(digitalRead(relayBom));
  Serial.print('d');
  Serial.print(digitalRead(relayDen));
  Serial.print('e');
  Serial.print((status_config));
  Serial.print('f');
  Serial.print((status_auto));
  Serial.println('g');
}



void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // display.display();
  // lcd.init();
  // lcd.backlight();
  // if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;)
  //     ;  // Don't proceed, loop forever
  // }
  pinMode(btnBom, INPUT_PULLUP);
  pinMode(btnDen, INPUT_PULLUP);
  pinMode(btnConfig, INPUT_PULLUP);
  pinMode(relayBom, OUTPUT);
  digitalWrite(relayBom, HIGH);
  pinMode(relayDen, OUTPUT);
  digitalWrite(relayDen, HIGH);
  // xMutex = xSemaphoreCreateBinary();
  xTaskCreate(Task1, "Task1", 128, configMINIMAL_STACK_SIZE, 1, &Task1Handle);
  xTaskCreate(Task2, "Task2", 128, configMINIMAL_STACK_SIZE, 1, &Task2Handle);
  // xSemaphoreGive(xMutex);
  vTaskStartScheduler();
}



void loop() {
}
