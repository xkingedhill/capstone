# 1 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
/*************************************************************************
* OBD-II based performance timer and logger
* Distributed under GPL v2.0
* Copyright (c) 2014 Stanley Huang <stanleyhuangyc@gmail.com>
* All rights reserved.
*************************************************************************/

# 9 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 10 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 11 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 12 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 13 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 14 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2
# 15 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2



# 19 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino" 2

// logger states




static uint32_t lastFileSize = 0;
static int lastSpeed = -1;
static uint32_t lastSpeedTime = 0;
static int speed = 0;
static uint32_t distance = 0;
static uint16_t fileIndex = 0;
static uint32_t startTime = 0;





static byte stage = 0;






static uint16_t times[4] = {0};




class COBDLogger : public COBD

{
public:
    COBDLogger():state(0) {}
    void setup()
    {
# 65 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
        do {
            showStates();
        } while (!init());

        state |= 0x2;

        showStates();
# 85 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
        initTimerScreen();
    }
    void loop()
    {
        static byte index = 0;
        static byte index2 = 0;
        static byte index3 = 0;

        timerLoop();
# 104 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
        if (errors >= 2) {
            reconnect();
        }
    }
# 167 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
private:
    void dataIdleLoop()
    {
        if (state & 0x20) return;

        if (getState() == OBD_CONNECTED)
            return;

        // called while initializing
        char buf[10];
        unsigned int t = (millis() - startTime) / 1000;
        sprintf(buf, "%02u:%02u", t / 60, t % 60);
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(97, 7);
        lcd.print(buf);
    }
    void timerLoop()
    {
        uint32_t elapsed = millis() - startTime;
        uint16_t n;

        uint32_t dataTime;

        int speed;
        if (!readPID(0x0D, speed))
            return;

        dataTime = millis();
# 205 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
        lcd.setFontSize(FONT_SIZE_XLARGE);
        // estimate distance
        distance += (uint32_t)(speed + lastSpeed) * (dataTime - lastSpeedTime) / 2 / 3600;

        if (lastSpeed != speed) {
            lcd.setCursor(0, 4);
            lcd.printInt((unsigned int)speed % 1000, 3);
            lastSpeed = speed;
        }

        lastSpeedTime = dataTime;

        if (stage == 1) {
            if (speed > 0) {
                stage = 2;
                startTime = lastSpeedTime;
# 230 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
                lastSpeed = 0;
                distance = 0;

                memset(times, 0, sizeof(times));

                initTimerScreen();
            }
        } else if (stage == 2) {
            // display elapsed time (mm:ss:mm)
            n = elapsed / 1000;
            if (n < 100) {
                lcd.setCursor(0, 0);
                lcd.printInt(n, 2);
                n = (elapsed % 1000) / 100;
                lcd.setFontSize(FONT_SIZE_MEDIUM);
                lcd.setCursor(32, 1);
                lcd.write('.');
                lcd.write('0' + n);
            }
            if (times[2] == 0 && speed >= 200 /* kph */) {
                times[2] = elapsed / 100;
                stage = 0;
                lcd.clear(0, 0, 128, 24);
                showTimerResults();
                lcd.setFontSize(FONT_SIZE_MEDIUM);
                lcd.setCursor(0, 0);
                lcd.print("DONE!");
            } else if (times[1] == 0 && speed >= 100 /* kph */) {
                times[1] = elapsed / 100;
                showTimerResults();
            } else if (times[0] == 0 && speed >= 60 /* kph */) {
                times[0] = elapsed / 100;
                showTimerResults();
            } else if (speed == 0) {
                // speed go back to 0
                stage = 0;
            }
            if (distance > 0) {
                lcd.setFontSize(FONT_SIZE_SMALL);
                lcd.setCursor(62, 6);
                if (distance >= 400) {
                    lcd.printInt(400, 3);
                    if (!times[3]) {
                        times[3] = elapsed / 100;
                        showTimerResults();
                    }
                } else {
                    lcd.printInt(distance, 3);
                }
            }
# 290 "C:\\Users\\Nhill\\OneDrive\\Documents\\College_Stuff\\Capstone\\ArduinoOBD-master\\nanotimer\\nanotimer.ino"
        } else {
            if (speed == 0) {
                stage = 1;
                initTimerScreen();
                lcd.setFontSize(FONT_SIZE_MEDIUM);
                lcd.setCursor(0, 0);
                lcd.println(" GET");
                lcd.println("READY");
                delay(500);
            }
        }
    }
    void reconnect()
    {



        lcd.clear();
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        lcd.print("Reconnecting");
        startTime = millis();
        state &= ~0x2;
        state |= 0x20;
        //digitalWrite(SD_CS_PIN, LOW);
        for (int i = 0; !init(); i++) {
            if (i == 10) lcd.clear();
        }
        state &= ~0x20;
        fileIndex++;
        setup();
    }
    byte state;

    // screen layout related stuff
    void showStates()
    {
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(0, 3);
        if (state & 0x2) {
            lcd.println("OBD connected!   ");
        } else {
            lcd.println("Connecting OBD...");
        }
    }
    void showTimerResults()
    {
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(56, 0);
        lcd.print(" 0~60:  --");
        lcd.setCursor(56, 2);
        lcd.print("0~100:  --");
        lcd.setCursor(56, 4);
        lcd.print("0~200:  --");
        lcd.setCursor(56, 6);
        lcd.print(" 400m:  --");
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        char buf[8];
        if (times[0]) {
            sprintf(buf, "%2d.%1d", times[0] / 10, times[0] % 10);
            Serial.println(times[0]);
            lcd.setCursor(92, 0);
            lcd.print(buf);
        }
        if (times[1]) {
            sprintf(buf, "%2d.%1d", times[1] / 10, times[1] % 10);
            Serial.println(buf);
            lcd.setCursor(92, 2);
            lcd.print(buf);
        }
        if (times[2]) {
            sprintf(buf, "%2d.%1d", times[2] / 10, times[2] % 10);
            Serial.println(buf);
            lcd.setCursor(92, 4);
            lcd.print(buf);
        }
        if (times[3]) {
            sprintf(buf, "%2d.%1d", times[3] / 10, times[3] % 10);
            Serial.println(buf);
            lcd.setCursor(92, 6);
            lcd.print(buf);
        }
    }
    void initTimerScreen()
    {
        lcd.clear();
        showTimerResults();
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(24, 7);
        lcd.print("km/h");
    }
};

static COBDLogger logger;

void setup()
{




    lcd.begin();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.println("PerformanceBox");

    logger.begin();




    logger.setup();
}

void loop()
{
    logger.loop();
}
