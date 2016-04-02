#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

const int chipSelect = 10;

int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

String to2digit(int number) {
  if (number >= 0 && number < 10) {
    return "0" + String(number);
  }

  return String(number);
}

String getTime() {
  tmElements_t tm;

  if (RTC.read(tm)) {
    return String(tmYearToCalendar(tm.Year)) + "-" + tm.Month + "-" + tm.Day + " " + to2digit(tm.Hour) + ":" + to2digit(tm.Minute) + ":" + to2digit(tm.Second);
  }

  return "";
}

void rtcCheck() {
  if (RTC.chipPresent()) {
    Serial.println("The DS1307 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  } else {
    Serial.println("DS1307 read error!  Please check the circuitry.");
    Serial.println();
  }
}

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);

  pinMode(8, INPUT);
  starttime = millis();//get the current time;

  if (!SD.begin(chipSelect)) {
    lcd.print("Card failed, or not present");
    return;
  }

  lcd.print("Wait heating up.");

  delay(30 * 1000);
}

void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;
  if ((millis() - starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy / (sampletime_ms * 10.0);
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve

    Serial.print("Time: " + getTime());
    Serial.print(lowpulseoccupancy);
    Serial.print(" concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    lowpulseoccupancy = 0;
    starttime = millis();

    lcd.clear();
    lcd.setCursor(0, 0);
    File dataFile = SD.open("air.csv", FILE_WRITE);
    if (dataFile) {
      String t = getTime();
      String data = t + "," + String(concentration);

      dataFile.println(data);
      dataFile.close();
      // print to the serial port too:
      lcd.print(t);
      lcd.setCursor(0, 1);
      lcd.print(concentration);
    }
    else {
      lcd.print("Data write error");
    }
  }
}
