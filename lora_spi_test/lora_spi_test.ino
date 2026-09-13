/*
  Tes SPI mandiri ke modul LoRa SX1276 (MRV-RFM95W)
  Tidak memakai LMIC sama sekali. Tujuannya cuma satu:
  pastikan ESP32 bisa baca register RegVersion dari chip.
  Kalau hasilnya 0x12, wiring SPI dan RST sudah benar.
  Kalau bukan 0x12 (terutama 0x00 atau 0xFF), ada masalah wiring.

  Pin:
    NSS  = GPIO15 (OK)
    RST  = GPIO32 (ok)
    SCK  = GPIO18 (Tidak Menyala)
    MISO = GPIO19 (Tidak Menyala)
    MOSI = GPIO23 (OK)
*/

#include <SPI.h>

#define NSS_PIN  15
#define RST_PIN  32
#define SCK_PIN  18
#define MISO_PIN 19
#define MOSI_PIN 23

uint8_t readRegister(uint8_t addr) {
  digitalWrite(NSS_PIN, LOW);
  SPI.transfer(addr & 0x7F);
  uint8_t val = SPI.transfer(0x00);
  digitalWrite(NSS_PIN, HIGH);
  return val;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Tes SPI ke modul LoRa SX1276 dimulai"));

  pinMode(NSS_PIN, OUTPUT);
  digitalWrite(NSS_PIN, HIGH);

  // Hardware reset chip lewat pin RST
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(10);
  digitalWrite(RST_PIN, HIGH);
  delay(10);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);
  SPI.setFrequency(1000000);
  SPI.setDataMode(SPI_MODE0);

  delay(100);

  uint8_t version = readRegister(0x42); // RegVersion

  Serial.print(F("RegVersion = 0x"));
  Serial.println(version, HEX);

  if (version == 0x12) {
    Serial.println(F("SX1276 terdeteksi. SPI dan wiring OK."));
  } else if (version == 0x00 || version == 0xFF) {
    Serial.println(F("SX1276 TIDAK terdeteksi."));
    Serial.println(F("Cek: VCC 3.3V ke modul, GND sama dengan ESP32,"));
    Serial.println(F("kabel MISO/MOSI/SCK/NSS tidak tertukar, RST tersambung."));
  } else {
    Serial.println(F("Nilai tidak terduga, kemungkinan wiring sebagian benar."));
    Serial.println(F("Cek ulang sambungan SPI satu per satu."));
  }
}

void loop() {
  delay(2000);
  uint8_t version = readRegister(0x42);
  Serial.print(F("RegVersion sekarang = 0x"));
  Serial.println(version, HEX);
}
