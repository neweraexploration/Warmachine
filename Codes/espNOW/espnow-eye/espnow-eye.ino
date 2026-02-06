// ESP 32 Cam 
//const char *ssid = "WARMACHINE";
//const char *password = "warmachine1234";
//mac : FC:B4:67:C3:BA:3C

#include <esp_now.h>
#include <WiFi.h>

String myData = "";
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0xCE, 0xE6, 0x99};

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
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
  esp_now_register_recv_cb(onDataReceived);

}

 
void loop() {
  
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
    Serial.println("Sending Respond");
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
