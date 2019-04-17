/* SFE_BMP180 schița exemplului bibliotecii
Această schiță arată cum se utilizează biblioteca SFE_BMP180 pentru a citi
Bosch BMP180 senzor barometric de presiune.
https://www.sparkfun.com/products/11824

*/

#include <SFE_BMP180.h>
#include <Wire.h>
#include <U8glib.h>
#include "DHT.h"

#define DHTPIN 2     // senzorul DHT11 conectat la pinul D2
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

U8GLIB_ST7920_128X64_1X u8g(7,6,5);  // En, RW, RS          // Creați un obiect u8g care să funcționeze cu afișajul, specificând numărul de ieșire CS pentru magistrala hardware SPI

// You will need to create an SFE_BMP180 object, here called "pressure":

SFE_BMP180 pressure;

#define ALTITUDE 320.0 // Altitudine medie Targu Mures

const uint8_t picaturi[] PROGMEM = {
  0x04,
  0x04,
  0x0E,
  0x0E,
  0x1F,
  0x1F,
  0x1F,
  0x0E         
};

void subrutina0(void) {
  // pictez un chenar
  u8g.drawFrame(0,0,127,63);
  // pun un font maricel
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 15, 15, "Statie meteo");
    
//  u8g.setFont(u8g_font_unifont);
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 20, 26, "cu senzor BMP180");
  u8g.setFont(u8g_font_5x7);
  u8g.drawStr( 6, 38, "ecran LCD12864 (ST7920)");
  u8g.drawStr( 10, 60, "realizat de Pekatonix");
  u8g.setFont(u8g_font_6x10); 
  u8g.drawStr( 25, 50, "versiunea 3.1");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("REBOOT");
  dht.begin();
  
  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  
    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  // parte de prezentare
  u8g.firstPage();  
  do {
    subrutina0(); // unde e mesajul de intampinare
  } while( u8g.nextPage() );
 delay(3000);
 
}

void loop()
{
  char status;
  double T,P,p0,a;
  T = T - 1;  // Corectie temperatura
  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  
  // Partea de masarura cu DHT - umiditate
  float h = dht.readHumidity();
  
  u8g.firstPage();
   do{   char s[2] = " ";

 // Afisez temperatura
   // u8g.drawFrame(0,0,127,14);   
    u8g.setFont(u8g_font_7x13);  // folosesc font 6x12
    u8g.drawStr(4, 10, "Temp   :");
    u8g.setFont(u8g_font_6x10);   // folosesc font mai mic pentru indice
    u8g.drawStr(31, 13, "ext"); 
    u8g.setFont(u8g_font_7x13);                     
    u8g.setPrintPos( 78, 10); u8g.print(T);
    u8g.setFont(u8g_font_6x12);
    u8g.drawStr(113, 7, "o");
    u8g.setFont(u8g_font_7x13);
    u8g.drawStr(119, 10, "C");
    
  // Afisez presiunea absoluta 
   // u8g.drawFrame(0,15,127,14); 
    u8g.setFont(u8g_font_7x13);  // folosesc font 6x12
    u8g.drawStr(4, 25, "Pres   :");
    u8g.setFont(u8g_font_6x10);   // folosesc font mai mic pentru indice
    u8g.drawStr(32, 27, "abs"); 
    u8g.setFont(u8g_font_7x13);  
    u8g.setPrintPos( 72, 25); u8g.print(P);
    u8g.drawStr(114, 25, "mB");
    
   // Afisez presiunea relativa (sealevel)
   // u8g.drawFrame(0,30,127,14);  
    u8g.setFont(u8g_font_7x13);  // folosesc font 6x12
    u8g.drawStr(4, 40, "Pres   :");
    u8g.setFont(u8g_font_6x10);   // folosesc font mai mic pentru indice
    u8g.drawStr(32, 42, "rel"); 
    u8g.setFont(u8g_font_7x13); 
    u8g.setPrintPos( 65, 40); u8g.print(p0);
    u8g.drawStr(114, 40, "mB");

    // Afisez umiditatea relativa
   // u8g.drawFrame(0,45,127,14);  
    u8g.setFont(u8g_font_7x13);  // folosesc font 6x12
    u8g.drawBitmapP( 0, 50, 1, 8, picaturi);
    u8g.drawBitmapP( 8, 55, 1, 8, picaturi);
    u8g.drawBitmapP( 8, 45, 1, 8, picaturi);
 //   u8g.drawStr(4, 63, "Umiditate :");
	  u8g.setPrintPos( 20, 60); u8g.print((int)h);
    u8g.setFont(u8g_font_6x12);
    u8g.drawStr(35, 60, "%");

    // Logo
    u8g.setFont(u8g_font_7x14);  // folosesc font 6x12
    u8g.drawStr(66, 63, "M   GP");
    u8g.setFont(u8g_font_5x8);   // folosesc font mai mic pentru indice
    u8g.drawStr(73, 63, "eteo"); 
    u8g.setFont(u8g_font_5x8);
    u8g.drawStr(108, 63, "2019"); 
    
    }    while(u8g.nextPage());
  
  delay(5000);  // Pause for 5 seconds.
}
