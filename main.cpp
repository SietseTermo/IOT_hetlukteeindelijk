// #include <Arduino.h>
// #include <WiFi.h>
// #include "esp_wpa2.h"
// #include "esp_wifi.h"

// #include "config.h"
// #define Serial USBSerial

// void connectWiFi() {
//     Serial.print("Attemping WiFi connection");
//     WiFi.disconnect(true);
//     WiFi.mode(WIFI_STA);

//     // the line below is for connecting to an 'open' network
//     // WiFi.begin(SSID, PASS);

//     // the lines below are for connecting to a WPA2 enterprise network 
//     // (taken from the oficial wpa2_enterprise example from esp-idf)
//     ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
//     ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID)) );
//     ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_UNAME, strlen(EAP_UNAME)) );
//     ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASS, strlen(EAP_PASS)) );
//     ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
//     WiFi.begin(SSID);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(200);
//         Serial.print(".");
//     }
//     Serial.println();

//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
// }

// #include <PubSubClient.h>

// WiFiClient espClient;
// PubSubClient client(espClient);

// void reconnectMQTTClient() {
//     while (!client.connected()) {
//         Serial.print("Attempting MQTT connection...");

//         if (client.connect(CLIENT_NAME.c_str())) {
//             Serial.println("connected");
//         }
//         else {
//             Serial.print("Retrying in 5 seconds - failed, rc=");
//             Serial.println(client.state());
            
//             delay(5000);
//         }
//     }
// }

// void createMQTTClient() {
//     client.setServer(BROKER.c_str(), 1883);
//     reconnectMQTTClient();
// }

// void setup()
// {
//     Serial.begin(115200);
//     Serial.print("Ik connect opnieuw");
//     delay(1000);
//     connectWiFi();
//     createMQTTClient();
// }


// void loop() {
//     reconnectMQTTClient();
//     client.loop();

//     delay(10000);
// }

/*
   Based on 31337Ghost's reference code from https://github.com/nkolban/esp32-snippets/issues/385#issuecomment-362535434
   which is based on pcbreflux's Arduino ESP32 port of Neil Kolban's example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
*/

/*
   Create a BLE server that will send periodic iBeacon frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data
   3. Start advertising.
   4. wait
   5. Stop advertising.
*/

//beacon code
/*
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEBeacon.h>

#define DEVICE_NAME            "ESP321"
#define SERVICE_UUID           "7A0247E7-8E88-409B-A959-AB5092DDB03E"
#define BEACON_UUID            "2D7A9F0C-E0E8-4CC9-A71B-A21DB2D034A2"
#define BEACON_UUID_REV        "A134D0B2-1DA2-1BA7-C94C-E8E00C9F7A2D"
#define CHARACTERISTIC_UUID    "82258BAA-DF72-47E8-99BC-B73D7ECD08A5"

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("deviceConnected = true");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("deviceConnected = false");

      // Restart advertising to be visible and connectable again
      BLEAdvertising* pAdvertising;
      pAdvertising = pServer->getAdvertising();
      pAdvertising->start();
      Serial.println("iBeacon advertising restarted");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");

      }
    }
};


void init_service() {
  BLEAdvertising* pAdvertising;
  pAdvertising = pServer->getAdvertising();
  pAdvertising->stop();

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pAdvertising->addServiceUUID(BLEUUID(SERVICE_UUID));

  // Start the service
  pService->start();

  pAdvertising->start();
}

void init_beacon() {
  BLEAdvertising* pAdvertising;
  pAdvertising = pServer->getAdvertising();
  pAdvertising->stop();
  // iBeacon
  BLEBeacon myBeacon;
  myBeacon.setManufacturerId(0x4c00);
  myBeacon.setMajor(5);
  myBeacon.setMinor(88);
  myBeacon.setSignalPower(0xc5);
  myBeacon.setProximityUUID(BLEUUID(BEACON_UUID_REV));

  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x1A);
  advertisementData.setManufacturerData(myBeacon.getData());
  pAdvertising->setAdvertisementData(advertisementData);

  pAdvertising->start();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");
  delay(2000);
  Serial.flush();
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  delay(2000);
  init_service();
  delay(2000);
  init_beacon();

  Serial.println("iBeacon + service defined and advertising!");
  
}

void loop() {
  if (deviceConnected) {
    Serial.printf("*** NOTIFY: %d ***\n", value);
    pCharacteristic->setValue(&value, 1);
    pCharacteristic->notify();
    value++;
    Serial.print(BEACON_UUID);
  }
  delay(2000);
}
*/
//einde beacon code

/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
   Changed to a beacon scanner to report iBeacon, EddystoneURL and EddystoneTLM beacons by beegee-tokyo
*/


// scanner code

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <WiFi.h>
#include "esp_wpa2.h"
#include "esp_wifi.h"

#include "config.h"
#include <ArduinoJson.h>

int RSSI = 0;
int scanTime = 5; //In seconds
BLEScan *pBLEScan;

