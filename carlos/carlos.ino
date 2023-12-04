#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD.h>

//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

int startDistance, currentDistance, lastDistance, lastSpeed;

unsigned long currentTime;

unsigned int blinkInterval = 6000; // 6s

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
}

void clearEyebags() {
    display.fillRect(18, 38, 100, 5, SH110X_BLACK);
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

void drawShockedFace() {
    drawShockedEyes();
    drawShockedMouth();
    display.display();
}

/*** END OF DRAWING FUNCTIONS ***/

void decrementMood(int mood) {
    mood--;
    if (mood < 0) {
        mood = 0;
    }
}

void incrementMood(int mood) {
    mood++;
    if (mood > 10) {
        mood = 10;
    }
}

void eventHandler(byte pid, int value, int mood){

    static const int RPM_LIMIT = 3500;
    static const int SPEED_LIMIT = 30;
    static const unsigned int PENALTY_COOLDOWN = 30000; // 30s

    static unsigned long lastRpmTime, lastSpeedingTime, lastBrakeTime, lastSecond;


    switch(pid){
        case PID_RPM:
            if ((value % 10000) > RPM_LIMIT && currentTime - lastRpmTime >= PENALTY_COOLDOWN) {
                decrementMood(mood);
                lastRpmTime = currentTime;
            }
            break;
        
        case PID_DISTANCE:
            currentDistance = value;

            if (currentDistance - lastDistance >= 1) { 
                incrementMood(mood);
                lastDistance = currentDistance;
            }
            break;
        
        case PID_SPEED:
            if (value > SPEED_LIMIT && currentTime - lastSpeedingTime >= PENALTY_COOLDOWN) {
                decrementMood(mood);
                lastSpeedingTime = currentTime;
            }

            if (currentTime - lastSecond >= 1000) {
                if (value - lastSpeed <= -9 && currentTime - lastBrakeTime >= PENALTY_COOLDOWN) {
                    decrementMood(mood);
                    lastBrakeTime = currentTime;
                }
                lastSecond = currentTime;
                lastSpeed = value;
            }
            break;
    }
}

void resultsScreen(int currentFaceValue) {
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
    display.print(F("Final mood: "));
    display.display();
    delay(1750);
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
    delay(1000);
    display.println();
    display.println();
    display.println(F("Distance"));
    display.print(F("travelled: "));
    display.display();
    delay(1750);
    display.print(currentDistance - startDistance);
    display.print(F("mi"));
    display.display();
}

void setup(){

    pinMode(LED_BUILTIN, OUTPUT);

    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SH110X_WHITE);
    display.print(F("Connecting..."));
    display.display();

    obd.begin();
    //while (!obd.init());
    digitalWrite(LED_BUILTIN, HIGH); // obd connected

    display.clearDisplay();
    drawOpenEyes();
    drawNeutralMouth();
    display.display();

    obd.readPID(PID_DISTANCE, startDistance);
    lastDistance = startDistance;
}

void loop(){

    static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE};
    static byte index = 0;
    byte pid = pids[index];
    int value;

    static long lastBlinkTime;
    
    currentTime = millis();

    // Current mood value
    static int mood = 5;

    // Sad = 0
    // Unhappy = 1
    // Neutral = 2
    // Happy = 3
    static int lastFaceValue, currentFaceValue = 2;

    if (obd.readPID(pid, value)) {
        eventHandler(pid, value, mood);
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

    if (currentFaceValue != lastFaceValue) {
        display.display();
        lastFaceValue = currentFaceValue;
    }
    
    if (obd.errors >= 2) {
        //drive has ended
        digitalWrite(LED_BUILTIN, LOW);
        resultsScreen(currentFaceValue);
        exit(1);
    }
}