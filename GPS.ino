#include <SoftwareSerial.h>
#include <LiquidCrystal.h>;
#include "Wire.h" // This library allows you to communicate with I2C devices.

SoftwareSerial GPSModule(10, 11); // RX, TX
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
//GPS
int updates;
int failedUpdates;
int pos;
int stringplace = 0;
String timeUp;
String nmea[15];
String labels[12] {"Time: ", "Status: ", "Latitude: ", "Hemisphere: ", "Longitude: ", "Hemisphere: ", "Speed: ", "Track Angle: ", "Date: "};
double distance;
int times = 0;
int hh;
int mm;
int ss;
//Acc
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data
char tmp_str[7]; // temporary variable used in convert function

//condition
boolean statue = false;

int buttonPin = 8;
int sensorPin = A0;
int sensorValue = 0;
int buttonState = 0;
int outPin = 13;

int howMany = 0;

int tonePin = 12;

void setup() {
  Serial.begin(57600);
  //GPS
  GPSModule.begin(9600);
  //LCD
  lcd.begin(16, 2);
  //Acc
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  pinMode(buttonPin, INPUT);
  pinMode(outPin, OUTPUT);
  digitalWrite(outPin, LOW);
}

void loop() {
  welcome();

}
//-----------------------------end loop--------------------------------------

//-----------------------------Welcome Page----------------------------------
void welcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("Choose Function:");
  while (true) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 200) {
      lcd.setCursor(0, 1);
      lcd.print("Running    ");
      if (buttonState) {
        lcd.setCursor(0, 1);
        lcd.print("Begin...    ");
        delay(2000);
        Running();
      }

    } else if (sensorValue < 400) {
      lcd.setCursor(0, 1);
      lcd.print("Monitoring");
      if (buttonState) {
        lcd.setCursor(0, 1);
        lcd.print("Begin...    ");
        delay(2000);
        monitoring();
      }

    } else if (sensorValue < 600) {
      lcd.setCursor(0, 1);
      lcd.print("Timer      ");
      if (buttonState) {
        lcd.setCursor(0, 1);
        lcd.print("Begin...    ");
        delay(2000);
        Timer();
      }

    } else if (sensorValue < 800) {
      lcd.setCursor(0, 1);
      lcd.print("Counter    ");
      if (buttonState) {
        lcd.setCursor(0, 1);
        lcd.print("Begin...    ");
        delay(2000);
        counter();
      }

    } else {
      lcd.setCursor(0, 1);
      lcd.print("Time TMP     ");
      if (buttonState) {
        lcd.setCursor(0, 1);
        lcd.print("Begin...    ");
        delay(2000);
        tmp();
      }
    }
  }
}
//-------------------end Welcome-----------------------