StaticJsonDocument<96> doc;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      Serial.println("______________________________________________");
      if (advertisedDevice.haveName())
      {
        Serial.print("Device name: ");
        Serial.println(advertisedDevice.getName().c_str());
      }

      if (advertisedDevice.haveServiceUUID())
      {
        BLEUUID devUUID = advertisedDevice.getServiceUUID();
        Serial.print("ServiceUUID: ");
        Serial.println(devUUID.toString().c_str());
      }
      
      if (advertisedDevice.haveManufacturerData() == true)
      {
        std::string strManufacturerData = advertisedDevice.getManufacturerData();

        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

        if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
        {
          BLEBeacon oBeacon = BLEBeacon();
          oBeacon.setData(strManufacturerData);
          Serial.println("Found an iBeacon!");
          Serial.printf("ID: %04X, Major: %d, Minor: %d, RSSI: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), advertisedDevice.getRSSI());
          RSSI = advertisedDevice.getRSSI();

          doc["time"] = time(NULL);
          string id = oBeacon.getManufacturerId();
          float RSSI_value = advertisedDevice.getRSSI();

          doc["ID"] = id;
          doc["RSSI"] = RSSI_value;

        }
        else
        {
          Serial.println("Found another manufacturer's beacon!");
          Serial.printf("Data: %d ", strManufacturerData.length());
          for (int i = 0; i < strManufacturerData.length(); i++)
          {
            Serial.printf("%02X ", cManufacturerData[i]);
          }
          Serial.printf("\n");
        }
      }

      uint8_t *payLoad = advertisedDevice.getPayload();
      // search for Eddystone Service Data in the advertising payload
      // *payload shall point to eddystone data or to its end when not found
      const uint8_t serviceDataEddystone[3] = {0x16, 0xAA, 0xFE}; // it has Eddystone BLE UUID
      const size_t payLoadLen = advertisedDevice.getPayloadLength();
      uint8_t *payLoadEnd = payLoad + payLoadLen - 1; // address of the end of payLoad space
      while (payLoad < payLoadEnd) {
        if (payLoad[1] == serviceDataEddystone[0] && payLoad[2] == serviceDataEddystone[1] && payLoad[3] == serviceDataEddystone[2]) {
          // found!
          payLoad += 4;
          break;
        }
        payLoad += *payLoad + 1;  // payLoad[0] has the field Length
      }

      if (payLoad < payLoadEnd) // Eddystone Service Data and respective BLE UUID were found
      {
        if (*payLoad == 0x10)
        {
          Serial.println("Found an EddystoneURL beacon!");
          BLEEddystoneURL foundEddyURL = BLEEddystoneURL();
          uint8_t URLLen = *(payLoad - 4) - 3;  // Get Field Length less 3 bytes (type and UUID) 
          foundEddyURL.setData(std::string((char*)payLoad, URLLen));
          std::string bareURL = foundEddyURL.getURL();
          if (bareURL[0] == 0x00)
          {
            // dumps all bytes in advertising payload
            Serial.println("DATA-->");
            uint8_t *payLoad = advertisedDevice.getPayload();
            for (int idx = 0; idx < payLoadLen; idx++)
            {
              Serial.printf("0x%02X ", payLoad[idx]);
            }
            Serial.println("\nInvalid Data");
            return;
          }

          Serial.printf("Found URL: %s\n", foundEddyURL.getURL().c_str());
          Serial.printf("Decoded URL: %s\n", foundEddyURL.getDecodedURL().c_str());
          Serial.printf("TX power %d\n", foundEddyURL.getPower());
          Serial.println("\n");
        } 
        else if (*payLoad == 0x20)
        {
          Serial.println("Found an EddystoneTLM beacon!");
 
          BLEEddystoneTLM eddystoneTLM;
          eddystoneTLM.setData(std::string((char*)payLoad, 14));
          Serial.printf("Reported battery voltage: %dmV\n", eddystoneTLM.getVolt());
          Serial.printf("Reported temperature: %.2f°C (raw data=0x%04X)\n", eddystoneTLM.getTemp(), eddystoneTLM.getRawTemp());
          Serial.printf("Reported advertise count: %d\n", eddystoneTLM.getCount());
          Serial.printf("Reported time since last reboot: %ds\n", eddystoneTLM.getTime());
          Serial.println("\n");
          Serial.print(eddystoneTLM.toString().c_str());
          Serial.println("\n");
        }
      }
    }
};

void connectWiFi() {
    Serial.print("Attemping WiFi connection");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    // the line below is for connecting to an 'open' network
    // WiFi.begin(SSID, PASS);

    // the lines below are for connecting to a WPA2 enterprise network 
    // (taken from the oficial wpa2_enterprise example from esp-idf)
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_UNAME, strlen(EAP_UNAME)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASS, strlen(EAP_PASS)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    WiFi.begin(SSID);

    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

void reconnectMQTTClient() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (client.connect(CLIENT_NAME.c_str())) {
            Serial.println("connected");
        }
        else {
            Serial.print("Retrying in 5 seconds - failed, rc=");
            Serial.println(client.state());
            
            delay(5000);
        }
    }
}

void createMQTTClient() {
    client.setServer(BROKER.c_str(), 1883);
    reconnectMQTTClient();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
  connectWiFi();
  createMQTTClient();
}

void loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("\n* Devices found: ");
  Serial.println(foundDevices.getCount());
  // Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delay(10000);

  reconnectMQTTClient();
  client.loop();




  string JES20052006;
  serializeJson(doc, JES20052006);

  Serial.print("Sending telemetry ");
  Serial.println(JES20052006.c_str());

  client.publish(CLIENT_TELEMETRY_TOPIC.c_str(), JES20052006.c_str());

  delay(10000);
}

