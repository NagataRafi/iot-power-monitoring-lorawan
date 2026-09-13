#include <SPI.h>

#define NSS  15
#define RST  32
#define SCK  18
#define MISO 19
#define MOSI 23

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  pinMode(NSS, OUTPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(NSS, HIGH);
  
  // Reset lebih panjang
  digitalWrite(RST, LOW);
  delay(500);
  digitalWrite(RST, HIGH);
  delay(500);
  
  SPI.begin(SCK, MISO, MOSI, NSS);
  SPI.setFrequency(1000000); // Turunkan kecepatan SPI ke 1MHz
  
  // Baca register version beberapa kali
  for(int i = 0; i < 3; i++) {
    digitalWrite(NSS, LOW);
    delayMicroseconds(100);
    SPI.transfer(0x42); // Alamat register version
    byte version = SPI.transfer(0x00);
    digitalWrite(NSS, HIGH);
    
    Serial.print("Percobaan "); Serial.print(i+1);
    Serial.print(": 0x"); Serial.println(version, HEX);
    delay(100);
  }
}

void loop() {}