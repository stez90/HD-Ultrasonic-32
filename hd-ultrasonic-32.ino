/*************************************************************
    High Resolution Ultrasonic Sensor for ESP32 boards
    Copyright (C) 2021  Stefano Pieretti https://github.com/stez90

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  V1.0.0 7-mar-2021

  !!!USE ONLY WITH ESP32 BOARDS!!!

*************************************************************/

//check board compatibility
#ifndef ESP32
  #error This code is fo ESP32 only, check your board setting.
#endif

//CPU clock in MHz
int cpu_freq = 0;

//pin mapping
int pinTrigger = 34;
int pinEcho = 35;

//isr variables
volatile unsigned long cpuTimeRising = 0;
volatile unsigned long cpuTimeFalling = 0;
volatile unsigned long cpuTimePlaceholder = 0;

int currentEchoPin; //current pin to read from isr


//other variables
unsigned long elapsedCpuTime = 0;
long timeOfFlight = 0;

void IRAM_ATTR isrCHANGE() {
  //gets cpu timing when echo pin changes logic state

  cpuTimePlaceholder = ESP.getCycleCount(); //get cpu time before evaluating if statement

  //assign cpu time to the right variable for high and low cases
  if (digitalRead(currentEchoPin) == 1) {
    cpuTimeRising = cpuTimePlaceholder;
  }
  else {
    cpuTimeFalling = cpuTimePlaceholder;
  }
}

long ToF (int trigger1, int echo1) {
  //function: measure time of fligh of echo
  // units: nanoseconds (1e-9s)

  //enable interrupts for ToF calculation on echo1 pin and set current echo pin
  currentEchoPin = echo1;
  attachInterrupt(echo1, isrCHANGE, CHANGE);

  //trigger sensor
  digitalWrite(trigger1, LOW); //reset trigger logic level before start
  delayMicroseconds(5);
  digitalWrite(trigger1, HIGH); //send a 10uS trigger HIGH pulse
  delayMicroseconds(10);
  digitalWrite(trigger1, LOW);

  //wait while ISR collects timing data, adjust delay to at least double of expected ToF
  delay(20);

  //disable interrupts
  detachInterrupt(echo1);

  //do some math to retrieve elapsed time in nanoseconds
  elapsedCpuTime = (cpuTimeFalling - cpuTimeRising) * (1000.0 / cpu_freq); //elapsed time in ns

  return elapsedCpuTime;
}

void setup() {

  //set input and output
  pinMode(pinTrigger, OUTPUT);
  pinMode(pinEcho, INPUT);

  //enable serial communication
  Serial.begin(250000);

  //get cpu frequency
  cpu_freq = ESP.getCpuFreqMHz();
}

void loop() {

  //measure time of flight of the echo in nanoseconds
  timeOfFlight = ToF(pinTrigger, pinEcho);

  //print time of flight in nanosecodns and distance in microns assuming 344m/s as speed of sound
  Serial.print(timeOfFlight);
  Serial.print(" ");
  Serial.println(timeOfFlight * (3.44e-1 / 2));

}
