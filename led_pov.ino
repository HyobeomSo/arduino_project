#include <RTClib.h>
#include <DHT11.h>

RTC_DS1307 rtc;  // RTC 객체 생성
DHT11 dht11(6);  // 온습도 센서 객체 생성

const byte font[][5] = {
  // 5x5 폰트, 비트 연산 &를 통해 해당 위치의 LED를 ON 한다.
  { 0x00, 0x1F, 0x11, 0x1F, 0x00 },  // 0
  { 0x00, 0x00, 0x1F, 0x00, 0x00 },  // 1
  { 0x00, 0x17, 0x15, 0x1D, 0x00 },  // 2
  { 0x00, 0x15, 0x15, 0x1F, 0x00 },  // 3
  { 0x00, 0x1C, 0x04, 0x1F, 0x00 },  // 4
  { 0x00, 0x1D, 0x15, 0x17, 0x00 },  // 5
  { 0x00, 0x1F, 0x15, 0x17, 0x00 },  // 6
  { 0x00, 0x1C, 0x10, 0x1F, 0x00 },  // 7
  { 0x00, 0x1F, 0x15, 0x1F, 0x00 },  // 8
  { 0x00, 0x1D, 0x15, 0x1F, 0x00 },  // 9
  { 0x0E, 0x08, 0x0E, 0x08, 0x0E },  // 90도 돌린 3 10
  { 0x0E, 0x02, 0x0E, 0x0A, 0x0E },  // 90도 돌린 9 11
  { 0x1F, 0x14, 0x14, 0x0B, 0x00 },  // R 12
  { 0x1F, 0x14, 0x14, 0x08, 0x00 },  // P 13
  { 0x1F, 0x08, 0x04, 0x08, 0x1F },  // M 14
  { 0x11, 0x02, 0x04, 0x08, 0x11 },  // % 15
  { 0x14, 0x14, 0x1C, 0x00, 0x01 },  // 도씨 16
};

int b1 = 15;  // Blue LED
int b2 = 16;
int b3 = 17;
int b4 = 2;
int b5 = 3;
int g1 = 0;  // Green LED
int g2 = 1;
int g3 = 7;
int g4 = 8;
int g5 = 9;
int g6 = 10;
int g7 = 11;
int r1 = 12;  // Red LED
int r2 = 13;
int r3 = 14;

int sensorPin = 5;  // Hall 센서의 디지털 신호를 입력 받는 핀

unsigned int deg = 0;            // 모터의 현재 각도
unsigned long previousTime = 0;  // 동작시간 + 1000ms
unsigned long period = 0;        // 모터 1회전 주기
unsigned long prevTime = 0;      // 주기 측정을 위한 변수
unsigned long time;              // 잔상을 남기기 위해 LED가 켜지는 시간

int temperature = 19;  // 온도를 읽어 저장 할 전역변수
int humidity = 96;     // 습도를 읽어 저장 할 전역변수

byte hours = 12;  // 시간을 저장할 전역변수들, RTC halt시 해당 시간으로 동작
byte minutes = 27;
byte seconds = 0;

int split_rpm[7] = { 14, 13, 12, 0, 0, 0, 0 };  // RPM 표시를 위해 1000의 자리, 100의 자리, 10의 자리, 1의 자리로 나눠서 저장 할 배열
int split_dht[6] = { 0, 0, 16, 15, 0, 0 };      // 온도 습도 표시를 위해 1000의 자리, 100의 자리, 10의 자리, 1의 자리로 나눠서 저장 할 배열

int numCount;  // 5x5 폰트의 인덱스를 1씩 증가시켜 세로로 1줄씩 비트연산을 하기 위한 변수
int rpmCount;
int tempCount;
int humCount;
int rpmJarisu;  // RPM 배열의 자릿수를 증가 시키기 위한 변수
int dhtJarisu;  // 온습도 센서 자릿수를 증가 시키기 위한 변수

int humRpmFlag = 0;  // RPM을 표시할지, 온습도를 표시할지 결정할 플래그

int contact = 0;  // Hall 센서를 감지 할 전역변수

