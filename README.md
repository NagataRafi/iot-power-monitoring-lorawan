# Sistem IoT Monitoring Parameter Kelistrikan Gardu Listrik 20kV

Implementasi lengkap sistem IoT berbasis LoRaWAN untuk monitoring real-time parameter kelistrikan tiga fasa (tegangan, arus, daya, energi, frekuensi, faktor daya) pada panel distribusi tegangan menengah.

## Fitur Utama

- Monitoring 6 parameter kelistrikan per fasa (tegangan, arus, daya, energi, frekuensi, faktor daya)
- Komunikasi wireless LoRaWAN AS923-3 hingga 750 meter
- Dashboard Grafana real-time dengan visualisasi per fasa dan agregat
- Notifikasi alarm via Telegram
- Logging data ke InfluxDB dan Google Sheets
- Latensi end-to-end rata-rata 5.17 detik
- Packet Delivery Ratio 90.71% jarak jauh, 100% line-of-sight
- Akurasi sensor 96.98% - 100%

## Spesifikasi Hardware

### Node Sensor (ESP32)

- Mikrocontroller: ESP32-WROOM-DA 38-pin DevKit V1
- Tenaga: USB 5V atau Li-ion (4200mAh, 3.7V nominal)
- Sensor tegangan/arus/daya: 3x PZEM-004T v3.0 (Modbus RTU)
  - Alamat Modbus: 0x01, 0x02, 0x03
  - Rated: 100A / 240V AC
  - Akurasi tegangan: 0.5%
  - Akurasi arus: 1%
- Modul LoRa: Marnov MRV-RFM95W (SX1276)
  - Frekuensi: AS923-3 (923.2-923.6 MHz)
  - Daya TX: +20dBm (100mW)
  - Sensitivitas RX: -139dBm
- Koneksi UART:
  - Serial2 (GPIO16 RX, GPIO17 TX): PZEM-001 (0x01), PZEM-002 (0x02)
  - Serial1 (GPIO21 RX, GPIO22 TX): PZEM-003 (0x03)
- Koneksi SPI LoRa:
  - MOSI: GPIO23
  - MISO: GPIO19
  - CLK: GPIO18
  - CS: GPIO5
  - RST: GPIO14
  - DIO0: GPIO4 (interrupt-capable)

### Gateway (Raspberry Pi)

- Mikrokomputer: Raspberry Pi 4 Model B (4GB RAM)
- Concentrator LoRa: Seeed Studio WM1302 (SPI mode)
  - Radio: SX1302 + SX1250
  - Koneksi SPI ke Raspberry Pi GPIO
- Sistem Operasi: Raspberry Pi OS (64-bit)
- Network: Ethernet atau Wi-Fi

## Arsitektur Sistem

```
PZEM Sensors (3x) 
      |
      | Modbus RTU
      v
ESP32-WROOM-DA
      |
      | LoRaWAN OTAA
      v
WM1302 Gateway (Raspberry Pi 4)
      |
      | LoRaWAN packet forwarder
      v
ChirpStack v4.17.0
      |
      | MQTT
      v
Mosquitto MQTT Broker
      |
      +-----> Node-RED
      |          |
      |          | Processing & Calculation
      |          v
      |       InfluxDB v1.6.7
      |          |
      |          v
      |       Grafana v13.0.1
      |          |
      |    +-----+-----+
      |    |     |     |
      |    v     v     v
      |   Web  Google Telegram
      |      Sheets  Bot
      |
      +-----> Google Sheets Integration
      |       (via Google Apps Script)
      |
      +-----> Telegram Bot
           (Notifications)
```

## Stack Software

### Di Node (ESP32)

```
Arduino IDE
├── MCCI LoRaWAN LMIC v6.0.1
├── PZEM004Tv30 v1.2.1 (Jakub Mandula)
├── HardwareSerial (built-in)
└── Custom payload encoder
```

