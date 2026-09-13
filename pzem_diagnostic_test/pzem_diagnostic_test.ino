/*
 * PZEM Diagnostic Test - 3 Sensor
 * ESP32 38-Pin DevKit V1
 *
 * PZEM1 -> UART1: RX=21, TX=22, addr 0x01
 * PZEM2 -> UART2: RX=14, TX=13, addr 0x03
 * PZEM3 -> UART0: RX=3,  TX=1,  addr 0x02
 *
 * PENTING: Lepas kabel data PZEM3 (RX0/TX0) sebelum upload sketch ini.
 * Pasang kembali setelah upload selesai untuk membaca PZEM3.
 *
 * Library: PZEM004Tv30 (pastikan versi >= 1.1.0)
 * Install via Library Manager: "PZEM004Tv30 by olehs"
 */

#include <PZEM004Tv30.h>

// ===== Definisi UART untuk masing-masing PZEM =====
HardwareSerial pzemSerial1(1);  // UART1 -> PZEM1
HardwareSerial pzemSerial2(2);  // UART2 -> PZEM2
HardwareSerial pzemSerial0(0);  // UART0 -> PZEM3 (shared dengan USB)

// ===== Pin assignment =====
#define PZEM1_RX 21
#define PZEM1_TX 22
#define PZEM1_ADDR 0x01

#define PZEM2_RX 14
#define PZEM2_TX 13
#define PZEM2_ADDR 0x03

#define PZEM3_RX 3
#define PZEM3_TX 1
#define PZEM3_ADDR 0x02

// ===== Objek PZEM =====
PZEM004Tv30 pzem1(pzemSerial1, PZEM1_RX, PZEM1_TX, PZEM1_ADDR);
PZEM004Tv30 pzem2(pzemSerial2, PZEM2_RX, PZEM2_TX, PZEM2_ADDR);
PZEM004Tv30 pzem3(pzemSerial0, PZEM3_RX, PZEM3_TX, PZEM3_ADDR);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("=== PZEM Diagnostic Test - 3 Sensor ===");
  Serial.println("Memastikan ketiga sensor terbaca dengan parameter lengkap.");
  Serial.println();
}

void loop() {
  readAndPrintPZEM("PZEM1 (Phase 1)", pzem1);
  delay(500);
  readAndPrintPZEM("PZEM2 (Phase 2)", pzem2);
  delay(500);
  readAndPrintPZEM("PZEM3 (Phase 3)", pzem3);
  delay(500);

  Serial.println("----------------------------------------");
  delay(2000);
}

void readAndPrintPZEM(const char* label, PZEM004Tv30 &pzem) {
  float voltage      = pzem.voltage();
  float current       = pzem.current();
  float power          = pzem.power();
  float energy         = pzem.energy();
  float frequency      = pzem.frequency();
  float pf             = pzem.pf();

  Serial.print("[");
  Serial.print(label);
  Serial.println("]");

  if (isnan(voltage)) {
    Serial.println("  Error membaca data. Periksa wiring/alamat sensor.");
    return;
  }

  Serial.print("  Voltage   : "); Serial.print(voltage, 1);  Serial.println(" V");
  Serial.print("  Current   : "); Serial.print(current, 3);  Serial.println(" A");
  Serial.print("  Power     : "); Serial.print(power, 1);    Serial.println(" W");
  Serial.print("  Energy    : "); Serial.print(energy, 3);   Serial.println(" kWh");
  Serial.print("  Frequency : "); Serial.print(frequency, 1);Serial.println(" Hz");
  Serial.print("  PF        : "); Serial.print(pf, 2);       Serial.println("");
  Serial.println();
}
