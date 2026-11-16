#include <Arduino.h>
#include <cstring>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

namespace {

// button connected to GPIO10
constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_10;
constexpr char WIFI_SSID[] = "<wifi ssid>";
constexpr char WIFI_PASSWORD[] = "<wifi passphrase>";
constexpr char REQUEST_URL[] = "<paste webhooks link here>";

// Wire the button from GPIO10 to GND; INPUT_PULLUP provides the pull-up.
// For HTTPS, paste the server's trusted root CA certificate here.
constexpr char ROOT_CA[] = R"(-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----)";

void makeGetRequest() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  const unsigned long connectStarted = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - connectStarted < 20000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed; skipping GET request.");
    return;
  }

  HTTPClient http;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;
  bool started = false;

  if (strncmp(REQUEST_URL, "https://", 8) == 0) {
    if (ROOT_CA[0] == '\0') {
      Serial.println("HTTPS requires the server's root CA in ROOT_CA.");
      return;
    }
    secureClient.setCACert(ROOT_CA);
    started = http.begin(secureClient, REQUEST_URL);
  } else {
    started = http.begin(plainClient, REQUEST_URL);
  }

  if (!started) {
    Serial.println("Could not start HTTP client.");
    return;
  }

  const int status = http.GET();
  if (status > 0) {
    Serial.printf("GET returned HTTP status %d\n", status);
  } else {
    Serial.printf("GET failed: %s\n", http.errorToString(status).c_str());
  }
  http.end();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(static_cast<int>(BUTTON_PIN), INPUT_PULLUP);
  Serial.println("Running continuously; press the button to send the GET.");
}

void loop() {
  if (digitalRead(static_cast<int>(BUTTON_PIN)) != LOW) {
    delay(10);
    return;
  }

  delay(40);
  if (digitalRead(static_cast<int>(BUTTON_PIN)) == LOW) {
    makeGetRequest();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  while (digitalRead(static_cast<int>(BUTTON_PIN)) == LOW) {
    delay(20);
  }
  delay(40);
}
