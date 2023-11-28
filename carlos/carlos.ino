#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD.h>

//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE, PID_FUEL_LEVEL};
static byte index = 0;
byte pid = pids[index];
int value;

// Current mood value
int mood = 5;

// Sad = 0
// Unhappy = 1
// Neutral = 2
// Happy = 3
int startFace, currentFace = 2;

// Causes penalty when reaching this RPM
int rpmLimit = 2000;

unsigned long startTime, currentTime, lastRpmTime, lastSpeedingTime;

const unsigned long blinkCooldown = 6000; // 6s
const unsigned long rpmCooldown, speedingCooldown = 30000; // 30s

/*** DRAWING FUNCTIONS ***/

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

/*** END OF DRAWING FUNCTIONS ***/

void decrementMood() {
    mood--;
    if (mood < 0) {
        mood = 0;
    }
}

void incrementMood() {
    mood++;
    if (mood > 10) {
        mood = 10;
    }
}

void eventHandler(byte pid, int value){
    switch(pid){
        case PID_RPM:
            if (((unsigned int)value % 10000) > rpmLimit
                && currentTime - lastRpmTime >= rpmCooldown) {
                decrementMood();
                lastRpmTime = currentTime;
            }
            break;
        
        case PID_SPEED:
            break;
        /*
            //check for speeding
            
            break;
        case PID_DISTANCE:
        check for distance
            if  value - startOdo > 5mi
            mood++
            startOdo = value
                   
            break;
        
        //fuel level above 90%?
        case PID_FUEL_LEVEL:
            break;
        */
    }
}

void reconnect() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Reconnecting");
    for (uint16_t i = 0; !obd.init(); i++) {
    if (i == 5) {
        display.clearDisplay();
    }
    delay(3000);
    }
}

void setup(){

    pinMode(LED_BUILTIN, OUTPUT);

    obd.dataMode = 1;
    obd.begin();
    while (!obd.init());
    digitalWrite(LED_BUILTIN, HIGH); // connection established

    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.print("CAR-LOS");
    display.display();
    delay(1000);
    display.setCursor(0,32);
    display.setTextSize(1);
    display.println("Connected!");
    display.println("Starting CAR-LOS...");
    display.display();

    delay(750);

    display.clearDisplay();
    drawOpenEyes();
    drawNeutralMouth();
    display.display();
    }

void loop(){
    
    //starting at neutral face

    /*if (obd.readPID(pid, value)) {
        eventHandler(pid, value);
    }
    index = (index + 1) % sizeof(pids);

    // determine face and draw
    
    //display.display();*/
}