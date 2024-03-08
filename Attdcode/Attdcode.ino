#include <WiFi.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "Galaxy M42";
const char* password = "135792468";

// Initialize RFID reader
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// LED pins
#define GREEN_LED 4
#define RED_LED 5

// Structure to store card punch status
struct CardStatus {
  String uid;
  bool punchedIn;
};

// Array to store card punch status
CardStatus cardStatus[10]; // Assuming maximum 10 cards

String formattedDate;
String dayStamp;
String timeStamp;

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize SPI bus and RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Place your RFID tag near the reader...");
  Serial.println();

  // Initialize LED pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  timeClient.begin();
  timeClient.setTimeOffset(19800);

  // Initialize card status
  for (int i = 0; i < 10; i++) {
    cardStatus[i].uid = "";
    cardStatus[i].punchedIn = false;
  }
}

void loop() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();

    // Look for new RFID cards
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Get UID of card
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }

      // Check if card is registered
      int cardIndex = findCard(uid);

      if (cardIndex == -1) {
        // Card not registered, find an empty slot
        cardIndex = findEmptySlot();
        if (cardIndex != -1) {
          // Store UID and mark as punched in
          cardStatus[cardIndex].uid = uid;
          cardStatus[cardIndex].punchedIn = true;
          Serial.println("New card registered. UID: " + uid);
          Serial.println("Punched In");
        }
      } else {
        // Card already registered, toggle punch status
        cardStatus[cardIndex].punchedIn = !cardStatus[cardIndex].punchedIn;
        Serial.println("UID: " + uid);
        Serial.println(cardStatus[cardIndex].punchedIn ? "Punched In" : "Punched Out");
      }

      // Get current time
      formattedDate = timeClient.getFormattedTime();
      dayStamp = formattedDate.substring(0, 11); // Extract date stamp
      timeStamp = formattedDate.substring(11); // Extract time stamp
      
      // Print timestamp along with UID
      Serial.println("Timestamp: " + dayStamp + " " + timeStamp);
      
      // Blink LED
      blinkLED(GREEN_LED);
    }
    delay(3000);
  }
}

// Function to find card index in cardStatus array
int findCard(String uid) {
  for (int i = 0; i < 10; i++) {
    if (cardStatus[i].uid == uid) {
      return i;
    }
  }
  return -1; // Card not found
}

// Function to find an empty slot in cardStatus array
int findEmptySlot() {
  for (int i = 0; i < 10; i++) {
    if (cardStatus[i].uid == "") {
      return i;
    }
  }
  return -1; // No empty slot found
}

void blinkLED(int ledPin) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(300);
  }
}

