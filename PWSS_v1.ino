/*
 Name:		PWSS_v1.ino
 Created:	9/15/2025 11:42:06 AM
 Author:	Rahul Krishna
*/

// Included Lib:
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Init:
LiquidCrystal lcd(13, 12, 11, 10, 9, 8); // RS, E, D4-D7

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BME280 bme;  

#define MQ135PIN A0
#define LDRPIN   A1

// Functions:
void printCenter(int row, String text) {
  int spaces = (20 - text.length()) / 2;
  if (spaces < 0) spaces = 0;
  lcd.setCursor(spaces, row);
  lcd.print(text);
}

// Displays loading (with animation) while POST is running
void showLoading() {  
  for (int i = 1; i <= 3; i++) {
    lcd.clear();
    String dots = "";
    for (int j = 0; j < i; j++) dots += ".";
    printCenter(1, "Loading" + dots);
    delay(500);
  }
}

// Generate random value around a base with small variation
float randomAround(float base, float variation) {
  return base + ((random(-100, 101) / 100.0) * variation); 
}

// Check analog sensor error (disconnected)
bool isAnalogSensorError(float value) {
  return (value < 5 || value > 1015); // near 0 or 1023
}

// Display sensor errors, 2 per line, center aligned
void showSensorErrors(String errors[], int count) {
  lcd.clear();
  printCenter(0, "Sensor Error:");
  int row = 1;
  for (int i = 0; i < count; i += 2) {
    String line = errors[i];
    if (i + 1 < count) line += ", " + errors[i + 1];
    printCenter(row, line);
    row++;
  }
  delay(3000); // keep errors displayed for 3 sec
}

// Forecast Logic
String getForecast(float temp, float hum, float pressure, float light) {
  if (temp >= 22 && temp <= 32 && hum >= 40 && hum <= 60 && pressure > 1000 && light > 500) {
    return "Sunny ☀️";
  } else if (temp >= 20 && temp <= 28 && hum >= 60 && hum <= 80 && pressure >= 990 && pressure <= 1010 && light >= 200 && light <= 500) {
    return "Cloudy ☁️";
  } else if (temp < 28 && hum > 80 && pressure < 990 && light < 300) {
    return "Rain Likely 🌧️";
  } else if (temp < 25 && hum > 85 && pressure < 980) {
    return "Storm Alert ⚡";
  } else {
    return "Unstable 🌍";
  }
}

void setup() {
  lcd.begin(20, 4);
  lcd.clear();

  // Splash
  printCenter(1, "Nano Bugs Team");
  printCenter(2, "Weather Station");
  delay(2000);

  // Loading animation
  for (int i = 0; i < 3; i++) {
    showLoading();
  }

  randomSeed(analogRead(0));
  dht.begin();
  bme.begin(0x76);
}

void loop() {
  String errorList[4];
  int errorCount = 0;

  // --- DHT22 ---
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) errorList[errorCount++] = "DHT22";

  // --- BME280 ---
  float pressure = 0;
  bool bmeStatus = bme.begin(0x76); // re-init in case disconnected
  if (bmeStatus) pressure = bme.readPressure() / 100.0F;
  if (!bmeStatus || pressure == 0) errorList[errorCount++] = "BME280";

  // --- MQ135 ---
  float mq135Val = randomAround(85, 5); // simulated
  if (isAnalogSensorError(mq135Val)) errorList[errorCount++] = "MQ135";

  // --- LDR ---
  float ldrVal = randomAround(400, 50); // simulated
  if (isAnalogSensorError(ldrVal)) errorList[errorCount++] = "LDR";

  // --- Show errors if any ---
  if (errorCount > 0) {
    showSensorErrors(errorList, errorCount);
    return; // skip normal display
  }

  String forecast = "Fcst: " + getForecast(t, h, pressure, ldrVal);

  // --- Display simulated values ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t, 1); lcd.print((char)223); lcd.print("C ");
  lcd.print("H:"); lcd.print(h, 0); lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("P:"); lcd.print(pressure, 1); lcd.print("hPa");

  lcd.setCursor(0, 2);
  lcd.print("AQ:"); lcd.print(mq135Val, 0); lcd.print("  ");
  lcd.print("L:"); lcd.print(ldrVal, 0);

  printCenter(3, forecast);

  delay(5000); // Refresh the values on display every 5 sec to keep the values more stable 
}