//------------------Running-----------------------------
void Running() {
  lcd.clear();
  times = 0;
  distance = 0;
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    while (GPSModule.available() > 0)
    {
      Serial.println(GPSModule.read());

    }
    if (GPSModule.find("$GPRMC,")) {
      String tempMsg = GPSModule.readStringUntil('\n');
      for (int i = 0; i < tempMsg.length(); i++) {
        if (tempMsg.substring(i, i + 1) == ",") {
          nmea[pos] = tempMsg.substring(stringplace, i);
          stringplace = i + 1;
          pos++;
        }
        if (i == tempMsg.length() - 1) {
          nmea[pos] = tempMsg.substring(stringplace, i);
        }
      }
      updates++;
      nmea[2] = ConvertLat();
      nmea[4] = ConvertLng();
      distance += nmea[6].toDouble() / 3600;
      times++;
      lcd.setCursor(0, 0);
      lcd.print(labels[6] + nmea[6]  + "    ");
      lcd.setCursor(0, 1);
      lcd.print("D:" + String(distance) + " T:" + (int)(times / 60) + ":" + times % 60);

    }
    else {
      failedUpdates++;
    }
    stringplace = 0;
    pos = 0;
    buttonState = digitalRead(buttonPin);
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TotalD:" + String(distance));
  lcd.setCursor(0, 1);
  lcd.print("TotalT:" + String((int)(times / 60)) + ":" + String(times % 60));
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AveSpeed:" + String((distance / times) * 3600));
  delay(5000);
  lcd.clear();
  buttonState = 0;
  lcd.setCursor(0, 0);
  lcd.print("New Trip?");
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        Running();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}
//-------------------end Running-------------------
//-------------------Monitoring---------------------
void monitoring() {
  lcd.clear();
  times = 0;
  distance = 0;
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    acc();
    if (accelerometer_z < - 10000) {
      buttonState = digitalRead(buttonPin);
      lcd.setCursor(0, 0);
      lcd.print("Danger!!!!!!!!!!");
      lcd.setCursor(0, 1);
      lcd.print("Danger!!!!!!!!!!");
      sendMessage(true);
      tone(tonePin, 500);
      delay(1000);
      lcd.clear();
    } else {
      noTone(tonePin);
      while (GPSModule.available() > 0)
      {
        GPSModule.read();

      }
      if (GPSModule.find("$GPRMC,")) {
        String tempMsg = GPSModule.readStringUntil('\n');
        for (int i = 0; i < tempMsg.length(); i++) {
          if (tempMsg.substring(i, i + 1) == ",") {
            nmea[pos] = tempMsg.substring(stringplace, i);
            stringplace = i + 1;
            pos++;
          }
          if (i == tempMsg.length() - 1) {
            nmea[pos] = tempMsg.substring(stringplace, i);
          }
        }
        updates++;
        nmea[2] = ConvertLat();
        nmea[4] = ConvertLng();
        distance += nmea[6].toDouble() / 3600;
        times++;
        lcd.setCursor(0, 0);
        lcd.print(labels[6] + nmea[6]  + "    ");
        lcd.setCursor(0, 1);
        lcd.print("D:" + String(distance) + " T:" + (int)(times / 60) + ":" + times % 60);
        sendMessage(false);
      }
      else {
        failedUpdates++;
      }
      stringplace = 0;
      pos = 0;
      buttonState = digitalRead(buttonPin);
    }
  }
  noTone(tonePin);
  digitalWrite(outPin, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TotalD:" + String(distance));
  lcd.setCursor(0, 1);
  lcd.print("TotalT:" + String((int)(times / 60)) + ":" + String(times % 60));
  delay(5000);
  buttonState = digitalRead(buttonPin);
  if(buttonState){
    while(true){
      buttonState = digitalRead(buttonPin);
      lcd.setCursor(0, 0);
      lcd.print("Danger!!!!!!!!!!");
      lcd.setCursor(0, 1);
      lcd.print("Danger!!!!!!!!!!");
      sendMessage(true);
      tone(tonePin, 500);
      delay(1000);
      lcd.clear();
      buttonState = digitalRead(buttonPin);
      if(buttonState) break;
    }
    noTone(tonePin);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AveSpeed:" + String((distance / times) * 3600));
  delay(5000);
  lcd.clear();
  buttonState = 0;
  lcd.setCursor(0, 0);
  lcd.print("New Monitoring?");
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        monitoring();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}
//-----------------END Monitoring--------------------

//-----------------Timer-----------------------------
void Timer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer/Count Down" );
  while (true) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("Timer        ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        upT();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Count Down");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        downT();
      }
    }
  }
}
void upT() {
  times = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer Begin:" );
  lcd.setCursor(0, 1);
  lcd.print("3");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("2");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time:" );
  while (!buttonState) {
    buttonState = digitalRead(buttonPin);
    lcd.setCursor(0, 1);
    lcd.print(String((int)(times / 3600)) + ":" + String((int)(times / 60)) + ":" + String(times % 60));
    delay(1000);
    times++;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Total Time:");
  lcd.setCursor(0, 1);
  lcd.print(String((int)(times / 3600)) + ":" + String((int)(times / 60)) + ":" + String(times % 60));

  delay(5000);
  lcd.clear();
  buttonState = 0;
  lcd.setCursor(0, 0);
  lcd.print("New Timer?");
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        Timer();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}

void downT() {
  times = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Minutes:" );
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    sensorValue = analogRead(sensorPin) - 100;
    if (sensorValue < 0) sensorValue = 0;
    buttonState = digitalRead(buttonPin);
    lcd.setCursor(0, 1);
    lcd.print(String((int)(sensorValue / 15)) + " ");
    if (buttonState) {
      mm = (int)(sensorValue / 15);
      delay(1000);
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Seconds:" );
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    sensorValue = analogRead(sensorPin) - 100;
    if (sensorValue < 0) sensorValue = 0;
    buttonState = digitalRead(buttonPin);
    lcd.setCursor(0, 1);
    lcd.print(String((int)(sensorValue / 15)) + " ");
    if (buttonState) {
      ss = (int)(sensorValue / 15);
      delay(1000);
    }
  }
  times = mm * 60 + ss;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Begin:" );
  lcd.setCursor(0, 1);
  lcd.print("3");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("2");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time:" );
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    buttonState = digitalRead(buttonPin);
    if (times > 0) {
      lcd.setCursor(0, 1);
      lcd.print(String((int)(times / 60)) + " :" + String(times % 60) + "  ");
      delay(1000);
      times--;
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Time Out!!!");
      delay(1000);
    }
  }
  buttonState = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Timer?");
  delay(2000);
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        Timer();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}
//-----------------End Timer-------------------------

