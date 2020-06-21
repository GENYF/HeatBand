#include <SSD1306.h>
#include <DHT.h>
#include <DFRobot_Heartrate.h>
#include <SoftwareSerial.h>

const int HeartRatePin = A0;
const int DHTPIN = D5;
const int WALKPIN = D6;
const int TUNEPIN = D7;
int BPM = 0;
int Humi = 0;
int Temp = 0;
int Walk = 0;

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
SSD1306  display(0x3c, 5, 4);
DFRobot_Heartrate HeartRate(DIGITAL_MODE);
SoftwareSerial Bluetooth(D4, D3); // RX, TX

void setup()
{
	Serial.begin(115200);
	while (!Serial);
	Bluetooth.begin(9600);

	pinMode(WALKPIN, INPUT);
	pinMode(TUNEPIN, OUTPUT);

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
	GetHeartRate();
	GetDHT();
	GetWalk();
	SetTune();

	display.clear();
	DrawDHT();
	DrawHeart();
	DrawStep();
	display.display();

	GetBluetooth();
	SendBluetooth();
}

void SendBluetooth()
{
	String Text = String(Humi) + "/" + String(Temp) + "/" + String(BPM) + "/" + String(Walk);
	Bluetooth.println(Text);
	delay(1000);
}

void GetBluetooth()
{
	if (Bluetooth.available())
	{
		Serial.write(Bluetooth.read());
		int Data = Bluetooth.read();
		tone(TUNEPIN, 956);
		delay(50);
		noTone(TUNEPIN);

		switch (Data)
		{
			case 'R' : display.clear();
						DrawMessage();
						display.display();
						delay(3000);
						break;

			case 'M': display.clear();
						DrawRest();
						display.display();
						delay(3000);
						break;

			default:
				break;
		}
	}
	else
	{
		noTone(TUNEPIN);
	}
}

void GetHeartRate()
{
	int SumRate = 0;
	uint8_t RateValue;
	HeartRate.getValue(HeartRatePin);
	RateValue = HeartRate.getRate();

	while (0 == RateValue || 239 == RateValue)
	{
		GetWalk();
		GetBluetooth();
		HeartRate.getValue(HeartRatePin);
		RateValue = HeartRate.getRate();
		delay(50);
	}

	BPM = RateValue;
}

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

void GetWalk()
{
	if (digitalRead(WALKPIN) == HIGH)
	{
		Walk++;
	}
}

void SetTune()
{
	if (BPM > 100 || BPM < 50)
	{
		tone(TUNEPIN, 956);
		delay(1000);
		noTone(TUNEPIN);
	}
	else
	{
		noTone(TUNEPIN);
	}

	if (Temp >= 33)
	{
		tone(TUNEPIN, 956);
		delay(500);
	}
	else if (Temp >= 35)
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

void DrawBoot()
{
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(5, 10, "Heat Band");
	display.setFont(ArialMT_Plain_10);
	display.drawString(5, 35, "Loading...");
}

void DrawMessage()
{
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(5, 10, "Message");
	display.setFont(ArialMT_Plain_10);
	display.drawString(5, 35, "Arrival...");
}

void DrawRest()
{
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(5, 10, "Rest");
	display.setFont(ArialMT_Plain_10);
	display.drawString(5, 35, "Required...");
}

void DrawDHT()
{
	int x = 0;
	int y = 0;
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

void DrawHeart()
{
	int x = 0;
	int y = 0;
	display.setFont(ArialMT_Plain_16);
	String BPMText = String(BPM) + " BPM";
	display.drawString(0 + x, 40 + y, BPMText);
}

void DrawStep()
{
	int x = 0;
	int y = 0;
	display.setFont(ArialMT_Plain_10);
	String WalkText = String(Walk) + " Step";
	display.drawString(80 + x, 40 + y, WalkText);
}