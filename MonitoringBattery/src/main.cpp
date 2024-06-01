// Include required libraries
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_INA219.h>
#include <WiFiClientSecure.h>

Adafruit_INA219 ina219;

// WiFi credentials
const char *ssid = "MAKERINDO2";        // change SSID
const char *password = "makerindo2019"; // change password

// Gscript ID and required credentials
const int httpsPort = 443;
const char *host = "script.google.com";
String GAS_ID = "AKfycbyI7NGupDXLwHU1upIfox_356g1stPt-HaKxGzv6E752ZeSXJOtQg_DPuSYk-6bZ0qu";

WiFiClientSecure client;

void sendData(float load, float current, float power);

void setup()
{
  Serial.begin(9600);
  delay(1000);
  // Start the INA219 sensor
  if (!ina219.begin())
  {
    Serial.print("FAILED TO FIND INA219 MODULE");
    while (1)
    {
      delay(10);
    }
  }
  ina219.setCalibration_16V_400mA();

  // Setup WiFi connection
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.print(ssid);
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setInsecure();
}

void loop()
{
  // float shuntVoltage = 0;
  // float busVoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  // shuntVoltage = ina219.getShuntVoltage_mV();
  // busVoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = ina219.getBusVoltage_V() + (ina219.getShuntVoltage_mV() / 1000);

  // String Bus = "Bus Voltage : " + String((int)(busVoltage * 1000)) + " mV ";
  //  String Shunt = "Shunt Voltage : " + String(shuntVoltage) + " mV ";
  String Load = "Load Voltage : " + String(loadvoltage * 1000) + " mV ";
  String Current = "Current : " + String(current_mA) + " mA ";
  String Power = "Power : " + String(power_mW) + " mW ";

  // Serial.print(Bus);
  // Serial.print(",");
  // Serial.print(Shunt);
  // Serial.print(",");
  Serial.print(Load);
  // Serial.print(",");
  Serial.print(Current);
  // Serial.print(",");
  Serial.print(Power);
  Serial.println(";");
  /*
    float load_mV = loadvoltage * 1000;
    Serial.print("01");
    Serial.print(",");
    Serial.print(loadvoltage);
    Serial.print(",");
    Serial.print(current_mA);
    Serial.print(",");
    Serial.print(power_mW);
    Serial.print(",");
    Serial.print("#");
  */

  sendData(loadvoltage, current_mA, power_mW);
  delay(10000);
}

void sendData(float load, float current, float power)
{
  Serial.println("==========");
  Serial.print("Connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }

  // String Bus = String((int)(bus * 1000));
  // String Shunt = String(shunt);
  String Load = String(load * 1000);
  String Current = String(current);
  String Power = String(power);

  // set string url untuk request
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?loadVoltage=" + Load + "&current=" + Current + "&power=" + Power;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: LogDataBaterai\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");

  // reply content dari server
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\""))
  {

  }
  else
  {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  client.stop();
  Serial.println("==========");
  Serial.println();
}