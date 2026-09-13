/*
  Power Monitoring 3 Fase - ESP32 + LoRaWAN OTAA + 3x PZEM-004T v3.0
  ---------------------------------------------------------------
  Pin LoRa (SX1276):
    NSS  = GPIO15
    RST  = GPIO32
    DIO0 = GPIO4
    DIO1 = GPIO25
    SPI  = SCK18, MISO19, MOSI23 (default VSPI ESP32)

  Pin Sensor (semua PZEM digabung satu bus):
    HardwareSerial2: RX2 = GPIO16, TX2 = GPIO17
    PZEM1 alamat Modbus 0x01
    PZEM2 alamat Modbus 0x03
    PZEM3 alamat Modbus 0x02

  Catatan wiring:
    Semua pin TX ketiga PZEM digabung ke RX2 (GPIO16).
    Semua pin RX ketiga PZEM digabung ke TX2 (GPIO17).
    Pastikan setiap PZEM sudah diset alamat unik di atas SEBELUM
    digabung ke satu bus. Kalau belum, set satu per satu pakai
    UART sendiri dulu dengan fungsi setAddress().

  Format payload uplink (26 byte, FPort 1), urutan sesuai codec ChirpStack:
    byte 0-5   : voltage R, S, T   (x10, contoh 220.5V -> 2205)
    byte 6-11  : power R, S, T     (bulat, Watt)
    byte 12-17 : current R, S, T   (x100, contoh 1.25A -> 125)
    byte 18-23 : energy R, S, T    (x10, contoh 12.3 kWh -> 123)
    byte 24-25 : frequency         (x10, contoh 50.0 Hz -> 500, satu nilai bersama)
*/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <PZEM004Tv30.h>

// ---------- Struct harus di paling atas, sebelum prototype apapun ----------
struct SensorData {
  float voltage1, current1, power1, energy1;
  float voltage2, current2, power2, energy2;
  float voltage3, current3, power3, energy3;
  float frequency;
};

// ---------- Forward declaration manual (wajib, hindari error compile) ----------
void doSend(osjob_t* j);
SensorData readAllPZEM();
void write16(uint8_t* buf, int idx, uint16_t val);

// ---------- Kredensial OTAA ----------
// DevEUI dan AppEUI harus little endian (LSB dulu).
// AppKey tetap apa adanya (MSB / big endian).

static const u1_t PROGMEM DEVEUI[8] = { 0x50, 0xD5, 0x97, 0x23, 0x55, 0xB9, 0x5B, 0x84 };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = {
  0x1E, 0xEA, 0x4F, 0x49, 0x96, 0x73, 0xAF, 0x37,
  0x31, 0x64, 0x55, 0x9D, 0xAE, 0x5B, 0x7E, 0x8E
};
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// ---------- Pin map LMIC ----------
const lmic_pinmap lmic_pins = {
  .nss = 15,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 32,
  .dio = { 4, 25, LMIC_UNUSED_PIN },
};

// ---------- Objek PZEM (satu bus, tiga alamat) ----------
PZEM004Tv30 pzem1(Serial2, 16, 17, 0x01);
PZEM004Tv30 pzem2(Serial2, 16, 17, 0x03);
PZEM004Tv30 pzem3(Serial2, 16, 17, 0x02);

osjob_t sendjob;
const unsigned TX_INTERVAL = 60; // detik antar pengiriman

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Mulai monitoring daya 3 fase"));

  // Warmup sensor sebelum dibaca, hindari pembacaan awal yang tidak stabil
  delay(5000);
  Serial.println(F("Warmup sensor selesai"));

  Serial.println(F("Sebelum os_init()"));
  os_init();
  Serial.println(F("Setelah os_init(), sebelum LMIC_reset()"));

  LMIC_reset();
  Serial.println(F("Setelah LMIC_reset()"));

  // AS923-3 region settings sesuai konfigurasi gateway
  LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

  LMIC_startJoining();
  Serial.println(F("Proses join OTAA dimulai..."));
}

void loop() {
  os_runloop_once();
}

