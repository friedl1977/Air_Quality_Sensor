
//#include "Adafruit_SPIDevice.h"
#include "../lib/SGP30/src/Adafruit_SPIDevice.h"
#include "../lib/SGP30/src/Adafruit_I2CDevice.h"
#include "../lib/SGP30/src/Adafruit_BusIO_Register.h"
#include "../lib/SGP30/src/Adafruit_SGP30.h"

// Include ST7789 TFT Display libraries //
#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX.h"
#include "../lib/Adafruit_ST7735_RK/src/Adafruit_ST7789.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansBold12pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansOblique12pt7b.h"
#include <SPI.h>

// ST7789 TFT  definitions // 
#define TFT_CS        S3
#define TFT_RST       D6        
#define TFT_DC        D5


int tvoc_state[4] = {0,0,0,0};
int co2_state[4] = {0,0,0,0};

unsigned long previousMillis = 0;        
const long interval = 10000;   

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);     // Hardware SPI

float p = 3.1415926;


Adafruit_SGP30 sgp;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {

   // TFT Setup //
  Serial.begin(115200);
  
  tft.init(240, 320);                                                                  // Init ST7789 320x240 
  tft.fillScreen(ST77XX_BLACK);

  while (!Serial) { delay(10); } // Wait for serial console to open!

  //Particle.publish("SGP30 test", PRIVATE);                                           // DEBUG

  if (! sgp.begin()){
    //Particle.publish("Sensor not found", PRIVATE);                                   // DEBUG -- initialising SGP30 sensor
    while (1);
  }
  
  //Particle.publish("Found SGP30 serial #", PRIVATE);                                 // DEBUG                   
  //Serial.print(sgp.serialnumber[0], HEX);
  //Serial.print(sgp.serialnumber[1], HEX);
  //Serial.println(sgp.serialnumber[2], HEX);

  delay(50);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
  
}

int counter = 0;

void measure() {

// If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
//float temperature = 22.1; // [°C]
//float humidity = 45.2; // [%RH]
//sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  
  //Particle.publish("eCO2 " + String(sgp.eCO2) +" ppm", PRIVATE);                     // DEBUG

  if ((sgp.TVOC >= 0) && (sgp.TVOC <= 220) && (tvoc_state[0] == 0)) {
    Serial.print("G_TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");           // DEBUG
    tft.fillRect(0,165,240,95,ST77XX_GREEN);
  
    tvoc_state[0] = 1;
    tvoc_state[1] = 0;
    tvoc_state[2] = 0;
    tvoc_state[3] = 0;

    print_values();
  
  } else if ((sgp.TVOC >= 221) && (sgp.TVOC <= 660) && (tvoc_state[1] == 0)) {
          Serial.print("G_TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");     // DEBUG
          tft.fillRect(0,165,240,95,ST77XX_YELLOW);
          tvoc_state[0] = 0;
          tvoc_state[1] = 1;
          tvoc_state[2] = 0;
          tvoc_state[3] = 0;
    
  } else if ((sgp.TVOC >= 661) && (sgp.TVOC <= 1430) && (tvoc_state[2] == 0)) {
          Serial.print("G_TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");    // DEBUG
          tft.fillRect(0,165,240,95,ST77XX_ORANGE);
          tvoc_state[0] = 0;
          tvoc_state[1] = 0;
          tvoc_state[2] = 1;
          tvoc_state[3] = 0;

  } else if ((sgp.TVOC >= 1431) && (tvoc_state[3] == 0)) {
          Serial.print("G_TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");     // DEBUG
          tft.fillRect(0,165,240,95,ST77XX_RED);
          tvoc_state[0] = 0;
          tvoc_state[1] = 0;
          tvoc_state[2] = 0;
          tvoc_state[3] = 1;
  } 



if ((sgp.eCO2 >= 0) && (sgp.eCO2 <= 1000) && (co2_state[0] == 0)) {
  Serial.print("G_CO2 "); Serial.print(sgp.eCO2); Serial.print(" ppm");                // DEBUG
  tft.fillRect(0,0,240,95,ST77XX_GREEN);
  
  co2_state[0] = 1;
  co2_state[1] = 0;
  co2_state[2] = 0;
  co2_state[3] = 0;
  
  } else if ((sgp.eCO2 >= 1001) && (sgp.eCO2 <= 2000) && (co2_state[1] == 0)) {
          Serial.print("G_CO2 "); Serial.print(sgp.eCO2); Serial.print(" ppm");        // DEBUG
          tft.fillRect(0,0,240,95,ST77XX_YELLOW);
          co2_state[0] = 0;
          co2_state[1] = 1;
          co2_state[2] = 0;
          co2_state[3] = 0;

  } else if ((sgp.eCO2 >= 2001) && (sgp.eCO2 <= 5000) && (co2_state[2] == 0)) {
          Serial.print("G_CO2 "); Serial.print(sgp.eCO2); Serial.print(" ppm");        // DEBUG
          tft.fillRect(0,0,240,95,ST77XX_ORANGE);
          co2_state[0] = 0;
          co2_state[1] = 0;
          co2_state[2] = 1;
          co2_state[3] = 0;

  } else if ((sgp.eCO2 >= 5000) && (co2_state[3] == 0)) {
          Serial.print("G_CO2 "); Serial.print(sgp.eCO2); Serial.print(" ppm");        // DEBUG
          tft.fillRect(0,0,240,95,ST77XX_RED);
          co2_state[0] = 0;
          co2_state[1] = 0;
          co2_state[2] = 0;
          co2_state[3] = 1;

  } 

  //Particle.publish("Raw H2 " + String(sgp.rawH2) + " \t", PRIVATE);                  // additional paramenters
  //Particle.publish("Raw Ethanol "+ String (sgp.rawEthanol) + "", PRIVATE);           // additional paramenters

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Particle.publish("Failed to get baseline readings", PRIVATE);
      return;
    }
  
  //Particle.publish("****Baseline values: eCO2: 0x" + String(eCO2_base, HEX), PRIVATE);
  //Particle.publish(" & TVOC: 0x" + String(TVOC_base, HEX), PRIVATE);
  }

  draw_screen();
}