//------------------Counter-------------------------
void counter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Count/Count Down" );
  buttonState = digitalRead(buttonPin);
  while (true) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("Count        ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        count();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Count Down");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        countDown();
      }
    }
  }
}

void count() {
  howMany = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counter Begin:" );
  lcd.setCursor(0, 1);
  lcd.print("3");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("2");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counter:" );
  buttonState = digitalRead(buttonPin);
  lcd.setCursor(0, 1);
  lcd.print(String(howMany) + "  ");
  while (!buttonState) {
    acc();
    buttonState = digitalRead(buttonPin);
    if (accelerometer_z > 2000) {
      howMany++;
      lcd.setCursor(0, 1);
      lcd.print(String(howMany) + "  ");
      delay(800);
      tone(tonePin, 500);
      delay(200);
      noTone(tonePin);
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Total Done:");
  lcd.setCursor(0, 1);
  lcd.print(String(howMany) + "  ");
  delay(5000);
  lcd.clear();
  buttonState = 0;
  lcd.setCursor(0, 0);
  lcd.print("New Counter?");
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        counter();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO  ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}

void countDown() {
  howMany = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("How Many:" );
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    sensorValue = analogRead(sensorPin) - 100;
    if (sensorValue < 0) sensorValue = 0;
    buttonState = digitalRead(buttonPin);
    lcd.setCursor(0, 1);
    lcd.print(String((int)(sensorValue / 15)) + " ");
    if (buttonState) {
      howMany = (int)(sensorValue / 15);
      delay(1000);
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Begin:" );
  lcd.setCursor(0, 1);
  lcd.print("3");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("2");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Need to do:" );
  lcd.setCursor(0, 1);
  lcd.print(String(howMany) + "  ");
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    acc();
    buttonState = digitalRead(buttonPin);
    if (howMany > 0) {
      if (accelerometer_z > 2000) {
        howMany--;
        lcd.setCursor(0, 1);
        lcd.print(String(howMany) + "  ");
        delay(800);
        tone(tonePin, 500);
        delay(100);
        noTone(tonePin);
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Your Done!!!");
      delay(1000);
    }
  }
  buttonState = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Counter?");
  delay(2000);
  buttonState = digitalRead(buttonPin);
  while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        counter();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    }
  }
}
//---------------END Counter----------------

//--------------Time TMP-------------------
void tmp(){
  lcd.clear();
  buttonState = 0;
   while (!buttonState) {
    acc();
    while (GPSModule.available() > 0)
    {
      Serial.println(GPSModule.read());

    }
    if (GPSModule.find("$GPRMC,")) {
      String tempMsg = GPSModule.readStringUntil('\n');
      for (int i = 0; i < tempMsg.length(); i++) {
        if (tempMsg.substring(i, i + 1) == ",") {
          nmea[pos] = tempMsg.substring(stringplace, i);
          stringplace = i + 1;
          pos++;
        }
        if (i == tempMsg.length() - 1) {
          nmea[pos] = tempMsg.substring(stringplace, i);
        }
      }
      updates++;
      nmea[2] = ConvertLat();
      nmea[4] = ConvertLng();
      
      hh = (nmea[0].substring(0, 2).toInt() + 20)%24;
      lcd.setCursor(0, 0);
      lcd.print("Time:" + String(hh) + ":" +nmea[0].substring(2, 4) + ":" +nmea[0].substring(4, 6) + "   ");
      lcd.setCursor(0, 1);
      lcd.print("TMP:" + String(temperature / 340.00 + 25) + "  ");

    }
    else {
      failedUpdates++;
    }
    stringplace = 0;
    pos = 0;
    buttonState = digitalRead(buttonPin);
  }
  buttonState = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Want Quit?");
  delay(2000);
   while (!buttonState) {
    sensorValue = analogRead(sensorPin);
    buttonState = digitalRead(buttonPin);
    if (sensorValue < 500) {
      lcd.setCursor(0, 1);
      lcd.print("YES");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        welcome();
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("NO ");
      if (buttonState) {
        buttonState = 0;
        delay(1000);
        tmp();
      }
    }
  }
}
//----------------END TMP--------------------------------------------------

String ConvertLat() {
  String posneg = "";
  if (nmea[3] == "S") {
    posneg = "-";
  }
  String latfirst;
  float latsecond;
  for (int i = 0; i < nmea[2].length(); i++) {
    if (nmea[2].substring(i, i + 1) == ".") {
      latfirst = nmea[2].substring(0, i - 2);
      latsecond = nmea[2].substring(i - 2).toFloat();
    }
  }
  latsecond = latsecond / 60;
  String CalcLat = "";

  char charVal[9];
  dtostrf(latsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLat += charVal[i];
  }
  latfirst += CalcLat.substring(1);
  latfirst = posneg += latfirst;
  return latfirst;
}

String ConvertLng() {
  String posneg = "";
  if (nmea[5] == "W") {
    posneg = "-";
  }

  String lngfirst;
  float lngsecond;
  for (int i = 0; i < nmea[4].length(); i++) {
    if (nmea[4].substring(i, i + 1) == ".") {
      lngfirst = nmea[4].substring(0, i - 2);
      //Serial.println(lngfirst);
      lngsecond = nmea[4].substring(i - 2).toFloat();
      //Serial.println(lngsecond);

    }
  }
  lngsecond = lngsecond / 60;
  String CalcLng = "";
  char charVal[9];
  dtostrf(lngsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLng += charVal[i];
  }
  lngfirst += CalcLng.substring(1);
  lngfirst = posneg += lngfirst;
  return lngfirst;
}
void acc() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers

  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = (Wire.read() << 8 | Wire.read()) + 8000; // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = (Wire.read() << 8 | Wire.read()) - 2750; // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = (Wire.read() << 8 | Wire.read()) - 16000; // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = (Wire.read() << 8 | Wire.read()) - 117; // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = (Wire.read() << 8 | Wire.read()) + 400; // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = (Wire.read() << 8 | Wire.read()) + 48; // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
}
void sendMessage(boolean danger) {
  if (danger) {
   digitalWrite(outPin, HIGH);
  } else {
   digitalWrite(outPin, LOW);
}
}
