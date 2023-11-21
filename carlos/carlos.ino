#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD2UART.h>


//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

long mood;
int faceValue;
int prevFaceValue;

unsigned long currentTime;
unsigned long startTime = millis();
long blinkInterval = 6000;

static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE, PID_FUEL_LEVEL};
static byte index = 0;
byte pid = pids[index];
int value;

void reconnect() {
  display.clear();
  display.setTextSize(2);
  display.print("Reconnecting");
  for (uint16_t i = 0; !obd.init(); i++) {
    if (i == 5) {
      display.clearDisplay();
    }
    delay(3000);
  }
}

void setup(){
    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    drawOpenEyes();
    drawNeutralMouth();
    display.display();
}

void loop(){

    //starting at neutral face
    

    if (obd.readPID(pid, value)) {
        eventHandler(pid, value);
    }
    index = (index + 1) % sizeof(pids);

    // determine face and draw

    if (obd.errors >= 2) {
        reconnect();
        setup();
    }
    
    display.display();
}

void eventHandler(byte pid, int value){
    switch(pid){
        case PID_RPM:
    /* check for high RPM
        if value > 4,000rpm
            mood--
            wait for cooldown until next event
    */
            break;
        case PID_SPEED:
        /* 
        */
            break;
        case PID_DISTANCE:
        /* check for distance
            if  value - startOdo > 5mi
            mood++
            startOdo = value
        */            
            break;
        
        //fuel level above 90%?
        case PID_FUEL_LEVEL:
            break;
    }
}

void drawEyebags() {
    display.drawCircle(30, 30, 12, SH110X_WHITE);
    display.drawCircle(98, 30, 12, SH110X_WHITE);
    display.fillRect(18, 18, 100, 20, SH110X_BLACK);
    drawOpenEyes();
}

void drawHappyMouth() {
    display.fillCircle(64, 25, 25, SH110X_WHITE);    // full smile
    display.fillCircle(64, 23, 25, SH110X_BLACK);
    display.fillRect(38, 20, 53, 25, SH110X_BLACK);  // hide top half of smile
}

void drawNeutralMouth() {
    display.fillRect(55, 45, 15, 3, SH110X_WHITE);
}

void drawUnhappyMouth() {
    display.drawLine(20, 60, 105, 50, SH110X_WHITE);
}

void drawSadMouth() {
    display.fillCircle(64, 69, 25, SH110X_WHITE);
    display.fillCircle(64, 71, 25, SH110X_BLACK);
    display.fillRect(30, 54, 60, 25, SH110X_BLACK);
}

void drawShockedMouth() {
    display.fillCircle(64, 60, 10, SH110X_WHITE);
    display.fillRect(50, 60, 30, 20, SH110X_BLACK);
}

void drawOpenEyes() {
    display.fillCircle(30, 30, 6, SH110X_WHITE);
    display.fillCircle(98, 30, 6, SH110X_WHITE);
}

void drawShockedEyes() {
    display.fillCircle(30, 25, 9, SH110X_WHITE);
    display.fillCircle(98, 25, 9, SH110X_WHITE);
}

void drawClosedEyes() {
    display.fillRect(23, 27, 14, 3, SH110X_WHITE);
    display.fillRect(91, 27, 14, 3, SH110X_WHITE);
}

void clearEyes() {
    display.fillRect(20, 14, 104, 23, SH110X_BLACK);
    display.display();
}

void clearEyebags() {
    display.fillRect(18, 38, 100, 5, SH110X_BLACK);
    display.display();
}

void clearMouth() {
    display.fillRect(0, 43, 120, 25, SH110X_BLACK);
    display.display();
}

void blink() {
    clearEyes();
    drawClosedEyes();
    display.display();
    delay(30);
    clearEyes();
    drawOpenEyes();
}

void drawShockedFace() {
    drawShockedEyes();
    drawShockedMouth();
    display.display();
}
