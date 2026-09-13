// ============================================================
// TEST OTAA LMIC - MRV-RFM95W (SX1276) + ESP32
// Device  : TA2026
// Region  : AS923-3
// Aktivasi: OTAA
// ============================================================

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// DevEUI: 845bb9552397d550 - format LSB (balik urutannya)
static const u1_t PROGMEM DEVEUI[8] = { 0x50, 0xd5, 0x97, 0x23, 0x55, 0xb9, 0x5b, 0x84 };

// AppEUI: 0000000000000000 - format LSB
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// AppKey: 1eea4f499673af373164559dae5b7e8e - format MSB (urutan asli)
static const u1_t PROGMEM APPKEY[16] = {
  0x1e, 0xea, 0x4f, 0x49, 0x96, 0x73, 0xaf, 0x37,
  0x31, 0x64, 0x55, 0x9d, 0xae, 0x5b, 0x7e, 0x8e
};

void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// Mapping pin MRV-RFM95W ke ESP32
const lmic_pinmap lmic_pins = {
  .nss  = 15,
  .rxtx = LMIC_UNUSED_PIN,
  .rst  = 32,
  .dio  = { 2, 25, LMIC_UNUSED_PIN },
};

static osjob_t sendjob;
bool joined = false;

// Payload dummy untuk test
static uint8_t mydata[] = { 0x01, 0x02, 0x03 };

void do_send(osjob_t* j) {
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println("TX pending, skip.");
  } else {
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    Serial.println("Packet dijadwalkan untuk TX.");
  }
}

void onEvent(ev_t ev) {
  Serial.print("[EVENT] ");
  switch (ev) {
    case EV_JOINING:
      Serial.println("Mencoba JOIN...");
      break;

    case EV_JOINED:
      Serial.println("JOIN BERHASIL!");
      joined = true;
      {
        u4_t netid = 0;
        devaddr_t devaddr = 0;
        u1_t nwkKey[16], artKey[16];
        LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
        Serial.print("  NetID   : "); Serial.println(netid, HEX);
        Serial.print("  DevAddr : "); Serial.println(devaddr, HEX);
      }
      LMIC_setLinkCheckMode(0);
      // Kirim data pertama setelah join
      do_send(&sendjob);
      break;

    case EV_JOIN_FAILED:
      Serial.println("JOIN GAGAL. Cek DevEUI/AppEUI/AppKey di ChirpStack.");
      break;

    case EV_REJOIN_FAILED:
      Serial.println("REJOIN GAGAL.");
      break;

    case EV_TXCOMPLETE:
      Serial.println("TX selesai.");
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println("  ACK diterima.");
      }
      if (LMIC.dataLen) {
        Serial.print("  Downlink diterima, panjang: ");
        Serial.println(LMIC.dataLen);
      }
      // Jadwalkan TX berikutnya setelah 60 detik
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(60), do_send);
      break;

    case EV_JOIN_TXCOMPLETE:
      Serial.println("JOIN TX selesai, menunggu respons...");
      break;

    case EV_TXSTART:
      Serial.println("TX dimulai.");
      break;

    case EV_RXSTART:
      break;

    case EV_RESET:
      Serial.println("RESET.");
      break;

    default:
      Serial.print("Event tidak dikenal: ");
      Serial.println((unsigned)ev);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== TEST OTAA TA2026 ===");
  Serial.println("Region  : AS923-3");
  Serial.println("DevEUI  : 845bb9552397d550");
  Serial.println("Memulai LMIC...");

  os_init();
  LMIC_reset();

  // Nonaktifkan link check dan ADR untuk test awal
  LMIC_setLinkCheckMode(0);
  LMIC_setAdrMode(0);

  // Set data rate dan TX power awal
  LMIC_setDrTxpow(DR_SF7, 14);

  // Mulai JOIN
  LMIC_startJoining();
  Serial.println("JOIN request dikirim, menunggu respons gateway...");
}

void loop() {
  os_runloop_once();
}