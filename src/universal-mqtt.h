// #include <ESP8266WiFi.h>
// #include "FS.h"
// #include <WiFiClientSecure.h>
#include <Wire.h>

#include <time.h>
#include <Arduino.h>

#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" // Wifi configuration here

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

#define SerialMon Serial
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 9600
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
///////////////////////////////
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";
SoftwareSerial SerialAT(10, 11); // RX, TX
TinyGsm modem(SerialAT);
TinyGsmClientSecure *gsmclient;

// Initialize WiFi and MQTT for this board
MQTTClient *mqttClient;
// BearSSL::WiFiClientSecure *netClient;

CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
unsigned long iat = 0;
String jwt;

String getJwt()
{
  // Disable software watchdog as these operations can take a while.
  // ESP.wdtDisable();
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  // ESP.wdtEnable(0);
  return jwt;
}

void setupCert()
{
  // Set CA cert on wifi client
  // If using a static (pem) cert, uncomment in ciotc_config.h:
  // certList.append(primary_ca);
  // certList.append(backup_ca);
  // netClient->setTrustAnchors(&certList);
  return;
}

// void setupWifi()
// {
//   Serial.println("Init WiFi");
//   WiFi.disconnect();
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   Serial.println("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.println("Connecting...");
//     Serial.println(WiFi.status());
//   }

//   configTime(0, 0, ntp_primary, ntp_secondary);
//   Serial.println("Waiting on time sync...");
//   while (time(nullptr) < 1510644967)
//   {
//     delay(10);
//   }
// }

// void connectWifi()
// {
//   Serial.print("checking wifi..."); // TODO: Necessary?
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(1000);
//   }
// }

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data)
{

  return mqtt->publishTelemetry(data);
}

bool publishCabinetTelemetry(String cabinet_id, String data)
{
  Serial.println(cabinet_id + "   " + data);

  bool s = mqtt->publishTelemetry(data);
  return s;
}

bool publishCabinetState(String state)
{

  return mqtt->publishState(state);
}

bool publishPortTelemetry(String cabinet_id, String port_id, String data)
{
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length)
{
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data)
{
  //mqtt->publishState("testado");
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char *data, int length)
{
  return mqtt->publishTelemetry(subfolder, data, length);
}

void connect()
{
  mqtt->mqttConnect();
}

// TODO: fix globals
void setupCloudIoT()
{
  // Create the device
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);

  // ESP8266 WiFi setup
  //gsmclient = gsmclient(modem);
  gsmclient = new TinyGsmClientSecure(modem);

  // setupWifi();

  // ESP8266 WiFi secure initialization
  // setupCert();

  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, gsmclient, device);
  mqtt->setLogConnect(false); // If mqtt registry state ever connected on cloudiot core
  mqtt->setUseLts(true);      // Long-term service for MQTT

  mqtt->startMQTT(); // Opens connection
}
