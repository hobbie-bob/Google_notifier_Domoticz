/*
So i wanted to control my Domoticz (smart-home on rasberry pi) by voice using google nest. 
Domoticz can also send messages like "you've got mail" "rain/water sensor activated" 
It is send via http to a single or to both my google nest speakers. (i hope 3 or 4 nest speaker will work, but haven't tested it yet.)
You can also type a message in your browser and it will sound from the speakers (LOL)
In this example i'm using a couple of physical buttons just in case something stops working.
Arduino ide OTA update works fine in this setup. My esp32 board is hidden somewhere in the kitchen.
Feel free to use and modify etc. 

Most of my work is inspierd by:
https://randomnerdtutorials.com
techiesms youtube channel
DroneBot Workshop youtube channel
and of course the guy with the swiss accent (youtube)

*/

// for rainmaker aap
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <HTTPClient.h>
#include <wifi_provisioning/manager.h>


// For OTA
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


// For IR signals
#include <IRremoteESP8266.h>
#include <IRsend.h>


//to make the google speaker talk works with es32
#include <esp8266-google-home-notifier.h>
const char displayName1[] = "livingroom"; //Name of the Nest speaker, must be exact name as in the google homeApp
const char displayName2[] = "garage"; //must be exact name as in the google homeApp

GoogleHomeNotifier ghn1;
GoogleHomeNotifier ghn2;
//GoogleHomeNotifier ghn3; //future plans...



#include "OneButton.h"

//switch inputs
const uint16_t sw1 = 18;  // switch 1  (GPIO19) (toggle afzuigkap light (ir)
const uint16_t sw2 = 21;  // switch 2  (GPIO21) (toggle schermerlamp light (webhook Domoticz)
// Setup a new OneButton
OneButton button1(sw1, true);
OneButton button2(sw2, true);


WiFiServer server(80);


// Set Defalt Values for output relais
#define DEFAULT_RELAY_MODE true

// BLE Credentils
const char *service_name = "PROV_12345";
const char *pop = "1234567";

// Devices of devices connected to GPIO
static uint8_t gpio_reset = 0;
static uint8_t IRpin = 23;
static uint8_t relay1 = 19;
static uint8_t relay2 = 22;
static uint8_t schemerlamp;
static uint8_t ventilator;
static uint8_t afzuiging;
static uint8_t elektrischedeken;
static uint8_t kerstboom;
static uint8_t regenalarm;
static uint8_t koofjes;
static uint8_t afzuigkap;
static uint8_t afzuigkaplampjes;


IRsend irsend(IRpin);


bool relay_state1 = true;
bool relay_state2 = true;
bool schemerlamp_state = false;
bool ventilator_state = false;
bool afzuiging_state = false;
bool elektrischedeken_state = false;
bool kerstboom_state = false;
bool regenalarm_state = false;
bool koofjes_state = false;
bool afzuigkap_state = false;
bool afzuigkaplampjes_state = false;


bool wifi_connected = 0;

const char* host = "1xx.1xx.1xx.x"; //domoticz ip
const int   port = 8080;            //domoticz port (default)
String Username = "dXNlcm5hbWUK"; //domoticz 'username' encoded in this example using https://www.base64encode.org
String Password = "J2RvbW90aWN6Jw=="; //'domoticz' password endcoded 



//------------------------------------------- Declaring Devices -----------------------------------------------------//

//The framework provides some standard device types like Switch, lightbulb, fan, temperature sensor.
static Switch my_switch1 ("Koof1", &relay1);
static Switch my_switch2 ("Koof2", &relay2);
static Switch my_switch3 ("Schemerlamp", &schemerlamp);
static Fan    my_switch4 ("Ventilator", &ventilator); //if used "Fan" the icon in the app is a fan.
static Fan    my_switch5 ("Afzuiging", &afzuiging);
static Switch my_switch6 ("Elektrischedeken", &elektrischedeken);
static Switch my_switch7 ("Kerstboom", &kerstboom);
static Switch my_switch8 ("Regenalarm", &regenalarm);
static Switch my_switch9 ("Koofjes", &koofjes);
static Switch my_switch10("Afzuigkap", &afzuigkap);
static Switch my_switch11("Afzuigkaplampjes", &afzuigkaplampjes);





