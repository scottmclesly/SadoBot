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

Light Sensor values
<80 - no light
100- - ambient light
>500 - grow lamp

*/


#include <LiquidCrystal.h> // LCD Library
// #include <NewPing.h> // Sonar Library


// pH sensor stuff
#define SensorPin 2          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

#define LightSensePin 0 // Arduino pin tied to trigger pin on the Light.
//#define TRIGGER_PIN  8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
//#define ECHO_PIN     9  // Arduino pin tied to echo pin on the ultrasonic sensor.
//#define MAX_DISTANCE 20 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define IRpin A2 // Arduino pin tied to InfraRed sensor
// NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

enum water_code {
  WATER_NORMAL,
  WATER_LOW,
  WATER_TOO_LOW,
  WATER_HIGH,
  WATER_TOO_HIGH
};

struct water_level_entry {
  float litres;
  float distance;
  water_code severity;
};

// Инициализируем объект-экран, передаём использованные 
// для подключения контакты на Arduino в порядке:
// RS, E, DB4, DB5, DB6, DB7
LiquidCrystal lcd(2, 3, 10, 11, 12, 13);


// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 40;

// Approximator

double readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
double total = 0;                  // the running total
double average = 0;                // the average

const int LightTreshhold = 500;// Light On/Off toggle

// Water
float water_level(float);

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  pinMode(IRpin, INPUT);
  
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}


void loop() {

//Ph Sensor stuff

  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value

// end of pH sensor stuff

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

  float water_volume = water_level(average);

  //Printout
  Serial.print("Infrared ");
  Serial.print (average);
  Serial.print ("cm    ");
  Serial.print("Vol ");
  Serial.print (water_volume);
  Serial.print ("l ");
  Serial.print("    pH:");  
  Serial.print(phValue,2);
  Serial.print("        Light:"); 
  Serial.print(analogRead(LightSensePin));
  if (LightSensePin >= LightTreshhold) {
    Serial.print("     ON");
    } else { 
      Serial.print("     OFF");
    }
  Serial.println ();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
//  lcd.print("Sonar       ");
//  lcd.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range);
  lcd.print("Vol ");
  lcd.print (water_volume,1);
  lcd.print ("L ");
  lcd.print (average);
  lcd.print ("cm");
  lcd.setCursor(0, 1);
  lcd.print("pH: ");  
  lcd.print(phValue,2);
  lcd.print(" Lt:"); 
  lcd.print(analogRead(LightSensePin));
 
  
  delay(500);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  
}


//return distance (cm)
double get_IR (uint16_t value) {
        if (value < 16)  value = 16;
        return 2076.0 / (value - 11.0);
}

const int water_levels_count = 8;

//DISTANCE TO LITERS

float water_levels[8][2] = {
  { 0, 10 },
  { 1, 9.91 },
  { 2, 9.73 },
  { 3, 9.60 },
  { 4, 9.59 },
  { 5, 9.15 },
  { 6, 8.60 },
  { 7, 7.20 },
};

float interpolate(float o1, float o2, float i1, float i2, float i) {
    float d_o = o2 - o1;
    float d_i = i2 - i1;
    return o1 + d_o * (i - i1) / d_i;
}

float water_level(float distance) {
  if (distance > water_levels[0][1])
    return water_levels[0][0];

  for (int i = 1; i < water_levels_count; i++) {
    if (distance > water_levels[i][1]) {
      return interpolate(
        water_levels[i - 1][0], water_levels[i][0],
        water_levels[i - 1][1], water_levels[i][1],
        distance
      );
    }
  }
  return water_levels[water_levels_count - 1][0];
}

