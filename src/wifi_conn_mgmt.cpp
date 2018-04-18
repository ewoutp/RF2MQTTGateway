#include "wifi_conn_mgmt.h"
#include "mqtt.h"
#include "secrets.h"

static TimerHandle_t wifiReconnectTimer;
/*static String wifi_ssid;
static String wifi_pass;*/

void connectToWifi()
{
  //WiFi.disconnect();
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

static void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connectToMqtt();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    stopMqttReconnect();
    xTimerStart(wifiReconnectTimer, 0);
    break;
  }
}

// setupWifiManager creates a WifiManager and configures Wifi from
// eeprom, or creates a portal to allow a user to configure it.
void setupWifi()
{
  /*
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  // Fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP" and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("RF2MQTTGateway");
  wifi_ssid = wifiManager.getSSID();
  wifi_pass = wifiManager.getPassword();
  Serial.println("SSID:");
  Serial.println(wifi_ssid);
  Serial.println(wifi_pass);
  */
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);
}
