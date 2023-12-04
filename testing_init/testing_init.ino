#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD.h>

//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE};
static byte index = 0;
byte pid = pids[index];
int value;

// Current mood value
int mood = 5;

// Sad = 0
// Unhappy = 1
// Neutral = 2
// Happy = 3
int prevFaceValue, faceValue = 2;

// Causes penalty when reaching this value
const int RPM_LIMIT = 3500;
const int SPEED_LIMIT = 30;

int startDistance, currentDistance, lastDistance, lastSpeed;

unsigned long currentTime, lastRpmTime, lastSpeedingTime, lastBrakeTime, lastSecond, lastBlinkTime, lastLoopTime;

unsigned long blinkInterval = 6000; // 6s

const unsigned long PENALTY_COOLDOWN = 30000; // 30s

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
            if ((value % 10000) > RPM_LIMIT && currentTime - lastRpmTime >= PENALTY_COOLDOWN) {
                decrementMood();
                lastRpmTime = currentTime;
            }
            break;
        
        case PID_DISTANCE:
            currentDistance = value;

            if (currentDistance - lastDistance >= 2) {
                incrementMood();
                lastDistance = currentDistance;
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
    delay(500);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Drive complete!");
    display.display();
    delay(2000);
    display.println();
    display.setTextSize(1);
    display.print("Final mood: ");
    display.display();
    delay(1750);
    switch(faceValue) {
        case 0:
            display.print("Sad");
        case 1:
            display.print("Unhappy");
        case 2:
            display.print("Neutral");
        case 3: 
            display.print("Happy");
    }
    display.display();
    delay(1000);
    display.println();
    display.println();
    display.println("Distance");
    display.print("travelled: ");
    display.display();
    delay(1750);
    display.print(currentDistance - startDistance);
    display.print("mi");
    display.display();
}

void setup(){

    pinMode(LED_BUILTIN, OUTPUT);

    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    drawOpenEyes();
    drawNeutralMouth();
    display.display();

    obd.begin();
    //while (!obd.init());
    digitalWrite(LED_BUILTIN, HIGH); // obd connected

    lastRpmTime = lastSpeedingTime = lastSecond = millis();
    //obd.readPID(PID_DISTANCE, startDistance);
    lastDistance = startDistance;
}

void loop(){

    currentTime = millis();

    if (currentTime - lastLoopTime >= 3000) {
        
        mood = random(10);

        if (currentTime - lastBlinkTime >= blinkInterval) {
            blink();
            lastBlinkTime = currentTime;
            blinkInterval = random(2000, 12000);
        }

        if (mood > 5) {
            clearMouth();
            drawHappyMouth();
            faceValue = 4;

        }
        else if (mood == 5) {
            clearMouth();
            drawNeutralMouth();
            faceValue = 3;
        }
        else if (mood == 4 || mood == 3) {
            clearMouth();
            drawUnhappyMouth();
            faceValue = 2;
        }
        else if (mood < 3) {
            clearMouth();
            drawSadMouth();
            faceValue = 1;
        }

        if (faceValue != prevFaceValue) {
            display.display();
            prevFaceValue = faceValue;
        }
        lastLoopTime = currentTime;
    }
}