//라이브러리와 제조사 레퍼런스 코드외에 모든 코드를 직접 작성하였습니다.
#include <SSD1306.h>
#include <DHT.h>
#include <DFRobot_Heartrate.h>
#include <SoftwareSerial.h>

//센서 핀 설정
const int HeartRatePin = A0;
const int DHTPIN = D5;  
const int WALKPIN = D6;
const int TUNEPIN = D7; 

int BPM = 0;//심박
int Humi = 0;//습도
int Temp = 0;//온도
int Walk = 0;//걸음

//제조사 레퍼런스 코드 이용
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);//DTH22 온습도 센서
SSD1306  display(0x3c, 5, 4); //OLED 디스플레이
DFRobot_Heartrate HeartRate(DIGITAL_MODE);//심박센서
SoftwareSerial BT05(D4, D3); // RX, TX, 블루투스

void setup() 
{
  //블루투스 시동
  Serial.begin(115200);
  while (!Serial);
  BT05.begin(9600);

  pinMode(WALKPIN, INPUT);
  pinMode(TUNEPIN, OUTPUT); 

  //시제품 부팅 알림
  tone(TUNEPIN, 956); 
  delay(500); 
  tone(TUNEPIN, 0);
  delay(500);
  tone(TUNEPIN, 956); 
  delay(500);
  noTone(TUNEPIN);
  
  display.init(); 
  display.flipScreenVertically();
  display.clear();
  DrawBoot(); 
  display.display();
  delay(3000);
}

void loop() 
{
  GetHeartRate();//심박 데이터 측정
  GetDHT();//온습도 데이터 측정
  GetWalk();//걸음수 데이터 측정
  SetTune();//소리 알림 설정

  //디스플레이 데이터 출력
  display.clear();
  DrawDHT();//온습도 출력
  DrawHeart();//심박 출력
  DrawStep(); //걸음 출력
  display.display();

  GetBluetooth();//블루투스 데이터 읽어오기
  SendBluetooth();//블루투스 데이터 전송
}

//블루투스 데이터 전송
void SendBluetooth()
{
  String Text = String(Humi) + "/" + String(Temp) + "/" + String(BPM) + "/" + String(Walk);
  BT05.println(Text);
  delay(1000);
}

//블루투스 데이터 읽어오기
void GetBluetooth()
{
  //일반 알림, 데이터를 받아서 소리 알림
  if (BT05.available()) 
  {
    Serial.write(BT05.read());
    tone(TUNEPIN, 956); 
    delay(50);
    noTone(TUNEPIN);
    
    display.clear();
    DrawRest();
    display.display();
    delay(3000);
  }
  else
  {
    noTone(TUNEPIN); 
  }
}

//심박 데이터 측정, 제조사 레퍼런스 코드 이용
void GetHeartRate()
{
  int SumRate = 0;
  uint8_t RateValue;
  HeartRate.getValue(HeartRatePin); 
  RateValue = HeartRate.getRate(); 

  while(0 == RateValue || 239 == RateValue)  
  {
    GetWalk();
    GetBluetooth();
    HeartRate.getValue(HeartRatePin); 
    RateValue = HeartRate.getRate();
    delay(50);
  }
  
  BPM = RateValue;
}

//온습도 데이터 측정
void GetDHT()
{
  Temp = dht.readTemperature();
  Humi = dht.readHumidity();
  
  while (isnan(Humi) || isnan(Temp) || Humi > 100 || Humi < 0 || Temp > 80 || Temp < -40)
  {
    GetWalk();
    GetBluetooth();
    Temp = dht.readTemperature();
    Humi = dht.readHumidity();
    delay(1000);
  }
}

//걸음수 데이터 측정
void GetWalk()
{
  if(digitalRead(WALKPIN) == HIGH)
  {
    Walk++;
  }
}

//소리 알림 설정
void SetTune()
{
  //심박이 기준보다 높거나 낮으면 알림
  if(BPM > 100 || BPM < 50)
  {
    tone(TUNEPIN, 956);  
    delay(1000);
    noTone(TUNEPIN); 
  }
  else
  {
    noTone(TUNEPIN); 
  }
  
  //온도가 기준보다 높으면 알림
  if(Temp >= 33)
  {
    tone(TUNEPIN, 956); 
    delay(500);
  }
  else if(Temp >= 35)
  {
    tone(TUNEPIN, 956); 
    delay(500); 
    tone(TUNEPIN, 0);
    delay(500);
    tone(TUNEPIN, 956); 
    delay(500);
    noTone(TUNEPIN);
  }
  else
  {
    noTone(TUNEPIN);
  }
}

//부팅 출력
void DrawBoot() 
{
    display.setTextAlignment (TEXT_ALIGN_LEFT);
    display.setFont (ArialMT_Plain_24);
    display.drawString (5, 10, "Heat Band");
    display.setFont (ArialMT_Plain_10);
    display.drawString (5, 35, "Loading...");
}

//휴식 알림 출력
void DrawRest() 
{
    display.setTextAlignment (TEXT_ALIGN_LEFT);
    display.setFont (ArialMT_Plain_24);
    display.drawString (5, 10, "Rest");
    display.setFont (ArialMT_Plain_10);
    display.drawString (5, 35, "Required...");
}

//온도 출력
void DrawDHT() 
{ 
  int x=0;
  int y=0;
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0 + x, 0 + y, "Humi");
  
  display.setFont(ArialMT_Plain_24);
  String HumiText = String(Humi) + "%";
  display.drawString(0 + x, 10 + y, HumiText);
  
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(95 + x, 0 + y, "Temp");
  
  display.setFont(ArialMT_Plain_24);
  String TempText = String(Temp) + "°C";
  display.drawString(70 + x, 10 + y, TempText);
}

//심박 출력
void DrawHeart() 
{ 
  int x=0;
  int y=0;
  display.setFont(ArialMT_Plain_16);
  String BPMText = String(BPM) + " BPM";
  display.drawString(0 + x, 40 + y, BPMText);
}

//걸음 수 출력
void DrawStep() 
{ 
  int x=0;
  int y=0;
  display.setFont(ArialMT_Plain_10);
  String WalkText = String(Walk) + " Step";
  display.drawString(80 + x, 40 + y, WalkText);
}