### Di Gateway (Raspberry Pi)

```
Raspberry Pi OS 64-bit
├── ChirpStack v4.17.0 (SQLite)
├── ChirpStack Gateway Bridge v4.1.1
├── sx1302_hal v2.1.0 (lora_pkt_fwd)
├── Mosquitto MQTT v2.x
├── InfluxDB v1.6.7
├── Grafana v13.0.1
├── Node-RED v3.1.x
└── Systemd services
    ├── chirpstack-sqlite
    ├── chirpstack-gateway-bridge
    ├── mosquitto
    ├── influxdb
    ├── grafana-server
    └── node-red
```

## Konfigurasi Detail

### A. ESP32 Arduino Setup

1. Buka Arduino IDE, pilih Board: ESP32 Dev Module
2. Setting:
   - Upload Speed: 115200
   - CPU Frequency: 80 MHz
   - Flash Frequency: 40 MHz
   - Flash Mode: DIO
   - Partition Scheme: Huge APP (3MB No OTA / 1MB SPIFFS)

3. Instalasi libraries (Sketch > Include Library > Manage Libraries):
   - MCCI LoRaWAN LMIC Library v6.0.1
   - PZEM004Tv30 v1.2.1
   - ArduinoJson v6.x

### B. Pin Configuration (LMIC_BeginEnd.h)

```c
#define CFG_sx1276_radio 1
#define LMIC_USE_INTERRUPTS 0  // PENTING: disable untuk ESP32 Arduino Core 3.x

// SPI Pins
#define PIN_SPI_NSS    5   // CS
#define PIN_SPI_SCK    18  // CLK
#define PIN_SPI_MOSI   23  // MOSI
#define PIN_SPI_MISO   19  // MISO

// LoRa Control Pins
#define PIN_LORA_RST   14  // RST
#define PIN_LORA_DIO0  4   // DIO0 (must be interrupt-capable)

// UART Pins (software defined untuk PZEM)
// Serial2: RX=16, TX=17 (PZEM 0x01, 0x02)
// Serial1: RX=21, TX=22 (PZEM 0x03)
```

### C. PZEM Sensor Configuration

Split UART Bus Architecture:

```
ESP32 Serial2 (GPIO16/17)     ESP32 Serial1 (GPIO21/22)
    |                              |
    +--PZEM-004T (0x01)           PZEM-004T (0x03)
    |
    +--PZEM-004T (0x02)
```

Modbus RTU Settings per sensor:
- Baud Rate: 9600
- Data Bits: 8
- Stop Bits: 1
- Parity: Even
- Slave Address: 0x01, 0x02, 0x03 (berbeda untuk setiap sensor)

Konfigurasi alamat sensor (hanya dilakukan sekali):

```cpp
// Reset address via broadcast (hanya jika diperlukan)
PZEM004Tv30 pzem(Serial2, 0xF8);  // 0xF8 = broadcast
pzem.setAddress(0x01);  // Set ke address 0x01

// Normal operation
PZEM004Tv30 pzem1(Serial2, 0x01);
PZEM004Tv30 pzem2(Serial2, 0x02);
PZEM004Tv30 pzem3(Serial1, 0x03);
```

Daya sensor PZEM dari pin 3.3V ESP32 (bukan 5V).

### D. LoRaWAN Configuration (OTAA)

Device Credentials:
- DevEUI: 845bb9552397d550
- AppEUI: 0000000000000000
- AppKey: 1eea4f499673af373164559dae5b7e8e

Region: AS923-3
- Uplink channel: 923.2 MHz
- RX2 frequency: 923.2 MHz
- RX2 datarate: SF10 BW125

Setup di Arduino:

