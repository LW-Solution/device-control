// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

#include "Grove_Temperature_And_Humidity_Sensor.h"

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11

#define LUMINOSIDADEPIN 4
/*Notice: The DHT10 and DHT20 is different from other DHT* sensor ,it uses i2c interface rather than one wire*/
/*So it doesn't require a pin.*/
#define DHTPIN 14     // what pin we're connected to（DHT10 and DHT20 don't need define it）
DHT dht(DHTPIN, DHTTYPE);   //   DHT11 DHT21 DHT22
//DHT dht(DHTTYPE);         //   DHT10 DHT20 don't need to define Pin


void setup() {

    Serial.begin(115200);
    Serial.println("DHTxx test!");
    Wire.begin();
    pinMode(LUMINOSIDADEPIN, INPUT);
    /*if using WIO link,must pull up the power pin.*/
    // pinMode(PIN_GROVE_POWER, OUTPUT);
    // digitalWrite(PIN_GROVE_POWER, 1);

    dht.begin();
}

void loop() {

  analogReadResolution(10);

  float volts = analogRead(LUMINOSIDADEPIN) * 5 / 1024.0;

  float amps = volts / 10000.0;
  float microamps = amps * 1000000;
  float lux = microamps * 2.0;

  Serial.print("LUX ");
  Serial.print(lux);
  Serial.println("lx");

    float temp_hum_val[2] = {0};
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)


    if (!dht.readTempAndHumidity(temp_hum_val)) {
        Serial.print("Humidity: ");
        Serial.print(temp_hum_val[0]);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(temp_hum_val[1]);
        Serial.println(" *C");
    } 

    delay(1500);
}
