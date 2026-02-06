// ESP 32  
//const char *ssid = "WARMACHINE";
//const char *password = "warmachine1234";
//mac : D4:8A:FC:CE:E6:99

#include <WiFi.h>
#include <esp_now.h>


String myData = "";
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x08, 0xD1, 0xF9, 0x6B, 0x21, 0xF1};

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



// Access point credentials
const char *apSSID = "WARMACHINE";
const char *apPassword = "warmachine1234";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Set up ESP32 as an access point
  setupAccessPoint();

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Print MAC address
  printMACAddress();

  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register callback for received data
  esp_now_register_recv_cb(onDataReceived);
}

void loop() {
  
  // This example doesn't use the loop function
}

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
  // Callback function to handle received data
  Serial.print("Data received: ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();

  // Blink the LED if the message contains "1001"
  if (strncmp((const char*)data, "1001", len) == 0) {
    myData = "1002";
  // Convert String to C-style string
  char dataBuffer[myData.length() + 1];
  myData.toCharArray(dataBuffer, myData.length() + 1);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) dataBuffer, myData.length() + 1);
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
  }
}

void setupAccessPoint() {
  Serial.println("Setting up ESP32 as access point...");
  WiFi.softAP(apSSID, apPassword);
  Serial.println("Access point SSID: " + String(apSSID));
  Serial.println("Access point password: " + String(apPassword));
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void printMACAddress() {
  Serial.print("MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
}