```cpp
static const u1_t PROGMEM DEVEUI[8] = {0x50, 0xd5, 0x97, 0x23, 0x55, 0xb9, 0x5b, 0x84};
static const u1_t PROGMEM APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const u1_t PROGMEM APPKEY[16] = {0x1e, 0xea, 0x4f, 0x49, 0x96, 0x73, 0xaf, 0x37, 0x31, 0x64, 0x55, 0x9d, 0xae, 0x5b, 0x7e, 0x8e};

void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
LMIC_startJoining();
```

### E. Payload Format (26 bytes, Big-Endian)

```
Byte 0-1   : Tegangan Fasa R (V * 10) - uint16
Byte 2-3   : Tegangan Fasa S (V * 10) - uint16
Byte 4-5   : Tegangan Fasa T (V * 10) - uint16
Byte 6-7   : Arus Fasa R (A * 100) - uint16
Byte 8-9   : Arus Fasa S (A * 100) - uint16
Byte 10-11 : Arus Fasa T (A * 100) - uint16
Byte 12-15 : Daya Fasa R (W) - uint32
Byte 16-19 : Daya Fasa S (W) - uint32
Byte 20-23 : Daya Fasa T (W) - uint32
Byte 24    : Frekuensi (Hz * 10) - uint8
Byte 25    : Faktor Daya rata-rata (0-100) - uint8
```

Contoh encoder Arduino:

```cpp
uint8_t payload[26];
uint16_t v_r = (uint16_t)(pzem1.voltage() * 10);
uint16_t v_s = (uint16_t)(pzem2.voltage() * 10);
uint16_t v_t = (uint16_t)(pzem3.voltage() * 10);
// ... dst untuk current dan power

payload[0] = (v_r >> 8) & 0xFF;
payload[1] = v_r & 0xFF;
// ... copy setiap parameter dengan format big-endian
```

### F. Raspberry Pi Gateway Setup

#### 1. Update sistem

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y git build-essential pkg-config libmosquitto-dev
```

#### 2. Install ChirpStack

Download dan install dari repository resmi:

```bash
# Add ChirpStack repository
sudo apt-get install -y curl gpg
curl https://repos.chirpstack.io/chirpstack-repo.gpg | sudo apt-key add -
echo "deb https://repos.chirpstack.io/chirpstack ubuntu focal main" | sudo tee /etc/apt/sources.list.d/chirpstack.list
sudo apt update

# Install ChirpStack
sudo apt install -y chirpstack chirpstack-gateway-bridge

# Verifikasi service name
systemctl list-unit-files | grep chirpstack
```

#### 3. Konfigurasi ChirpStack

File: /etc/chirpstack/chirpstack.toml

```toml
[general]
log_level="info"
log_format="json"

[database]
dsn="sqlite:////var/lib/chirpstack/chirpstack.sqlite"

[api]
bind="0.0.0.0:8080"
secret="your-secret-key-here"

[integration]
backend="mqtt"

[integration.mqtt]
broker="tcp://localhost:1883"
client_id="chirpstack"
auth_type="username_password"
username="chirpstack"
password="chirpstack"
```

#### 4. Konfigurasi ChirpStack Gateway Bridge

File: /etc/chirpstack-gateway-bridge/chirpstack-gateway-bridge.toml

```toml
[general]
log_level="info"

[backend]
type="semtech"
bind="0.0.0.0:3001"

[integration.mqtt]
auth_type="username_password"
broker="tcp://localhost:1883"
username="chirpstack"
password="chirpstack"
client_id="chirpstack-gateway-bridge"
```

#### 5. Install Semtech Packet Forwarder (untuk WM1302)

```bash
cd ~/
git clone https://github.com/Lora-net/sx302_hal.git
cd sx302_hal
git checkout v2.1.0
make clean
make

