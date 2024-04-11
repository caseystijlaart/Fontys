/*
 * Students   :   Giel Willemsen, Casey Stijlaart
 * Course     :   COM3 - HTTP
 * Date       :   10-09-2021
 * 
 * This program allows the user to receive and send data to
 * the ESP server with the help of GET and POST requests. 
 * 
 * The ESP server receives GET and POST requests trough the 
 * http / url. The available information the htpp / url can
 * request to GET is the LED status and/or the HALL sensor 
 * value. Lastly, with the POST request the http / url ís 
 * capable of changing the LED status, as in turning it on 
 * and off. 
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "";
const char *password = "";
bool led_state = false;
const int LED0 = 2;
WebServer server(80);

void setBuiltInLed() {
  // Handles the POST request for the LED.
  String message = server.arg("state");
  Serial.println(message);

  if (message == "true")
  {
    digitalWrite(LED0, HIGH);
    led_state = true;
  } else if (message == "false")
  {
    digitalWrite(LED0, LOW);
    led_state = false;
  }
  server.send(200);
}

void getHall() {
  // Handles the GET request for the HALL sensor.
  String hall = "{\"hall\": ";
  hall += String(hallRead());
  hall += "}\n";
  server.send(200, "text/json", hall);
}

void getLED() {
  // Handles the GET request for the LED.
  String led = "{\"led\": ";
  led += led_state ? "true" : "false";
  led += "}\n";
  server.send(200, "text/json", led);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void restServerRouting() {
  // Sets up the possible HTTP / url for the POST
  // and GET requests.
  server.on(F("/hall"), HTTP_GET, getHall);
  server.on(F("/led"), HTTP_GET, getLED);
  server.on(F("/led"), HTTP_POST, setBuiltInLed);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED0, OUTPUT);

  Serial.println("Start Wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  restServerRouting();
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  static unsigned long start = millis();
  if (millis() - start >= 500)
  {
    int val = hallRead();
    Serial.println(val, DEC);
    start = millis();
  }
}
