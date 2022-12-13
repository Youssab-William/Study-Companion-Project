#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <M5Core2.h>
#define Calibration 10
 
double avgHR = 0;
unsigned long avgHRV = 0;
unsigned long timeSum = 0;
double HRSum = 0;
unsigned long i = 0;
 
unsigned long interval2 = 1800000; 
unsigned long Time1 = 0;
unsigned long Time2;
unsigned long StartTime = 0;

// Create a PulseOximeter object
PulseOximeter pox;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    pox.update();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Heart rate: ");
    M5.Lcd.print(pox.getHeartRate());
    M5.Lcd.setCursor(0, 35);
    Time2 = millis();
    M5.Lcd.print("Time: ");
    M5.Lcd.print(Time2);
    M5.Lcd.print(" ms");
    M5.Lcd.setCursor(0, 70);
    unsigned long TimeDif;
    TimeDif = Time2 - Time1;
    Time1 = Time2;
    timeSum = timeSum + TimeDif;
    HRSum = HRSum + pox.getHeartRate();
    i = i + 1;
    if (i == Calibration)
 {
 avgHR = HRSum/(double)i;
 avgHRV = timeSum/i;
  M5.Lcd.clear(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Your avg. resting HR:");
  M5.Lcd.print(avgHR);
  M5.Lcd.setCursor(0, 35);
  M5.Lcd.print("Your avg. resting HRV:");
  M5.Lcd.print(avgHRV);

  }

}

void setup() {
    M5.Lcd.fillScreen(BLACK);
 
    M5.Lcd.setCursor(0, 0);
 
    M5.Lcd.setTextColor(WHITE);
  
    M5.Lcd.setTextSize(2);
    M5.begin(115200);

    M5.Lcd.print("Initializing pulse oximeter..");

    // Initialize sensor
    if (!pox.begin()) {
        M5.Lcd.print("FAILED");
        for(;;);
    } else {
        M5.Lcd.print("SUCCESS");
    }

  // Configure LED
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
}


void loop() {
 pox.update();
 unsigned long time_since_last_reset = millis();
 while ((millis() - time_since_last_reset) < interval2 && i > Calibration)
 {
   pox.update(); 
   if (pox.getHeartRate() > avgHR || (Time2 - Time1) < avgHRV)
 {
   M5.Lcd.setCursor(0, 105);
   M5.Lcd.print("DEVIATION DETECTED!");
   return;
 }
 }

 while ((millis() - time_since_last_reset) >= interval2)
 {
 M5.Lcd.fillScreen(BLACK);
 M5.Lcd.setCursor(0, 0);
 //M5.Lcd.setTextColor(WHITE);
 //M5.Lcd.setTextSize(2); 
 M5.Lcd.print("Session ended.");
 delay (5000);
 exit(0);
 }
}