void sysProvEvent(arduino_event_t *sys_event) {

  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      wifi_connected = 1;
      delay(500);
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
      }
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}




void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {

  Serial.println("callback triggerd");

  const char *device_name = device->getDeviceName();
  Serial.println(device_name);
  const char *param_name = param->getParamName();



  if (strcmp(device_name, "Koof1") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay_state1 = val.val.b;
      (relay_state1 == false) ? Leddriver1() : Leddriver1();
      param->updateAndReport(val);
    }
  }



  if (strcmp(device_name, "Koof2") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay_state2 = val.val.b;
      (relay_state2 == false) ? Leddriver2() : Leddriver2();
      param->updateAndReport(val);
    }
  }




  if (strcmp(device_name, "Schemerlamp") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      schemerlamp_state = val.val.b;
      (schemerlamp_state == false) ? sendHTTP("16", "Off") : sendHTTP("16", "On");
      param->updateAndReport(val);
    }
  }


  if (strcmp(device_name, "Ventilator") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      ventilator_state = val.val.b;
      (ventilator_state == false) ? sendHTTP("58", "On") : sendHTTP("58", "On"); // this is a motion detector. it can only be "triggerd"
      param->updateAndReport(val);
    }
  }


  if (strcmp(device_name, "Afzuiging") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      afzuiging_state = val.val.b;
      (afzuiging_state == false) ? sendHTTP("15", "Off") : sendHTTP("15", "On");
      param->updateAndReport(val);
    }
  }


  if (strcmp(device_name, "Elektrischedeken") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      elektrischedeken_state = val.val.b;
      (elektrischedeken_state == false) ? sendHTTP("39", "Off") : sendHTTP("39", "On");
      param->updateAndReport(val);
    }
  }


  if (strcmp(device_name, "Kerstboom") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      kerstboom_state = val.val.b;
      (kerstboom_state == false) ? sendHTTP("287", "Off") : sendHTTP("287", "On");
      param->updateAndReport(val);
    }
  }

  if (strcmp(device_name, "Regenalarm") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      regenalarm_state = val.val.b;
      (regenalarm_state == false) ? sendHTTP("36", "Off") : sendHTTP("36", "On");
      param->updateAndReport(val);
    }
  }



  if (strcmp(device_name, "Koofjes") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      koofjes_state = val.val.b;
      (koofjes_state == false) ? kOOfjes() : kOOfjes();
      param->updateAndReport(val);
    }
  }



  if (strcmp(device_name, "Afzuigkap") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      afzuigkap_state = val.val.b;
      (afzuigkap_state == false) ? toggleAfzuigkap() : toggleAfzuigkap();
      param->updateAndReport(val);
    }
  }



  if (strcmp(device_name, "Afzuigkaplampjes") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      afzuigkaplampjes_state = val.val.b;
      (afzuigkaplampjes_state == false) ? toggleAfzuigkaplampjes() : toggleAfzuigkaplampjes();
      param->updateAndReport(val);
    }
  }
}


