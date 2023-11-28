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

/*
4: Happy (> 5)
3: Neutral (== 5)
2: Unhappy (4, 3)
1: Sad (< 3)
*/
int faceValue = 3;


unsigned long currentTime;
unsigned long startTime;
unsigned long rpmInterval = 30000; // 30s

void reconnect() {
    lcd.clear();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("Reconnecting");
    for (uint16_t i = 0; !obd.init(); i++) {
    if (i == 5) {
        lcd.clear();
    }
    delay(3000);
    }
}

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
            if (((unsigned int)value % 10000) > 2000 && currentTime - startTime >= rpmInterval) {
                decrementMood();
                startTime = currentTime;
            }
            break;
        
        case PID_DISTANCE:
            lcd.setCursor(64, 5);
            lcd.setFontSize(FONT_SIZE_XLARGE);
            lcd.printInt((unsigned int)value, 5);
            break;

        case PID_SPEED:
            //check for speeding
            break;
        
        //fuel level above 90%?
        //only offer this point once
        case PID_FUEL_LEVEL:
            break;
    }
}

void initScreen(){
    lcd.clear();
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.setCursor(0, 7);
    lcd.print("Mood");
    lcd.setCursor(110, 3);
    lcd.print("rpm");
    // ADD DISTANCE
}

void setup() {

    startTime = millis();

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
}

void loop() {

    currentTime = millis();

    if (obd.readPID(pid, value)) {
        eventHandler(pid, value);
    }
    index = (index + 1) % sizeof(pids);

    //print value of mood and elapsed time (for debugging)
    lcd.setCursor(18, 5);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt(mood, 2);

    // determine moodText and print


    if (obd.errors >= 2) {
        reconnect();
        setup();
    }
}
