#ifndef ARDUINO_INKPLATE2
#error "Wrong board selection for this example, please select Soldered Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "secrets.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "time.h"

const char ssid[] = WIFI_SSID;      // Your WiFi SSID
const char pass[] = WIFI_PASSWORD;  // Your WiFi password

// API endpoints
const char* US_DEBT_API = "https://treasurydirect.gov/NP_WS/debt/current";
const int MAX_HTTP_RETRIES = 10;
const int RETRY_DELAY_MS = 5000;  // 5 seconds between retries

// Time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800;                // PST: UTC-8 * 3600
const int daylightOffset_sec = 3600;              // 1 hour for daylight savings
const int UPDATE_INTERVAL = 12 * 60 * 60 * 1000;  // 12 hours in milliseconds
unsigned long lastUpdateTime = 0;

Inkplate display;

// Helper function to wrap text
void printWrappedText(const String& text, int textSize) {
  int charWidth = 6 * textSize;  // Base character width is ~6 pixels
  int maxCharsPerLine = display.width() / charWidth;

  String words = text;
  String line = "";

  while (words.length() > 0) {
    int spaceIndex = words.indexOf(' ');
    String word;

    if (spaceIndex == -1) {
      word = words;
      words = "";
    } else {
      word = words.substring(0, spaceIndex);
      words = words.substring(spaceIndex + 1);
    }

    if ((line.length() + word.length() + 1) <= maxCharsPerLine) {
      if (line.length() > 0) {
        line += " ";
      }
      line += word;
    } else {
      display.println(line);
      line = word;
    }

    if (words.length() == 0) {
      display.println(line);
    }
  }
}

// Function to convert string debt to double
double parseDebtString(const JsonVariant& debt) {
  if (debt.is<const char*>()) {
    // Original string handling
    String debtStr = String(debt.as<const char*>());
    debtStr.replace("$", "");
    debtStr.replace(",", "");
    return debtStr.toDouble();
  } else {
    // Direct number handling
    return debt.as<double>();
  }
}

// Function to format large numbers with commas
String formatWithCommas(double value) {
  char buf[30];
  char formatted[40];
  int idx = 0;
  int commaCount = 0;

  sprintf(buf, "%.2f", value);
  String numStr = String(buf);
  int decimalPos = numStr.indexOf('.');
  if (decimalPos == -1) decimalPos = numStr.length();

  // Add digits with commas
  for (int i = decimalPos - 1; i >= 0; i--) {
    if (commaCount == 3 && i != 0) {
      formatted[idx++] = ',';
      commaCount = 0;
    }
    formatted[idx++] = numStr[i];
    commaCount++;
  }
  formatted[idx] = '\0';

  // Reverse the string
  for (int i = 0; i < idx / 2; i++) {
    char temp = formatted[i];
    formatted[i] = formatted[idx - 1 - i];
    formatted[idx - 1 - i] = temp;
  }

  // Add decimal portion if exists
  if (decimalPos < numStr.length()) {
    strcat(formatted, numStr.substring(decimalPos).c_str());
  }

  return String(formatted);
}

