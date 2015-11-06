/*
Distance to liters convertion table for Hippo Pool V0.3

0 - 10.00 - 10.00 - 10 - Critical 
1 - 9.89 - 9.93 - 9 - Critical
2 - 9.70 - 9.75 - 8 - Critical
3 - 9.49 - 10.13 - 8 - Minimum
4 - 9.35 - 9.70 - 7 - Norm
5 - 9.03 - 9.31 - 6 - Norm
6 - 8.44 - 8.80 - 5 - Max
7 - 7.47 - 8.21 - 4 Critical 


*/
#include <LiquidCrystal.h> // LCD Library
#include <NewPing.h> // Sonar Library

#define TRIGGER_PIN  8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define IRpin A2 // Arduino pin tied to InfraRed sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


// Инициализируем объект-экран, передаём использованные 
// для подключения контакты на Arduino в порядке:
// RS, E, DB4, DB5, DB6, DB7
LiquidCrystal lcd(2, 3, 10, 11, 12, 13);
const int numReadings = 10;

// Approximator

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  pinMode(IRpin, INPUT);
  
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}


void loop() {
  uint16_t value = analogRead (IRpin);
  double distance = get_IR (value);
  
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = distance;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  Serial.print("Sonar ");  
  Serial.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.print("cm ");
  Serial.print("Infrared ");
  Serial.print (distance);
  Serial.print (" cm");
  Serial.println ();
  lcd.begin(16, 2);
  lcd.print("Sonar       ");
  lcd.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range);
  lcd.print (" cm");
  lcd.setCursor(0, 1);
  lcd.print("Infrared ");
  lcd.print (distance);
  lcd.print (" cm");
  delay(500);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  
}


//return distance (cm)
double get_IR (uint16_t value) {
        if (value < 16)  value = 16;
        return 2076.0 / (value - 11.0);
}