# Copy global config
sudo cp ~/sx302_hal/lora_pkt_fwd/global_conf.json /etc/lora/
```

Edit /etc/lora/global_conf.json. PENTING: Hilangkan comment dan pastikan JSON valid:

```json
{
  "SX130x_conf": {
    "spidev": "/dev/spidev0.0"
  },
  "radio_0": {
    "enable": true,
    "type": "SX1302",
    "freq": 923400000,
    "tx_enable": true,
    "tx_freq_min": 923200000,
    "tx_freq_max": 923600000
  },
  "radio_1": {
    "enable": true,
    "type": "SX1250",
    "freq": 923200000
  }
}
```

#### 6. Install Mosquitto MQTT

```bash
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Create MQTT user
sudo mosquitto_passwd -c /etc/mosquitto/passwd chirpstack
# password: chirpstack
```

#### 7. Install InfluxDB

```bash
# Download InfluxDB 1.6.7 armv7
wget https://dl.influxdata.com/influxdb/releases/influxdb-1.6.7_linux_armv7.tar.gz
tar xvfz influxdb-1.6.7_linux_armv7.tar.gz
sudo cp influxdb-1.6.7-1/usr/bin/influxd /usr/local/bin/
sudo cp influxdb-1.6.7-1/usr/bin/influx /usr/local/bin/

# Setup service
sudo tee /etc/systemd/system/influxdb.service > /dev/null <<EOF
[Unit]
Description=InfluxDB
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/influxd -config /etc/influxdb/influxdb.conf
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable influxdb
sudo systemctl start influxdb

# Buat database
influx -execute 'CREATE DATABASE pzem_data'
```

#### 8. Install Grafana

```bash
sudo apt install -y adduser libfontconfig1
wget https://dl.grafana.com/oss/release/grafana_13.0.1_armv7.deb
sudo dpkg -i grafana_13.0.1_armv7.deb

sudo systemctl enable grafana-server
sudo systemctl start grafana-server
```

Akses: http://192.168.100.55:3000 (username: admin, password: admin)

#### 9. Install Node-RED

```bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/install.sh)

# Enable autostart
sudo systemctl enable nodered
sudo systemctl start nodered
```

Akses: http://192.168.100.55:1880

### G. Node-RED Flow Configuration

Import JSON flow berikut ke Node-RED untuk Processing dan InfluxDB logging:

Flow mencakup:
- MQTT subscribe dari ChirpStack (payload decoder)
- Calculation node untuk daya rata-rata dan energi
- InfluxDB output
- Google Sheets API integration
- Telegram notification logic

### H. Grafana Dashboard Setup

1. Add data source InfluxDB:
   - URL: http://localhost:8086
   - Database: pzem_data
   - Measurement: pzem_3phase

2. Import dashboard JSON untuk visualisasi:
   - Per-phase panels (tegangan, arus, daya, frekuensi, faktor daya)
   - Aggregate/total panels
   - Time-series graphs
   - Geomap untuk lokasi node dan gateway
   - Alert threshold visualization

## Hasil Testing & Akurasi

### Akurasi Sensor PZEM-004T

| Parameter | Akurasi |
|-----------|---------|
| Tegangan | 99.77% |
| Arus | 97.89% |
| Daya | 98.55% |
| Frekuensi | 100% |
| Faktor Daya | 97.59% |

Pengujian menggunakan multimeter digital DT9205A sebagai referensi pada tiga jenis beban:
- Resistif (Setrika): 245V
- Induktif (Kipas): 244V
- Kapasitif (Lampu): 244V

### Komunikasi LoRaWAN

| Jarak | PDR | Kondisi |
|-------|-----|---------|
| 50-100m | 100% | Line-of-sight |
| 150-300m | 95% | Partial obstacle |
| 500-750m | 85-90% | Multi-wall |

Rata-rata PDR: 90.71% (140 paket, 7 test point)

### Latensi Sistem

- End-to-end average: 5.17 detik
- Notification delay (Telegram): 67-70 detik
- Uptime: 95.74%

### Test Duration

Aktual test: 233 menit (~3.88 jam) dengan system monitoring continuous

## Instalasi Cepat

### 1. Setup ESP32

```bash
# Buka Arduino IDE
# 1. Tools > Board > ESP32 > ESP32 Dev Module
# 2. Tools > Upload Speed > 115200
# 3. Sketch > Include Library > Manage Libraries
#    - Search "MCCI LoRaWAN LMIC"
#    - Search "PZEM004T"
#    - Search "ArduinoJson"
# 4. Copy pin config ke LMIC_BeginEnd.h (lihat Section B)
# 5. Copy payload encoder (Section E)
# 6. Upload ke ESP32
```

### 2. Setup Raspberry Pi Gateway

```bash
# SSH ke Raspberry Pi
ssh pi@192.168.100.55

