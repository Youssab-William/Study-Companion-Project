#include <M5Core2.h>
#include <SPIFFS.h>
#include <cmath>
#include <EEPROM.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

int address_stretch = 1;
int address_vibration_threshold = 5;
int size = 10;

int additionaldelay = 0;

float accX = 0.0F;  
float accY = 0.0F;  
float accZ = 0.0F;

float acc2X = 0.0F;  
float acc2Y = 0.0F;  
float acc2Z = 0.0F;

float reference_angle = 0.0F;
float maximum_angle = 0.0F;

int highscore = 0;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float maxgyroX = 0.0F;
float maxgyroY = 0.0F;
float maxgyroZ = 0.0F;

float acceleration = 0.0F;
float shaking_acceleration = 0.0F;

bool killUser = false;

float time_remaining = 0.0F;

int minutes_remaining = 0;
int seconds_remaining = 0;

bool focus_session = true;
bool was_pressed = false;

int max_i;
int max_j;

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
void stressFunctionSetup()
{
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
void stressFunctionLoop()
{
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
void checkHealthySitting()
{
    M5.IMU.getAccelData(&acc2X, &acc2Y, &acc2Z);  
    acc2X *= 90;
    acc2Y *= 90;
    acc2Z *= 90;
    while(abs(acc2X) > 30 || abs(acc2Y) < 70)
    {
      M5.IMU.getAccelData(&acc2X, &acc2Y, &acc2Z);  
      M5.Axp.SetLDOEnable(3, true);
      acc2X *= 90;
      acc2Y *= 90;
      acc2Z *= 90;
      delay(10);
      additionaldelay += 10;    
    }
    M5.Axp.SetLDOEnable(3, false);

}
void macroActivitySetup()
{
  
  // shaking calibration section. Due to too much error from the IMU sensor, we could not implement an automatic movement detection section.
  // This function is replaced by an activity that sollicitates user action in an awakening way: Violently shaking the device.
  
  M5.Lcd.clear(BLACK);

  M5.Lcd.setCursor(0, 20); 
  if (was_pressed){
    was_pressed = false;
    M5.Lcd.printf("Please shake the device as you normally would");

    for (int i = 0 ; i <= 500 ; ++i){
      M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

      if (abs(gyroX) > maxgyroX) maxgyroX = abs(gyroX);
      if (abs(gyroY) > maxgyroY) maxgyroY = abs(gyroY);
      if (abs(gyroZ) > maxgyroZ) maxgyroZ = abs(gyroZ);

      shaking_acceleration = sqrt( maxgyroX * maxgyroX + maxgyroY * maxgyroY + maxgyroZ * maxgyroZ );

      EEPROM.writeInt(address_vibration_threshold, shaking_acceleration);

      EEPROM.commit();

      delay(10);


    }
  }


}
void macroActivityLoop()
{
  if (focus_session){
      shaking_acceleration = EEPROM.readInt(address_vibration_threshold);

      max_i = 10; //Change max i for the study session duration (in seconds)

      max_i *= 2; 

      for (int i = 0; i <= max_i; ++i){
        M5.Lcd.clear(BLACK);

        M5.Lcd.setCursor(0, 20);  
    
        M5.Lcd.printf("30 Minutes Focus Session.");

        M5.Lcd.setCursor(0, 42);  
    
        M5.Lcd.printf("Don't forget to shake me  if you get up!");

        M5.Lcd.setCursor(0, 70);  

        time_remaining = (max_i-i)/2;

        minutes_remaining = int(time_remaining / 60);

        seconds_remaining = int( time_remaining - minutes_remaining * 60 );
    
        M5.Lcd.printf("Time remaining:");

        M5.Lcd.setCursor(0, 92);

        M5.Lcd.printf("%i min and %i s", minutes_remaining , seconds_remaining );

        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

        acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );
        
        


        
        if (acceleration > shaking_acceleration){
          M5.Lcd.clear(BLACK);
          
          M5.Lcd.setCursor(0, 20);

          M5.Lcd.printf("Pause initiated! You have 5 minutes to come back to your desk, or touch any button to resume studying");

          max_j = 5; //Change max j for the pause duration (in seconds)

          max_j *= 20;

          for (int j = 0 ; j<= max_j ; ++j){
            M5.update();

            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) break;

            M5.Lcd.setCursor(0, 122); 

            M5.Lcd.printf("Time remaining:");

            time_remaining = (max_j-j)/20;

            minutes_remaining = int(time_remaining / 60);

            seconds_remaining = int( time_remaining - minutes_remaining * 60 );

            M5.Lcd.setCursor(0, 142);

            M5.Lcd.printf("%i min and %i s", minutes_remaining , seconds_remaining );

            delay(50);
            
          }

        } 

        else{
          checkHealthySitting();

          if (additionaldelay > 500){
            i += int(additionaldelay / 500);
            additionaldelay %= 500 ; 
          }
          
          delay(500 - additionaldelay);
          
          additionaldelay = 0;

          if (i >= max_i)
            killUser = true;
        }
        }

      if (killUser == true){
        killUser = false;

        M5.Lcd.clear(BLACK);

        M5.Lcd.setCursor(0, 20);
        
        M5.Lcd.printf("It s been 30 minutes without you moving.");

        M5.Lcd.setCursor(0, 72);

        M5.Lcd.printf("Shake the device to stop the alert.");

        while (true){
          M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Axp.SetLDOEnable(3, false);  //Close the vibration.  

            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 62);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }

          delay(200);
          
          M5.Axp.SetLDOEnable(3, false);  //Close the vibration.  
          
          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 62);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }

          delay(200);
          
          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 82);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }
        }

        while (true){
          M5.update();
          if (M5.BtnC.wasPressed()) {
            focus_session = false;
            break;
          }
          

          else if (M5.BtnA.wasPressed()) break;
          }
      }
    }
    
    else{
      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("Attach the device to your upper chest and stand still. ");

      M5.Lcd.setCursor(0, 82);

      M5.Lcd.printf("You can stretch forward when you feel a vibration. ");

      M5.Lcd.setCursor(0, 122);

      M5.Lcd.printf("Whenever you feel comfortable, get up.");

      M5.Lcd.setCursor(0, 162);

      M5.Lcd.printf("Once you're ready, touch any button.");

      while (true){
        M5.update();
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) break;
      }

      for (int i=0 ; i<= 500 ; ++i){
        M5.IMU.getAccelData(&accX, &accY, &accZ); 

        if (accZ >= 0) reference_angle = -90 + (accY) *90;

        else reference_angle = 90 - accY *90 ;

        M5.Lcd.setCursor(0, 102);

       M5.Lcd.printf("Reference angle: %f", reference_angle);
        
        delay(10);
      }

      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("You can stretch now! ");

      M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

      delay(1000);

      M5.Axp.SetLDOEnable(3, false);  //Close the vibration.

      delay(5000);

      do{
        M5.IMU.getAccelData(&accX, &accY, &accZ);

        accY = 90 - accY *90 - reference_angle;

        if (accY > maximum_angle) maximum_angle = accY;
      }
      while ( accY >= (reference_angle + 5 ) );
      
      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("Good stretch! Your angle was %5.2f ", maximum_angle);

      M5.Lcd.setCursor(0, 82);

      highscore = EEPROM.readInt(address_stretch);

      if ( (int)maximum_angle > highscore) {
        M5.Lcd.printf("Congrats! New highscore: %d", (int)maximum_angle);

        EEPROM.writeInt(address_stretch, (int)maximum_angle);

        EEPROM.commit();
      }

      else {
        M5.Lcd.printf("Your highscore: %d", highscore);

        M5.Lcd.setCursor(0, 102);

        M5.Lcd.printf("This session's score: %d", (int)maximum_angle);
      }

      M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

      delay(1000);

      M5.Axp.SetLDOEnable(3, false);  //Close the vibration.

      M5.Lcd.setCursor(0, 122);

      M5.Lcd.printf("Study: leftmost button. Stretch: rightmost button");

      while (true){
        M5.update();
        if (M5.BtnC.wasPressed()) break;

        else if (M5.BtnA.wasPressed()) {
          focus_session = true;
          break;
        }
    }


    }


}
void setup() {

  //stressFunctionSetup();
  M5.begin();        

  M5.IMU.Init(); 

  EEPROM.begin(size);

  M5.Lcd.fillScreen(BLACK); 

  M5.Lcd.setTextColor(WHITE, BLACK);  

  M5.Lcd.setTextSize(2);  

  M5.Lcd.setCursor(0, 20);  

  M5.Lcd.print("New user? Leftmost button for Y, rightmost button for N.");

  while (true){
    M5.update();
    if (M5.BtnC.wasPressed()) break;

    else if (M5.BtnA.wasPressed()) {
      for (int i = 0 ; i < 10 ; ++i ) {
        EEPROM.writeInt(i, 0);
        EEPROM.commit();
        was_pressed = true;
      }
      break;

    }
  }
  
  macroActivitySetup();
  
}


void loop() {
  macroActivityLoop();

  }