void setup() {
  pinMode(b1, OUTPUT);  // LED 연결된 핀들 OUTPUT으로 설정
  pinMode(b2, OUTPUT);
  pinMode(b3, OUTPUT);
  pinMode(b4, OUTPUT);
  pinMode(b5, OUTPUT);
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(g1, OUTPUT);
  pinMode(g2, OUTPUT);
  pinMode(g3, OUTPUT);
  pinMode(g4, OUTPUT);
  pinMode(g5, OUTPUT);
  pinMode(g6, OUTPUT);
  pinMode(g7, OUTPUT);
  pinMode(sensorPin, INPUT);  // Hall 센서와 연결된 핀을 입력으로 설정

  dht11.readTemperatureHumidity(temperature, humidity);  // 온도, 습도 읽기

  split_dht[0] = temperature / 10;  // 자릿수 단위로 분리하여 split_dht 배열에 저장
  split_dht[1] = temperature % 10;
  split_dht[4] = humidity % 10;
  split_dht[5] = humidity / 10;

  rtc.begin();  // RTC와 I2C 통신 시작

  if (rtc.isrunning()) {  // RTC가 정상동작하면 RTC 시간을 시간 변수에 대입
    DateTime now = rtc.now();
    hours = now.hour();
    minutes = now.minute();
    seconds = now.second();
  }

  if (hours >= 12) {  // 24시간 표현에서 12시간 표현으로 변경
    hours -= 12;
  }

  digitalWrite(g6, 1);
}

void loop() {

  if (digitalRead(sensorPin) == HIGH) {  // Hall 센서에 자석이 감지되면

    if (contact == 0) {

      if (micros() - prevTime > 20000)
        period = micros() - prevTime;  // 한주기

      time = period / 120;  // LED ON 위치를 정하기 위해 한 주기를 120등분함, 한 주기당 360도, 360 / 120 = 3도

      if (millis() >= (previousTime)) {  // 아두이노 동작시간이 저장 된 동작시간보다 1000ms 이상 지나면
        previousTime += 1000;            // 저장된 동작시간에 +1000ms
        seconds++;                       // 시간 +1초

        if (seconds == 60) {  // 60초가 되면 0초로 변경 및 1분을 더함
          seconds = 0;
          ++minutes;
        }

        if (minutes == 60) {  // 60분이 되면 0분으로 변경 및 1시간 더함
          minutes = 0;
          ++hours;
        }

        if (hours == 12) {  // 12시간 형식으로 표현하기 위해 12가되면 hours를 0으로 변경
          hours = 0;
        }

        if (seconds % 5 == 0) {  // RPM, 온습도 표시 전환 플래그, 5초마다 바뀌도록 해놓음
          if (humRpmFlag == 1)
            humRpmFlag = 0;
          else
            humRpmFlag = 1;
        }
      }

      long temp = 60000000 / period;  // RPM 계산, 60000000마이크로초 = 60000밀리초 = 60초, RPM = 60초 / 1회전 주기

      split_rpm[6] = temp / 1000;  // RPM 자릿수 단위로 분리 ex) 1333 -> split_rpm[0] = 1, split_rpm[1] = 3, split_rpm[2] = 3, split_rpm[4] = 3
      split_rpm[5] = temp % 1000 / 100;
      split_rpm[4] = temp % 100 / 10;
      split_rpm[3] = temp % 10;

      rpmJarisu = 0;  // 글자 표현을 위해 각 전역 변수들 초기화
      dhtJarisu = 0;
      numCount = 0;
      rpmCount = 4;
      humCount = 4;
      tempCount = 0;
      deg = 0;  // 현재 각도 / 3

      while (deg < 120) {  // 360도를 120도로 쪼갬 즉, 1 deg당 3도

        if (deg > 1 && deg < 7)  // 해당 각도 범위동안 숫자 2를 그림
          drawNum(2);


        if (deg > 27 && deg < 33)  // 해당 각도 범위동안 90도 돌린 3을 그림
          drawNum(10);


        if (deg > 57 && deg < 63)  // 해당 각도 범위동안 9를 그림(뒤집혀서 6으로 보이기 때문)
          drawNum(9);


        if (deg > 87 && deg < 93)  // 해당 각도 범위동안 90도 돌린 9를 그림
          drawNum(11);



        if (deg > 114 && deg < 119)  // 해당 각도 범위동안 1을 그림
          drawNum(1);


        if (humRpmFlag == 1) {  // humRpmFlag가 1이면 RPM를 표시

          if (deg > 40 && deg % 2 == 0 && rpmJarisu < 6)
            drawRpm(split_rpm[rpmJarisu]);

        } else {  //  humRpmFlag가 0이면 온습도 표시

          if (deg % 2 == 0 && dhtJarisu < 3)
            drawTempHum(split_dht[dhtJarisu]);

          if (deg > 60 && deg % 2 == 0 && dhtJarisu < 6)
            drawTempHum(split_dht[dhtJarisu]);
        }

        if (deg % 10 == 0)  // 정각 간격으로 점을 찍음
          drawMinuteLine();

        if ((deg == (hours * 10) + (minutes / 12)))  // 현재 시간 곱하기 10이 deg와 일치하면 시침을 그림
          drawHoursNeedle();                         // ex) 3시 일 때 hours * 10은 30이고 deg가 루프를 반복하다가 30이 되면
                                                     // 3 * 30 = 90도 지점(3시)에 해당 LED가 켜짐
        if (deg == minutes * 2)                      // 현재 분 곱하기 2가 deg와 일치하면 분침을 그림
          drawMinutesNeedle();                       // ex) 30분 이면 minutes는 30이고 30 * 2 = 60, deg가 루프를 반복하다가 60이 되면
                                                     // 3 * 60 = 180도 지점(30분)에 해당 LED가 켜짐

        if (deg == seconds * 2)  // 현재 초 곱하기 2가 deg와 일치하면 초침을 그림
          drawSecondsNeedle();   // ex) 현재 45초 이면 seconds는 45이고 45 * 2 = 90, deg가 루프를 반복하다가 90이 되면
                                 // 90 * 3 = 270도 지점에서(45초)에 해당 LED가 켜짐

        delayMicroseconds(time);  // 잔상을 남기기 위해 LED가 켜지는 시간
        displayClear();           // 위 시간 동안 잔상을 남긴 후 꺼짐

        deg++;
      }

      prevTime = micros();  // 1바퀴 돌때마다 다시 한주기를 측정
    }

    contact = 1;

  } else {

    contact = 0;
  }
}

