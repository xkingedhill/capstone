// This program will test if the communications between hte arduino and the vehicle are working.


#include <OBD2UART.h>
#include <MicroLCD.h>

LCD_SH1106 lcd;
COBD obd; 

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

void loop() {
    delay(2000);
    lcd.clear();
    lcd.println("This board is ready to go!");
}