void draw_screen() {
  
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(false);
  tft.setTextSize(3);
  
  tft.setCursor(45, 70);
  tft.print("CO2");
  
  tft.setCursor(15, 235);
  tft.print("TVOC");
  
  delay(50);

}

void print_values() {

 // if ((sgp.eCO2 <=9) || (sgp.TVOC <=9)) {
    tft.fillRect(0,95,120,60,ST77XX_WHITE);
    tft.fillRect(121,95,120,60,ST77XX_BLUE);
    
    tft.fillRect(0,260,120,60,ST77XX_WHITE);
    tft.fillRect(121,260,120,60,ST77XX_BLUE);
   
    // } else if ((sgp.eCO2 <=99) || (sgp.TVOC <=99)) {
    //         tft.fillRect(0,125,135,30,ST77XX_WHITE);
    //         tft.fillRect(0,290,135,30,ST77XX_WHITE);
  
    // } else if ((sgp.eCO2 <=999) || (sgp.TVOC <=999)) { 
    //         tft.fillRect(0,125,135,30,ST77XX_WHITE);
    //         tft.fillRect(0,290,135,30,ST77XX_WHITE);
    // }

  tft.setTextWrap(false);
  tft.setCursor(25, 120);
  tft.setTextColor(ST77XX_BLUE);
  tft.setFont(&FreeSansOblique12pt7b);
  tft.setTextSize(1);
  tft.println(sgp.eCO2);
  tft.setCursor(25, 145);
  tft.println("ppm");
  //Serial.print("G_eCO2 "); Serial.print(sgp.eCO2); Serial.print(" ppm");
  
  tft.setCursor(25, 285);
  tft.println(sgp.TVOC); 
  tft.setCursor(25, 310);
  tft.println("ppb\\t");
  //Serial.print("G_TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");

  tft.setCursor(150, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(sgp.rawH2);
  tft.setCursor(150, 145); 
  tft.println("H2 \\t");

  tft.setCursor(150, 285);
  tft.println(sgp.rawEthanol); 
  tft.setCursor(145, 310);
  tft.print("Ethanol"); 

  delay(50);

}

void loop() {

  measure();

  unsigned long currentMillis = millis(); 
  if (currentMillis - previousMillis >= interval) {
    print_values();
    previousMillis = currentMillis;
  }
}
