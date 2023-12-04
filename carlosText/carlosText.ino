#include <Wire.h>
#include <OBD.h>
#include <MicroLCD.h>

LCD_SH1106 lcd;
COBD obd;

static byte pids[]= {PID_RPM, PID_SPEED, PID_DISTANCE};
static byte index = 0;
byte pid = pids[index];
int value;

int mood = 5;

// Causes penalty when reaching these limits
const int RPM_LIMIT = 3500;
const int SPEED_LIMIT = 30;

int startDistance, lastDistance, currentDistance, lastSpeed;

unsigned long currentTime, lastRpmTime, lastSpeedingTime, lastBrakeTime, lastSecond;

const unsigned long PENALTY_COOLDOWN = 30000; // 30s

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

void eventHandler(byte pid, int value) {
    switch(pid){
        case PID_RPM:
            lcd.setCursor(64, 0);
            lcd.setFontSize(FONT_SIZE_XLARGE);
            lcd.printInt((unsigned int)value, 4);
            if ((value % 10000) > RPM_LIMIT && currentTime - lastRpmTime >= PENALTY_COOLDOWN) {
                decrementMood();
                lastRpmTime = currentTime;
            }
            break;
        
        case PID_DISTANCE:
            currentDistance = value;

            lcd.setCursor(80, 5);
            lcd.setFontSize(FONT_SIZE_MEDIUM);
            lcd.printInt((unsigned int)currentDistance, 6);
            if (currentDistance - lastDistance >= 2) {
                incrementMood();
                lastDistance = currentDistance;
            }
            break;

        case PID_SPEED:
            lcd.setCursor(0, 0);
            lcd.setFontSize(FONT_SIZE_XLARGE);
            lcd.printInt((unsigned int)value % 1000, 3);
            //check for speeding
            if (value > SPEED_LIMIT && currentTime - lastSpeedingTime >= PENALTY_COOLDOWN) {
                decrementMood();
                lastSpeedingTime = currentTime;
            }
            //check for braking
            if (currentTime - lastSecond >= 1000) { // a second has passed
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
    lcd.clear();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("Drive complete");
    lcd.println();
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.print("Final mood: ");
    lcd.printInt(mood);
    lcd.println();
    lcd.println("Distance");
    lcd.print("travelled: ");
    lcd.printInt(currentDistance - startDistance, 3);
    lcd.print("mi");
}

void initScreen(){
    lcd.clear();
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.setCursor(24, 3);
    lcd.print("mph");
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.setCursor(0, 7);
    lcd.print("Mood");
    lcd.setCursor(110, 3);
    lcd.print("rpm");
    lcd.setCursor(80, 7);
    lcd.print("Distance");
}

void setup() {
    lcd.begin();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.println("CAR-LOS");

    delay(500);
    obd.dataMode = 1;
    obd.begin();

    lcd.println();
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.println("Connecting...");
    while (!obd.init());

    initScreen();

    lastRpmTime = lastSpeedingTime = lastSecond = millis();
    obd.readPID(PID_DISTANCE, startDistance);
    lastDistance = startDistance;
}

void loop() {

    currentTime = millis();

    if (obd.readPID(pid, value)) {
        eventHandler(pid, value);
    }
    index = (index + 1) % sizeof(pids);
    pid = pids[index];

    //print value of mood and elapsed time (for debugging)
    lcd.setCursor(25, 5);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt(mood, 2);

    if (obd.errors >= 2) {
        //obd has disconnected
        resultsScreen();
        exit(1);
    }
}