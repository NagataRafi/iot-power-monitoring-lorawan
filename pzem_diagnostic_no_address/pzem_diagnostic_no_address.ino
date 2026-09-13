/*
 * PZEM Diagnostic Test - 3 Sensor (TANPA address eksplisit)
 * ESP32 38-Pin DevKit V1
 *
 * Setiap PZEM dibedakan lewat UART fisik, bukan lewat alamat Modbus.
 * Semua sensor boleh tetap di alamat default 0xF8.
 *
 * PZEM1 -> UART1: RX=21, TX=22
 * PZEM2 -> UART2: RX=14, TX=13
 * PZEM3 -> UART0: RX=3,  TX=1   (shared dengan USB, lepas saat upload)
 *
 * Library: PZEM004Tv30 (pastikan versi >= 1.1.0)
 */

#include <PZEM004Tv30.h>

HardwareSerial pzemSerial1(1);  // UART1 -> PZEM1
HardwareSerial pzemSerial2(2);  // UART2 -> PZEM2
HardwareSerial pzemSerial0(0);  // UART0 -> PZEM3

#define PZEM1_RX 21
#define PZEM1_TX 22

#define PZEM2_RX 14
#define PZEM2_TX 13

#define PZEM3_RX 3
#define PZEM3_TX 1

// Constructor tanpa parameter address -> pakai default 0xF8
PZEM004Tv30 pzem1(pzemSerial1, PZEM1_RX, PZEM1_TX);
PZEM004Tv30 pzem2(pzemSerial2, PZEM2_RX, PZEM2_TX);
PZEM004Tv30 pzem3(pzemSerial0, PZEM3_RX, PZEM3_TX);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("=== PZEM Diagnostic Test - Tanpa Address ===");
  Serial.println("Sensor dibedakan lewat jalur UART, bukan address.");
  Serial.println();
}

void loop() {
  readAndPrintPZEM("PZEM1 (UART1, Phase 1)", pzem1);
  delay(500);
  readAndPrintPZEM("PZEM2 (UART2, Phase 2)", pzem2);
  delay(500);
  readAndPrintPZEM("PZEM3 (UART0, Phase 3)", pzem3);
  delay(500);

  Serial.println("----------------------------------------");
  delay(2000);
}

void readAndPrintPZEM(const char* label, PZEM004Tv30 &pzem) {
  float voltage   = pzem.voltage();
  float current   = pzem.current();
  float power     = pzem.power();
  float energy    = pzem.energy();
  float frequency = pzem.frequency();
  float pf        = pzem.pf();

  Serial.print("[");
  Serial.print(label);
  Serial.println("]");

  if (isnan(voltage)) {
    Serial.println("  Tidak ada respons. Periksa wiring TX/RX dan power 5V sensor ini.");
    Serial.println();
    return;
  }

  Serial.print("  Voltage   : "); Serial.print(voltage, 1);   Serial.println(" V");
  Serial.print("  Current   : "); Serial.print(current, 3);   Serial.println(" A");
  Serial.print("  Power     : "); Serial.print(power, 1);     Serial.println(" W");
  Serial.print("  Energy    : "); Serial.print(energy, 3);    Serial.println(" kWh");
  Serial.print("  Frequency : "); Serial.print(frequency, 1); Serial.println(" Hz");
  Serial.print("  PF        : "); Serial.print(pf, 2);        Serial.println("");
  Serial.println();
}