# Clone repository atau download setup script
git clone [repo-url]
cd [repo-dir]

# Run setup automation script (optional)
sudo bash setup_gateway.sh

# Atau setup manual (lihat Section F step 1-9)
```

### 3. Verify Services

```bash
# Cek semua service berjalan
sudo systemctl status chirpstack-sqlite
sudo systemctl status chirpstack-gateway-bridge
sudo systemctl status mosquitto
sudo systemctl status influxdb
sudo systemctl status grafana-server
sudo systemctl status nodered
```

### 4. Verify Device OTAA Join

ChirpStack Console: http://192.168.100.55:8080
- Login (default credentials perlu dikonfigurasi)
- Cek Applications > Device > Recent frames
- Tunggu hingga join successful

### 5. Access Grafana Dashboard

http://192.168.100.55:3000
- Login: admin / admin
- Import dashboard JSON (tersedia di repo)

## Troubleshooting

### ESP32 Not Joining LoRaWAN

Penyebab umum dan solusi:

1. Pin DIO0 tidak benar atau bukan interrupt-capable
   - Solusi: Gunakan GPIO4 atau GPIO2, hindari GPIO34
   
2. LMIC_USE_INTERRUPTS = 1 dengan ESP32 Arduino Core 3.x
   - Solusi: Set LMIC_USE_INTERRUPTS = 0

3. ChirpStack tidak menerima join request
   - Cek: ChirpStack service status
   - Cek: Gateway uplink configuration benar
   - Cek: Device credentials cocok dengan server

4. TX power limit di AS923
   - Solusi: Set max TX power sesuai region

### PZEM Sensor Tidak Respond

1. Address mismatch
   - Solusi: Reset sensor ke address 0xF8 (broadcast) lalu set individual address

2. UART bus contention (ketiga sensor di 1 UART)
   - Solusi: Gunakan split UART (Serial2 + Serial1)

3. Baud rate mismatch
   - Cek: Sensor setting di physical device (switch atau AT command)

4. Sensor tidak powered
   - Cek: Pin 3.3V (bukan 5V) untuk PZEM
   - Cek: GND connection

### ChirpStack Service Tidak Start

1. Database locked
   - Solusi: Restart service atau manual unlock SQLite DB

2. Port 8080 sudah digunakan
   - Solusi: Change port di config atau kill process

3. MQTT auth failed
   - Solusi: Verify Mosquitto credentials cocok di config

### Gateway Downlink Fail

1. RX2 frequency salah
   - AS923-3: RX2 = 923.2 MHz (bukan 916.6 MHz)

2. SPI concentrator tidak detect
   - Solusi: Verify WM1302 powered, SPI enabled di Raspberry Pi
   - Command: `raspi-config` > Interfacing Options > SPI

3. Packet forwarder log error
   - Solusi: Cek JSON config valid (no comments), permissions

## Lessons Learned & Best Practices

### Hardware

- Gunakan decoupling capacitor 100µF pada 3.3V rail dekat LoRa module untuk stabilitas
- Hindari GPIO12 (strapping pin) untuk sensor TX
- GPIO34 input-only, hindari untuk critical control
- PZEM dari 3.3V lebih stabil daripada 5V dalam setup ini

### LoRaWAN

- ChirpStack service name: chirpstack-sqlite (bukan chirpstack)
- DevNonce table di SQLite: device_keys (bukan device_nonce)
- JSON comments di config break packet forwarder silently
- RX2 frequency AS923-3 = 923.2 MHz, bukan 916.6 MHz
- Enable tx_enable = true di radio config

### Sensors

- Split UART bus untuk 3x PZEM: hindari bus contention
- Reset address menggunakan broadcast 0xF8
- Verify sensor dengan AT command atau physical switch sebelum implementasi

### Software Development

- Test komponen individually sebelum full integration
- Gunakan serial monitor ekstensif untuk debugging
- Log semua uplink/downlink frames untuk analisis
- Keep single version current file, delete stale backups

## Repository Structure

```
IoT-Power-Monitoring-LoRaWAN/
├── README.md
├── LICENSE
├── HARDWARE_CONFIG.md
├── SOFTWARE_SETUP.md
├── TROUBLESHOOTING.md
├── firmware/
│   ├── esp32_node/
│   │   ├── main.ino
│   │   ├── lmic_config.h
│   │   ├── payload_encoder.h
│   │   ├── pzem_handler.h
│   │   └── config.h
│   └── libraries/
│       ├── MCCI_LoRaWAN_LMIC_v6.0.1.zip
│       └── PZEM004Tv30_v1.2.1.zip
├── gateway/
│   ├── setup_gateway.sh
│   ├── chirpstack_config/
│   │   ├── chirpstack.toml
│   │   ├── chirpstack-gateway-bridge.toml
│   │   └── global_conf.json
│   ├── node-red/
│   │   └── flows.json
│   └── startup_gateway.sh
├── dashboard/
│   ├── grafana_dashboard.json
│   ├── influxdb_setup.sh
│   └── telegram_bot_setup.md
├── documentation/
│   ├── LAPORAN_AKHIR.pdf
│   ├── datasheet_esp32.pdf
│   ├── datasheet_pzem004t.pdf
│   ├── datasheet_rfm95w.pdf
│   └── datasheet_wm1302.pdf
└── test_results/
    ├── accuracy_test.xlsx
    ├── lora_range_test.xlsx
    └── latency_analysis.xlsx
