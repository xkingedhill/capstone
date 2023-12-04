#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD.h>


//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
COBD obd;

long mood;
int faceValue;
int prevFaceValue = 0;

unsigned long currentTime;
unsigned long lastLoopTime, lastBlinkTime;
long blinkInterval = 6000;

void setup(){

    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    drawOpenEyes();
    //drawEyebags();
    display.display();
    lastLoopTime = lastBlinkTime = millis();

    obd.begin();
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