bool fetchAndDisplayDebt() {
  HTTPClient http;
  bool success = false;
  int retryCount = 0;

  while (!success && retryCount < MAX_HTTP_RETRIES) {
    if (retryCount > 0) {
      Serial.printf("Retry attempt %d of %d\n", retryCount, MAX_HTTP_RETRIES);
      display.setCursor(0, 0);
      display.printf("Retry attempt %d of %d\n", retryCount, MAX_HTTP_RETRIES);
      display.display();
      delay(RETRY_DELAY_MS);
    }

    if (http.begin(US_DEBT_API)) {
      int httpCode = http.GET();
      Serial.printf("HTTP Response code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // Get the debt value and handle it as a variant
          JsonVariant totalDebt = doc["totalDebt"];
          if (!totalDebt.isNull()) {
            double totalDebtValue = parseDebtString(totalDebt);
            Serial.print("Parsed debt value: ");
            Serial.println(totalDebtValue, 2); // TODO: fix overflow

            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextSize(2);
            display.setTextColor(INKPLATE2_BLACK);
            display.println("US National Debt:");
            display.println();
            display.setTextColor(INKPLATE2_RED);
            display.setTextSize(2);
            String debtStr = "$" + formatWithCommas(totalDebtValue);
            printWrappedText(debtStr, 1);
            display.println();
            display.setTextSize(1);
            display.println("Updated: ");
            if (!doc["effectiveDate"].isNull()) {
              display.println(doc["effectiveDate"].as<const char*>());
            }
            display.display();

            success = true;
          } else {
            Serial.println("totalDebt is null in response");
          }
        } else {
          Serial.print("JSON parsing failed! Error: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.printf("HTTP request failed with code: %d\n", httpCode);
      }

      http.end();
    } else {
      Serial.println("Failed to connect to API");
    }

    retryCount++;
  }

  if (!success) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Failed to fetch data after all retries!");
    display.display();
  }

  return success;
}

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    display.println("Failed to obtain time");
    display.display();
    return;
  }

  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  display.println(timeStringBuff);
  display.display();
}

void setupWiFi() {
  display.println("Scanning for WiFi networks...");  // Write text
  display.display();                                 // Send everything to display (refresh display)

  int n =
    WiFi.scanNetworks();  // Start searching WiFi networks and put the nubmer of found WiFi networks in variable n
  Serial.println("Scanning networks");
  display.setCursor(0, 0);  // Set print position
  if (n == 0) {
    // If you did not find any network, show the message and stop the program.
    display.print("No WiFi networks found!");
    display.display();
    while (true)
      ;
  } else {
    if (n > 10)
      n = 10;  // If you did find, print name (SSID), encryption and signal strength of first 10 networks
    for (int i = 0; i < n; i++) {
      display.setTextColor(INKPLATE2_BLACK,
                           INKPLATE2_WHITE);  // Set text color to black and background color to white
      display.print(WiFi.SSID(i));
      display.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 'O' : '*');
      display.print(' ');
      display.setTextColor(INKPLATE2_RED, INKPLATE2_WHITE);  // Set text color to red and background color to white
      display.println(WiFi.RSSI(i), DEC);
    }
    display.display();  // Refresh screen
  }
  display.clearDisplay();   // Clear everything in frame buffer
  display.setCursor(0, 0);  // Set print cursor to new position
  display.print("Connecting to ");
  display.print(ssid);
  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi network");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)  // Add timeout
  {
    wl_status_t wifi_status = WiFi.status();
    switch (wifi_status) {
      case WL_IDLE_STATUS:
        Serial.println("WiFi status: IDLE");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("WiFi status: SSID not available");
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("WiFi status: Scan completed");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("WiFi status: Connection failed");
        break;
      case WL_CONNECTION_LOST:
        Serial.println("WiFi status: Connection lost");
        break;
      case WL_DISCONNECTED:
        Serial.println("WiFi status: Disconnected");
        break;
      default:
        Serial.print("WiFi status code: ");
        Serial.println(wifi_status);
    }
    delay(1000);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect after timeout");
    display.println("WiFi connection failed!");
    display.display();
  }
}

void setup() {
  Serial.begin(115200);
  display.begin();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);

  setupWiFi();
  setupTime();
  if (!fetchAndDisplayDebt()) {
    Serial.println("Failed to fetch data after all retry attempts");
  }

  lastUpdateTime = millis();
}

void loop() {
   unsigned long currentTime = millis();
    
    // Check if it's time for update
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
        // Reconnect to WiFi if necessary
        if (WiFi.status() != WL_CONNECTED) {
            setupWiFi();
        }
        
        // Fetch new debt data
        if (fetchAndDisplayDebt()) {
            lastUpdateTime = currentTime;
        }
    }
    
    // Deep sleep to save power
    delay(60000); // Check every minute if it's time to update
}