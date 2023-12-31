#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD.h>

//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

// Current mood value
int mood = 5;

// Sad = 0
// Unhappy = 1
// Neutral = 2
// Happy = 3
int lastFaceValue, currentFaceValue = 2;

int lastDistance, lastSpeed;

unsigned long currentTime;

/*** DRAWING FUNCTIONS ***/

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

void drawOpenEyes() {
    display.fillCircle(30, 30, 6, SH110X_WHITE);
    display.fillCircle(98, 30, 6, SH110X_WHITE);
}

void drawClosedEyes() {
    display.fillRect(23, 27, 14, 3, SH110X_WHITE);
    display.fillRect(91, 27, 14, 3, SH110X_WHITE);
}

void clearEyes() {
    display.fillRect(20, 14, 104, 23, SH110X_BLACK);
}

void clearMouth() {
    display.fillRect(0, 43, 120, 25, SH110X_BLACK);
}

void blink() {
    clearEyes();
    display.display();
    drawClosedEyes();
    display.display();
    delay(30);
    clearEyes();
    display.display();
    drawOpenEyes();
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

    const int RPM_LIMIT = 3000;
    const int SPEED_LIMIT = 50;
    const int PENALTY_COOLDOWN = 30000; // 30s

    static unsigned long lastRpmTime, lastSpeedingTime, lastBrakeTime, lastSecond;

    switch(pid){
        case PID_RPM:
            if ((value % 10000) > RPM_LIMIT && currentTime - lastRpmTime >= PENALTY_COOLDOWN) {
                decrementMood();
                lastRpmTime = currentTime;
            }
            break;
        
        case PID_DISTANCE:
            if (value - lastDistance >= 1) { 
                incrementMood();
                lastDistance = value;
            }
            break;
        
        case PID_SPEED:
            if (value > SPEED_LIMIT && currentTime - lastSpeedingTime >= PENALTY_COOLDOWN) {
                decrementMood();
                lastSpeedingTime = currentTime;
            }

            if (currentTime - lastSecond >= 1000) {
                if (value - lastSpeed <= -9 && currentTime - lastBrakeTime >= PENALTY_COOLDOWN) {
                    decrementMood();
                    lastBrakeTime = currentTime;
                }
                lastSecond = currentTime;
                lastSpeed = value;
            }
            break;
    }
}

void resultsScreen() {
    display.clearDisplay();
    display.display();
    display.setTextColor(SH110X_WHITE);
    delay(500);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println(F("Drive complete!"));
    display.display();
    delay(2000);
    display.println();
    display.println(F("Final mood: "));
    display.println();
    display.display();
    delay(1750);
    display.setTextSize(2);
    switch(currentFaceValue) {
        case 0:
            display.print(F("Sad"));
            break;
        case 1:
            display.print(F("Unhappy"));
            break;
        case 2:
            display.print(F("Neutral"));
            break;
        case 3: 
            display.print(F("Happy"));
            break;
    }
    display.display();
}

void setup(){

    pinMode(LED_BUILTIN, OUTPUT);

    display.begin(OLED_ADDRESS, true);
    delay(500);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SH110X_WHITE);
    display.print(F("Connecting..."));
    display.display();

    obd.begin();
    while (!obd.init());
    digitalWrite(LED_BUILTIN, HIGH); // obd connected

    //start on neutral face
    display.clearDisplay();
    drawOpenEyes();
    drawNeutralMouth();
    display.display();

    //get initial times and distances
    obd.readPID(PID_DISTANCE, lastDistance);
}

void loop(){

    //pids to cycle through
    static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE};
    static byte index = 0;
    byte pid = pids[index];
    int value;

    static unsigned long lastBlinkTime;
    static int blinkInterval = 6000; // 6s
    
    //get current time
    currentTime = millis();

    //read from vehicle and send to eventHandler
    if (obd.readPID(pid, value)) {
        eventHandler(pid, value);
    }
    index = (index + 1) % sizeof(pids);
    pid = pids[index];
    

    if (currentTime - lastBlinkTime >= blinkInterval) {
        blink();
        lastBlinkTime = currentTime;
        blinkInterval = random(2000, 12000);
    }

    // determine face and draw
    if (mood > 5) {
        clearMouth();
        drawHappyMouth();
        currentFaceValue = 3;
    }
    else if (mood == 5) {
        clearMouth();
        drawNeutralMouth();
        currentFaceValue = 2;
    }
    else if (mood == 4 || mood == 3) {
        clearMouth();
        drawUnhappyMouth();
        currentFaceValue = 1;
    }
    else if (mood < 3) {
        clearMouth();
        drawSadMouth();
        currentFaceValue = 0;
    }

    //if face needs to be updated, do so
    if (currentFaceValue != lastFaceValue) {
        display.display();
        lastFaceValue = currentFaceValue;
    }
    
    //check if drive has ended
    if (obd.errors >= 2) {
        digitalWrite(LED_BUILTIN, LOW);
        resultsScreen();
        exit(1);
    }
}