void displayClear() {  // LED를 전부 끔
  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
  digitalWrite(b3, LOW);
  digitalWrite(b4, LOW);
  digitalWrite(b5, LOW);
  digitalWrite(r1, LOW);
  digitalWrite(r2, LOW);
  digitalWrite(r3, LOW);
  digitalWrite(g1, LOW);
  digitalWrite(g2, LOW);
  digitalWrite(g3, LOW);
  digitalWrite(g4, LOW);
  digitalWrite(g5, LOW);
  digitalWrite(g7, LOW);
}

void drawMinuteLine() {  // 초침, 분침 간격을 표시 할 LED
  digitalWrite(g7, HIGH);
}

void drawHoursNeedle() {  // 초침을 그릴 LED
  digitalWrite(r3, HIGH);
}

void drawMinutesNeedle() {  // 분침을 그릴 LED
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
}

void drawSecondsNeedle() {  // 초침을 그릴 LED
  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
}

void drawNum(int num) {  // 12, 3, 6, 9 표시 함수
  digitalWrite(g5, font[num][numCount] & 1);
  digitalWrite(g4, font[num][numCount] & 2);
  digitalWrite(g3, font[num][numCount] & 4);
  digitalWrite(g2, font[num][numCount] & 8);
  digitalWrite(g1, font[num][numCount] & 16);
  numCount++;
  if (numCount == 5) {
    numCount = 0;
  }
}

void drawRpm(int num) {  // RPM 표시 함수
  digitalWrite(b1, font[num][rpmCount] & 1);
  digitalWrite(b2, font[num][rpmCount] & 2);
  digitalWrite(b3, font[num][rpmCount] & 4);
  digitalWrite(b4, font[num][rpmCount] & 8);
  digitalWrite(b5, font[num][rpmCount] & 16);
  rpmCount--;
  if (rpmCount == -1) {
    rpmCount = 4;
    rpmJarisu++;
  }
}

void drawTempHum(int num) {  // 온습도 표시 함수
  if (num < 2) {
    digitalWrite(b1, font[num][tempCount] & 1);
    digitalWrite(b2, font[num][tempCount] & 2);
    digitalWrite(b3, font[num][tempCount] & 4);
    digitalWrite(b4, font[num][tempCount] & 8);
    digitalWrite(b5, font[num][tempCount] & 16);
    tempCount++;
  } else {
    digitalWrite(b1, font[num][humCount] & 1);
    digitalWrite(b2, font[num][humCount] & 2);
    digitalWrite(b3, font[num][humCount] & 4);
    digitalWrite(b4, font[num][humCount] & 8);
    digitalWrite(b5, font[num][humCount] & 16);
    humCount--;
  }

  if (tempCount == 5) {
    tempCount = 0;
    dhtJarisu++;
  }
  if (humCount == -1) {
    humCount = 4;
    dhtJarisu++;
  }
}
