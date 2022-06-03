#include <Arduino.h>
#include <SparkFunLSM6DS3.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Adafruit_AHTX0.h>
#include <TFT_eSPI.h>
#define BUZZER 15

char ssid[] = "WIFI NAME";    // your network SSID (name) 
char pass[] = "WIFI PASSWORD"; // your network password (use for WPA, or use as key for WEP)

const IPAddress kIP = IPAddress(255, 255, 255, 255); // your AWS public IP address

const uint16_t kPort = 5000;
char kPath[50] = "/?time=0&temp=20&humid=50&step=0"; // default server input val

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

LSM6DS3 myIMU;
Adafruit_AHTX0 aht;
int count = 0;
int t = 0;
float stepVal = 0.50;
float fall = 1;
TFT_eSPI TFT = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  // temp humidity sensor set up
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  
  // buzzer set up
  ledcSetup(0, 1E5, 12);
  ledcAttachPin(BUZZER, 0);

  // screan set up
  TFT.init();
  TFT.setRotation(3);
  TFT.fillScreen(TFT_BLACK);
  TFT.setTextFont(6);

  // gyroscope set up
  myIMU.begin();

  // connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
}
 
void loop() {
  // check temp and humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  // count step
  float x = myIMU.readFloatAccelX();
  float z = myIMU.readFloatAccelZ();

  if (x > stepVal){
      count++;
  }
  
  // buzzer check
  Serial.println(z);
  if (abs(z - 1) > fall){
    ledcWriteTone(0, 500);
  }else{
    ledcWriteTone(0, 0);
  }
  
  Serial.print("step: ");
  Serial.println(count);

  // update screan display
  TFT.drawNumber(count, 0, 0);
  TFT.drawNumber(temp.temperature, 0, 40);
  TFT.drawNumber(humidity.relative_humidity, 0, 80);

  delay(250);

  // server data upload
  String path = "/?time=" + String(t) + "&temp=" + String(temp.temperature) + "&humid=" + String(humidity.relative_humidity) + "&step=" + String(count);
  path.toCharArray(kPath, 50); // create path

  int err =0;
  
  WiFiClient c;
  HttpClient http(c);
  
  err = http.get(kIP, NULL, kPort, kPath);
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop(); // And just stop, now that we've tried a download

  delay(250);
  t++; // increase the timer by 1
}