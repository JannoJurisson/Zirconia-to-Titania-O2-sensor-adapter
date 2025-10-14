#include <SPI.h>
#include <I2C.h>  // I2C libary with timeout
#include <MCP41_Simple.h>
#include <LiquidCrystal_I2C.h>
#include <avr/wdt.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

MCP41_Simple MyPot;

// ===== Tuning Parameters =====

// The maximum value for  digital potentiometer.
const float MAX_POT_VALUE = 255.0;
// The ADC value where the sharp transition should happen in raw ADC output
const float TRANSITION_POINT_ADC = 92.0;
// The steepness of the curve. Higher value = sharper transition.
const float STEEPNESS = 0.2;

int VoltageInPin = A0;
int InputADC = 0;
int potValue = 0;

int MainboardReset = 0;

unsigned long loopmillis = 0;
unsigned long measuremillis = 0;
const uint8_t CS_PIN = 10;

float voltage = 1.23;


void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT);
  pinMode(VoltageInPin, INPUT);

  // Construct an instance of the MCP4XXX to manage the digipot.
  MyPot.begin(CS_PIN);

  MainboardReset = EEPROM.read(800);

  lcd.init();  // initialize the lcd
  lcd.backlight();

  Serial.println("------------------------------------------------------------------");
  Serial.println("--------Zirconia to Titania oxygen sensor signal converter--------");
  Serial.println("------------------------------------------------------------------");
  Serial.println();
  Serial.println();
  Serial.println("Zirconia sensor input : 0V-0.9V");
  Serial.println("Titania sensor resistance output : 1Kohm-100Kohm");
  Serial.println();

  MainboardReset++;
  if (MainboardReset == 255) {
    EEPROM.write(800, 0);
  }
  EEPROM.write(800, MainboardReset);
  Serial.print("Mainboard reset counter - ");
  Serial.println(MainboardReset);

  wdt_enable(WDTO_8S);  // enable watchdog


  //EEPROM.write(800, 0);
  //   pot->set(0);
}

void loop() {
  if (millis() - loopmillis >= 500) {

    digitalWrite(5, !digitalRead(5));  // Heartbeat LED
    loopmillis = millis();
    wdt_reset();
  }



  if (millis() - measuremillis >= 50) {
    InputADC = analogRead(VoltageInPin);



    Serial.print("ADC in =");
    Serial.println(InputADC);

    // ======== Create a 'bar' on LCD ==========
        if (InputADC > 192) {
      InputADC = 192;
    }
    int bar = InputADC / 12;

    for (int h = 0; h < bar; h++) {
      lcd.setCursor(h, 0);
      lcd.print("\xff");
    }
    for (int h = 15; h >= bar; h--) {
      lcd.setCursor(h, 0);
      lcd.print(" ");
    }

    lcd.setCursor(8, 1);
    lcd.print("V= ");
    lcd.setCursor(11, 1);
    lcd.print("   ");

    int adcValue = analogRead(VoltageInPin);
    voltage = adcValue * 44;
    voltage = voltage / 10000;

    lcd.setCursor(11, 1);
    lcd.print(voltage);


    Serial.print("Voltage in =");
    Serial.println(voltage);


    /*
    // logistic function formula to mimic narrowband Titania oxygen sensor signal 
    // the resistance should rise sharply after 14.7:1 A:F ratio (at ~0.45V from zirconia sensor)
    */
    float exponent = -STEEPNESS * (adcValue - TRANSITION_POINT_ADC);  
    float calculatedValue = MAX_POT_VALUE / (1.0 + exp(exponent));
    int potValue = constrain((int)calculatedValue, 0, 255);


    // ========== Constrain the potentiometer values
    if (potValue > 255) {
      potValue = 255;
    }

    if (potValue < 5) {
      potValue = 5;
    }

    MyPot.setWiper(potValue);  // set the new potentiometer value


    Serial.print("Pot value =");
    Serial.println(potValue);
    Serial.println("*********");

    lcd.setCursor(0, 1);
    lcd.print("P = ");
    lcd.setCursor(4, 1);
    lcd.print("   ");
    lcd.setCursor(4, 1);
    lcd.print(potValue);

    measuremillis = millis();
  }
}