void setup() {

  Serial.begin(115200);

  // Configure the input GPIOs
  pinMode(gpio_reset, INPUT_PULLUP); //PULLUP to avoid floating contact (it was triggerd by touch)
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);


  digitalWrite(relay1, DEFAULT_RELAY_MODE);
  digitalWrite(relay2, DEFAULT_RELAY_MODE);

  button1.attachClick(toggleAfzuigkaplampjes);
  button2.attachClick(toggleSchemerlamp);

  button1.attachDuringLongPress(sayA);
  button2.attachDuringLongPress(sayB);





  //------------------------------------------- Declaring Node -----------------------------------------------------//
  Node my_node;
  my_node = RMaker.initNode("Kitchen_node");

  //Standard switch device
  my_switch1.addCb(write_callback);
  my_switch2.addCb(write_callback);
  my_switch3.addCb(write_callback);
  my_switch4.addCb(write_callback);
  my_switch5.addCb(write_callback);
  my_switch6.addCb(write_callback);
  my_switch7.addCb(write_callback);
  my_switch8.addCb(write_callback);
  my_switch9.addCb(write_callback);
  my_switch10.addCb(write_callback);
  my_switch11.addCb(write_callback);

  //------------------------------------------- Adding Devices in Node -----------------------------------------------------//
  my_node.addDevice(my_switch1);
  my_node.addDevice(my_switch2);
  my_node.addDevice(my_switch3);
  my_node.addDevice(my_switch4);
  my_node.addDevice(my_switch5);
  my_node.addDevice(my_switch6);
  my_node.addDevice(my_switch7);
  my_node.addDevice(my_switch8);
  my_node.addDevice(my_switch9);
  my_node.addDevice(my_switch10);
  my_node.addDevice(my_switch11);





  // This is optional... i use the arduino OTA.
  // RMaker.enableOTA(OTA_USING_PARAMS);
  // If you want to enable scheduling, set time zone for your region using setTimeZone().
  // The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  RMaker.setTimeZone("Europe/Amsterdam");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  // RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");

  RMaker.start();


  WiFi.onEvent(sysProvEvent);

#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

  irsend.begin();

  SetupOTA();

  //------------end--OTA----------------------//
  for (int8_t q = 4 ; q > 0; q--) {
    Serial.println (q);
    delay(500);
  }

  Serial.println();
  Serial.print(" IP Address node: ");
  Serial.println(WiFi.localIP());


connectGoogleSpeakers();

}







void loop() {

  ArduinoOTA.handle();


  //-----------------------------------  Logic to Reset RainMaker

  // Read GPIO0 (external button to reset device
  if (digitalRead(gpio_reset) == LOW) { //Push button pressed
    Serial.printf("Reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    uint64_t startTime = millis();

    while (digitalRead(gpio_reset) == LOW) delay(50);
    uint64_t endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      wifi_connected = 0;
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 5000) {
      Serial.printf("Reset Wi-Fi.\n");
      wifi_connected = 0;
      // If key pressed for more than 5secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    } 
  }



Scan_for_incomming_messages();


  //check for pushed buttons
  button1.tick();
  button2.tick();
}





void toggleSchemerlamp() {
  sendHTTP("16", "Toggle");
}


void Leddriver1() { // used to pulse al leddriver with an optocoupler
  digitalWrite(relay1, LOW);
  delay(250);
  digitalWrite(relay1, HIGH);
}


void Leddriver2() {
  digitalWrite(relay2, LOW);
  delay(250);
  digitalWrite(relay2, HIGH);
}


void kOOfjes() {
  Leddriver1();
  Leddriver2();
}


//builds a url to command to control switches in Domoticz
//looks something like this: 192.168.100.100:8080"/json.htm?username=endcodedUsernam==&password=encodedPassword&type=command&param=switchlight&idx=16&switchcmd=Toggle"
void sendHTTP(String idx, String Command) { //idx divice Domoticz,  "On/Off/Toggle"
  String url;

  url = "/json.htm?username=" + Username + "==&password=" + Password + "&type=command&param=switchlight&idx=" + idx + "&switchcmd=" + Command;

  Serial.print("url = ");
  Serial.println(url);

  WiFiClient client;
  HTTPClient http;
  Serial.println("sendHTTP");
  http.begin(client, host, port, url); //build the url and send the actual code

  //prints response from webpage
  int httpCode = http.GET();
  if (httpCode > 0   ) {
    String payload = http.getString();
    Serial.println("Domoticz response ");
    Serial.println(payload);
  }

  http.end();
}