void onEvent(ev_t ev) {
  switch (ev) {
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;

    case EV_JOINED:
      Serial.println(F("EV_JOINED - berhasil terhubung ke gateway"));
      LMIC_setLinkCheckMode(0);
      doSend(&sendjob);
      break;

    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;

    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;

    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE - paket terkirim"));
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("Diterima ACK dari gateway"));
      }
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), doSend);
      break;

    default:
      break;
  }
}

void doSend(osjob_t* j) {
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("Pengiriman sebelumnya belum selesai, lewati siklus ini"));
  } else {
    SensorData data = readAllPZEM();

    Serial.printf("Fase R: %.1fV %.2fA %.0fW %.1fkWh\n", data.voltage1, data.current1, data.power1, data.energy1);
    Serial.printf("Fase S: %.1fV %.2fA %.0fW %.1fkWh\n", data.voltage2, data.current2, data.power2, data.energy2);
    Serial.printf("Fase T: %.1fV %.2fA %.0fW %.1fkWh\n", data.voltage3, data.current3, data.power3, data.energy3);
    Serial.printf("Frequency: %.1fHz\n", data.frequency);

    uint8_t payload[26];

    write16(payload, 0,  (uint16_t)(data.voltage1 * 10));
    write16(payload, 2,  (uint16_t)(data.voltage2 * 10));
    write16(payload, 4,  (uint16_t)(data.voltage3 * 10));

    write16(payload, 6,  (uint16_t)(data.power1));
    write16(payload, 8,  (uint16_t)(data.power2));
    write16(payload, 10, (uint16_t)(data.power3));

    write16(payload, 12, (uint16_t)(data.current1 * 100));
    write16(payload, 14, (uint16_t)(data.current2 * 100));
    write16(payload, 16, (uint16_t)(data.current3 * 100));

    write16(payload, 18, (uint16_t)(data.energy1 * 10));
    write16(payload, 20, (uint16_t)(data.energy2 * 10));
    write16(payload, 22, (uint16_t)(data.energy3 * 10));

    write16(payload, 24, (uint16_t)(data.frequency * 10));

    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    Serial.println(F("Paket data dijadwalkan kirim ke gateway"));
  }
}

SensorData readAllPZEM() {
  SensorData data;

  data.voltage1 = pzem1.voltage();
  data.current1 = pzem1.current();
  data.power1   = pzem1.power();
  data.energy1  = pzem1.energy();

  data.voltage2 = pzem2.voltage();
  data.current2 = pzem2.current();
  data.power2   = pzem2.power();
  data.energy2  = pzem2.energy();

  data.voltage3 = pzem3.voltage();
  data.current3 = pzem3.current();
  data.power3   = pzem3.power();
  data.energy3  = pzem3.energy();

  // Frekuensi jaringan AC sama untuk ketiga fase, ambil dari salah satu sensor saja
  data.frequency = pzem1.frequency();

  // Kalau komunikasi gagal, PZEM mengirim NAN. Ganti jadi 0 supaya payload tetap valid.
  if (isnan(data.voltage1)) data.voltage1 = 0;
  if (isnan(data.current1)) data.current1 = 0;
  if (isnan(data.power1))   data.power1   = 0;
  if (isnan(data.energy1))  data.energy1  = 0;
  if (isnan(data.voltage2)) data.voltage2 = 0;
  if (isnan(data.current2)) data.current2 = 0;
  if (isnan(data.power2))   data.power2   = 0;
  if (isnan(data.energy2))  data.energy2  = 0;
  if (isnan(data.voltage3)) data.voltage3 = 0;
  if (isnan(data.current3)) data.current3 = 0;
  if (isnan(data.power3))   data.power3   = 0;
  if (isnan(data.energy3))  data.energy3  = 0;
  if (isnan(data.frequency)) data.frequency = 0;

  return data;
}

void write16(uint8_t* buf, int idx, uint16_t val) {
  buf[idx]     = val >> 8;
  buf[idx + 1] = val & 0xFF;
}
