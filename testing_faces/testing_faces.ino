#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OBD2UART.h>


//Make Sure Address is Correct
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
long mood;

void setup(){
    display.begin(OLED_ADDRESS, true);
    display.clearDisplay();
    drawOpenEyes();
    display.display();
}

void loop(){

    clearMouth();
    mood = random(10);

    /*if (mood % 2 == 0) {
        blink();
    }*/

    if (mood > 5) {
        drawHappyMouth();
    }
    else if (mood == 5) {
        drawNeutralMouth();
    }
    else if (mood == 4 || mood == 3) {
        drawUnhappyMouth();
    }
    else if (mood < 3) {
        drawSadMouth();
    }

    display.display();

    delay(3000);

}

void drawEyebags() {
    display.drawCircle(30, 30, 12, SH110X_WHITE);
    display.drawCircle(98, 30, 12, SH110X_WHITE);
    display.fillRect(18, 18, 100, 20, SH110X_BLACK);
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
    display.fillCircle(64, 64, 25, SH110X_WHITE);
    display.fillCircle(64, 66, 25, SH110X_BLACK);
    display.fillRect(30, 49, 60, 25, SH110X_BLACK);
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
    display.fillRect(20, 14, 104, 21, SH110X_BLACK);
    display.display();
}

void clearEyebags() {
    display.fillRect(18, 38, 100, 5, SH110X_BLACK);
    display.display();
}

void clearMouth() {
    display.fillRect(0, 43, 120, 20, SH110X_BLACK);
    display.display();
}

void blink() {
    clearEyes();
    drawClosedEyes();
    display.display();
    delay(10);
    clearEyes();
    drawOpenEyes();
}

void drawShockedFace() {
    drawShockedEyes();
    drawShockedMouth();
    display.display();
}

