// Test Code for LCD Screen
//
// @author Email: info@vortexit.co.nz 
//       Web: www.vortexit.co.nz

#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup() 
{
  lcd.init();
  lcd.clear();
  lcd.backlight();// Turn on backlight
  lcd.setCursor(0,0);
  lcd.print("Testing LCD 16x2");
  lcd.setCursor(0,1);
  lcd.print("0123456789ABCDEF");
}
  
void loop()
{
  
}
