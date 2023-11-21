// This program will attempt to read in data from the vehicle, and display text that corresponds to
// certain events that occur.
#include <OBD2UART.h>
#include <MicroLCD.h>

LCD_SH1106 lcd;
COBD obd; 

void reconnect()
{
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

void setup()
{
  lcd.begin();
  lcd.setFontSize(FONT_SIZE_MEDIUM);
  lcd.println("OBD DISPLAY");

  delay(500);
  obd.begin();

  lcd.println();
  lcd.println("Connecting...");
  while (!obd.init());
  lcd.clear();
  lcd.println("Connected!");
}

void loop()
{
  static byte pids[]= {PID_RPM, PID_SPEED, PID_ENGINE_LOAD, PID_THROTTLE};
  static byte index = 0;
  byte pid = pids[index];
  int value;
  // send a query to OBD adapter for specified OBD-II pid
  if (obd.readPID(pid, value)) {
      showData(pid, value); // implement this my own way
  }
  index = (index + 1) % sizeof(pids);

  if (obd.errors >= 2) {
      reconnect();
      setup();
  }
}