/*
 * PZEM2 -> LoRaWAN OTAA -> ChirpStack
 * ESP32 38-Pin DevKit V1
 *
 * Sensor: PZEM2 di UART2 (RX=14, TX=13), default address (tanpa address eksplisit)
 * LoRa  : MRV-RFM95W (SX1276)
 *   NSS  = GPIO15
 *   RST  = GPIO32*
 *   DIO0 = GPIO4
 *   DIO1 = GPIO25
 *   SCK=18, MISO=19, MOSI=23
 *
 * CATATAN PENTING:
 * Kapasitor decoupling di rail 3.3V LoRa BELUM dipasang.
 * Risiko reset/watchdog saat TX (SX1276 menarik arus hingga 120mA) masih tinggi.
 * Indikator LED:
 *   - Berkedip cepat terus      -> macet sebelum join (cek pin SPI/LoRa)
 *   - Nyala solid sebentar lalu mati -> kemungkinan reset terjadi (watchdog)
 *   - Berkedip 1x panjang       -> join berhasil, data terkirim
 *
 * Library: MCCI LoRaWAN LMIC, PZEM004Tv30
 */

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <PZEM004Tv30.h>

// ===== PZEM2 setup =====
HardwareSerial pzemSerial2(2);
#define PZEM2_RX 14
#define PZEM2_TX 13
PZEM004Tv30 pzem2(pzemSerial2, PZEM2_RX, PZEM2_TX);

// ===== LoRaWAN OTAA credentials (LSB format untuk LMIC) =====
// DevEUI: 845bb9552397d550
static const u1_t PROGMEM DEVEUI[8] = { 0x50, 0xD5, 0x97, 0x23, 0x55, 0xB9, 0x5B, 0x84 };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

// AppEUI: 0000000000000000
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

// AppKey: 1eea4f499673af373164559dae5b7e8e (MSB format, tidak dibalik)
static const u1_t PROGMEM APPKEY[16] = {
  0x1E, 0xEA, 0x4F, 0x49, 0x96, 0x73, 0xAF, 0x37,
  0x31, 0x64, 0x55, 0x9D, 0xAE, 0x5B, 0x7E, 0x8E
};
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// ===== Pin mapping LoRa =====
const lmic_pinmap lmic_pins = {
  .nss = 15,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 32,
  .dio = { 4, 25, LMIC_UNUSED_PIN },
};

#define LED_PIN 2

static osjob_t sendjob;
const unsigned TX_INTERVAL = 30; // detik antar pengiriman

void blinkFast(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(80);
    digitalWrite(LED_PIN, LOW);
    delay(80);
  }
}

void blinkLongOk() {
  digitalWrite(LED_PIN, HIGH);
  delay(1500);
  digitalWrite(LED_PIN, LOW);
}

void onEvent(ev_t ev) {
  switch (ev) {
    case EV_JOINING:
      blinkFast(1);
      break;
    case EV_JOINED:
      blinkLongOk();
      LMIC_setLinkCheckMode(0);
      break;
    case EV_JOIN_FAILED:
      blinkFast(5);
      break;
    case EV_TXCOMPLETE:
      blinkFast(2);
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), doSend);
      break;
    default:
      break;
  }
}

void doSend(osjob_t* j) {
  if (LMIC.opmode & OP_TXRXPEND) {
    // masih ada transmisi pending, lewati siklus ini
    return;
  }

  float voltage = pzem2.voltage();
  float current = pzem2.current();
  float power    = pzem2.power();

  if (isnan(voltage)) {
    voltage = 0;
    current = 0;
    power = 0;
  }

  // Payload sederhana: voltage*10, current*100, power*10 -> uint16 masing-masing (6 byte)
  uint16_t v = (uint16_t)(voltage * 10);
  uint16_t c = (uint16_t)(current * 100);
  uint16_t p = (uint16_t)(power * 10);

  uint8_t payload[6];
  payload[0] = v >> 8;   payload[1] = v & 0xFF;
  payload[2] = c >> 8;   payload[3] = c & 0xFF;
  payload[4] = p >> 8;   payload[5] = p & 0xFF;

  LMIC_setTxData2(1, payload, sizeof(payload), 0);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Beri waktu sensor & sistem stabil sebelum radio aktif
  delay(2000);

  os_init();
  LMIC_reset();

  // AS923-3: sesuaikan jika ChirpStack pakai sub-band/channel khusus
  LMIC_setLinkCheckMode(0);
  LMIC_setDrTxpow(DR_SF7, 14);

  doSend(&sendjob);
}

void loop() {
  Serial.println("=== BOOT ===");
  Serial.println(ESP.getResetReason());
  os_runloop_once();
}