```

## Performance Specifications

| Aspek | Target | Hasil |
|-------|--------|-------|
| Akurasi sensor | >95% | 96.98% - 100% |
| Jangkauan LoRa | 500m+ | 750m tested |
| PDR LoRa | >85% | 90.71% average |
| Latensi E2E | <5 detik | 5.17 detik |
| Uptime | >90% | 95.74% |
| Notification delay | <120 detik | 67-70 detik |

## Kontributor

**Muhammad Rafi Nagata Afendi** (2331130049)
- Lead Engineer
- ESP32 firmware, LoRaWAN configuration
- Email: nagatapct@gmail.com

**Thariza Revania Anggani** (2331130043)
- Co-Engineer
- Dashboard design, testing
- Email: tharizarevania18@gmail.com

## Pembimbing

- Pembimbing I: Ginanjar Suwasono Adi, S.ST., M.Sc.
- Pembimbing II: Dodit Suprianto, S.KOM., M.T.

Politeknik Negeri Malang, 2026

## Lisensi

MIT License - lihat LICENSE file untuk detail

## Referensi & Datasheet

1. MCCI LoRaWAN LMIC Repository: https://github.com/mcci-catena/arduino-lmic
2. ChirpStack Documentation: https://www.chirpstack.io/
3. Semtech SX1276 Datasheet (RFM95W)
4. Seeed Studio WM1302 Documentation
5. PZEM-004T Manual dan Modbus Protocol
6. Grafana Time-Series Visualization Guide
7. Node-RED Cookbook

---

Dokumentasi lengkap dan update terbaru tersedia di GitHub repository.
Untuk pertanyaan teknis atau issue, buat issue baru di repository.
