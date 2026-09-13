# Panduan Troubleshooting Lengkap

Koleksi masalah yang dijumpai selama development, testing, dan solusinya. Gunakan sebagai reference ketika mengalami issue.

## ESP32 & Firmware Issues

### 1. ESP32 Tidak Terdeteksi di Arduino IDE

**Gejala:**
- Arduino IDE tidak detect serial port COM
- Atau serial port ada tapi upload fails dengan timeout

**Penyebab umum:**
- USB cable rusak atau tidak support data transfer
- Driver CH340 / CP2102 tidak installed
- Bootloader corrupt
- USB conflict dengan software lain

**Solusi:**

```bash
# Step 1: Verifikasi USB cable (gunakan USB 2.0, bukan USB 3.0)
# Test dengan cable lain yang pasti working

# Step 2: Install driver CH340 (untuk ESP32-WROOM-DA)
# Windows: Download dari https://www.wch.cn/download/ch341ser_exe.html
# Linux: sudo apt install -y ch340-dkms
# Mac: Download dari vendor website

# Step 3: Check device terdaftar
# Windows: Device Manager > Ports (COM & LPT)
# Linux: ls -l /dev/ttyUSB* atau /dev/ttyACM*
# Mac: ls -l /dev/cu.* atau /dev/tty.*

# Step 4: Reset bootloader
# Tekan BOOT button saat upload, lepas ketika compile selesai
# Atau dengan PuTTY: esptool.py write_flash_status --non-interactive

# Step 5: Erase flash dan reflash
# Download esptool.py: https://github.com/espressif/esptool
# python esptool.py -p COM3 erase_flash  (Windows)
# python3 esptool.py -p /dev/ttyUSB0 erase_flash  (Linux)
```

### 2. LMIC Library Issues (LoRaWAN Join Failed)

**Gejala:**
- Arduino serial monitor: "Joining..."
- Stuck > 5 menit tanpa join
- ChirpStack console: no join request received

**Penyebab & Solusi:**

```
ISSUE A: Wrong LMIC version atau LMIC_USE_INTERRUPTS = 1

  Penyebab:
  - IBM LMIC vs MCCI LMIC (berbeda!)
  - LMIC_USE_INTERRUPTS = 1 tidak compatible dengan ESP32 Arduino Core 3.x
  
  Solusi:
  1. Hapus library: Arduino IDE > Sketch > Include Library > Manage Libraries
     Search "MCCI LoRaWAN LMIC" > Click > Uninstall
  
  2. Install versi benar:
     Arduino IDE > Tools > Manage Libraries
     Search: "MCCI LoRaWAN LMIC Library"
     Version: 6.0.1 (NOT latest)
     Click Install
  
  3. Edit LMIC_BeginEnd.h:
     File > Preferences > Show sketchbook location
     Open: libraries/MCCI_LoRaWAN_LMIC_Arduino-6.0.1/src/lmic/lmic_BeginEnd.h
     Cari: #define LMIC_USE_INTERRUPTS
     Ubah ke: #define LMIC_USE_INTERRUPTS 0
     Save

ISSUE B: DIO0 pin tidak interrupt-capable

  Penyebab:
  - GPIO34 (input-only) dipilih untuk DIO0
  - GPIO12 (strapping pin) conflict
  
  Solusi:
  - Gunakan GPIO4 (recommended)
  - Atau GPIO2, GPIO15, GPIO25, GPIO26, GPIO27, GPIO32, GPIO33 (tested safe)
  - Edit kode:
    #define PIN_LORA_DIO0  4  // Change dari 34 ke 4

ISSUE C: Clock error terlalu besar

  Penyebab:
  - Clock crystal tidak akurat
  - Baud rate serial monitor tidak match
  
  Solusi:
  - Di Arduino code, tambah:
    LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
    // Toleransi 1% dari MAX_CLOCK_ERROR (default terlalu ketat)

ISSUE D: Device credentials salah

  Penyebab:
  - Copy-paste DevEUI, AppEUI, AppKey salah
  - Byte order salah (MSB vs LSB)
  
  Solusi:
  - Verifikasi credentials di ChirpStack console (copy-paste dari sana)
  - DevEUI di code harus reverse byte order dari console
    Console: 845bb9552397d550
    Code PROGMEM: {0x50, 0xd5, 0x97, 0x23, 0x55, 0xb9, 0x5b, 0x84}
    (dibalik urutan byte-nya)
  - Test dengan simple echo program:
    void setup() {
      Serial.begin(115200);
      Serial.println("Device credentials:");
      for (int i=0; i<8; i++) {
        Serial.print(DEVEUI[i], HEX);
        Serial.print(" ");
      }
    }
```

### 3. Brownout atau Watchdog Reset

**Gejala:**
- ESP32 restart otomatis setiap beberapa detik
- Serial monitor: "Guru Meditation Error" atau "WDT reset"
- Terutama terjadi saat LoRa TX

**Penyebab:**
- LoRa module power spike tidak ter-supply dengan baik
- Kapasitor decoupling tidak cukup
- Power supply USB tidak stabil (<2A)

**Solusi:**

```bash
# Hardware fix (MOST IMPORTANT):
# 1. Tambah kapasitor 100µF pada 3.3V rail dekat LoRa module
#    Solder directly ke VCC-GND pins
# 2. Gunakan USB power 2A minimum (bukan 500mA)
# 3. Check solder joint jangan cold solder

# Software workaround (temporary):
void setup() {
  // Disable brownout detection (risky, temporary only)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Atau adjust WDT timeout
  ESP.wdtEnable(10000);  // 10 detik
}

# Diagnostic command:
# Serial.println(ESP.getChipId());
# Serial.println(ESP.getFreeHeap());
```

### 4. PZEM Sensor Tidak Respond

**Gejala:**
- Serial monitor: "PZEM voltage: nan"
- Atau sensor tidak communicate
- Tapi wiring sudah correct

**Penyebab umum:**

```
PENYEBAB A: Address mismatch

  Simbol: PZEM respond tapi address bukan 0x01/0x02/0x03
  
  Solusi:
  1. Identify physical address di sensor (lihat label/switch)
  2. Jika beda dari kode, reset ke broadcast terlebih dahulu:
     
     PZEM004Tv30 pzem_reset(Serial2, 0xF8);  // 0xF8 = broadcast
     pzem_reset.setAddress(0x01);  // Ubah ke address yang dikehendaki
  
  3. Test individual:
     PZEM004Tv30 pzem1(Serial2, 0x01);
     float v = pzem1.voltage();
     Serial.println(v);  // Should print 220-250, not NaN

PENYEBAB B: UART pin TX/RX terbalik

  Simbol: Upload OK tapi sensor tidak respond
  
  Solusi:
  1. Swap RX dan TX di code:
     // BEFORE:
     PZEM004Tv30 pzem1(Serial2, 0x01);  // GPIO16=RX, GPIO17=TX
     
     // AFTER:
     PZEM004Tv30 pzem1(Serial2, 0x01);  // Gunakan Serial1 instead
     // Atau swap pin assignment di board config
  
  2. Atau cek konfigurasi hardware benar

PENYEBAB C: Baud rate mismatch

  Simbol: Sensor tidak respond atau respond garbage
  
  Solusi:
  1. Confirm baud rate di sensor (default 9600)
  2. Di Arduino code:
     Serial2.begin(9600, SERIAL_8E1, 16, 17);
     // 9600 baud, 8 data bit, Even parity, 1 stop bit
  
  3. Jika sensor punya AT command menu:
     # Via serial terminal (PuTTY):
     AT+BAUD=1  # Set 9600
     AT+BAUD=2  # Set 19200
     # Test: AT+ADDR=01 (return: +ADDR=01)

PENYEBAB D: Bus contention (3 sensor di 1 UART)

  Simbol: 1-2 sensor OK, 1 sensor tidak respond / lag
  
  Solusi:
  - Gunakan split UART (MANDATORY untuk 3 sensor):
    Serial2 (GPIO16/17): PZEM-001 (0x01) + PZEM-002 (0x02)
    Serial1 (GPIO21/22): PZEM-003 (0x03)
  
  - Jangan put ketiga sensor di satu UART

PENYEBAB E: Sensor power off atau under-power

  Simbol: Sensor tidak respond, atau intermittent
  
  Solusi:
  1. Multimeter: Check 5V dan GND pada sensor
  2. Power dari 3.3V rail ESP32 (bukan 5V) - stable dalam kasus ini
  3. Pastikan GND common dengan ESP32
  4. Jika power dari external supply, pastikan:
     - Supply rated 5V/2A minimum
     - GND connected to ESP32 GND
```

## LoRaWAN & ChirpStack Issues

### 5. Gateway Tidak Receive Join Request

**Gejala:**
- ChirpStack console: no devices registered
- Atau registered tapi no uplinks received
- Gateway online (status active di console)

**Penyebab & Solusi:**

```
ISSUE A: Gateway tidak terdaftar di ChirpStack

  Solusi:
  1. Cek gateway EUI dari WM1302:
     # WM1302 EUI dapat dari:
     - Label fisik di module
     - Atau generate dari MAC address Raspberry Pi
     
  2. Register gateway di ChirpStack:
     Gateways menu > Create gateway
     - Gateway ID: (device identifier)
     - Gateway EUI: (dari WM1302 label)
     - Name: (custom name)
     - Location: (GPS coordinates)
  
  3. Verify koneksi ChirpStack-Gateway:
     sudo journalctl -u chirpstack-gateway-bridge -f
     # Look for: "gateway authenticated" message

ISSUE B: ChirpStack tidak listening

  Simbol: Gateway register OK tapi tidak send uplink
  
  Solusi:
  1. Cek service status:
     sudo systemctl status chirpstack-sqlite
     sudo systemctl status chirpstack-gateway-bridge
  
  2. Check API respond:
     curl http://localhost:8080/api/version
     # Should return JSON dengan version
  
  3. Lihat logs:
     sudo journalctl -u chirpstack-sqlite -f
     sudo journalctl -u chirpstack-gateway-bridge -f
  
  4. Database issue? Reset:
     sudo systemctl stop chirpstack-sqlite
     sudo rm /var/lib/chirpstack/chirpstack.sqlite*
     sudo systemctl start chirpstack-sqlite
     # (Will recreate empty database)

ISSUE C: RX2 frequency salah (AS923-3 trap!)

  Simbol: Join request sent, server receives, tapi device tidak receive join accept
  
  Penyebab:
  - Assume RX2 = 916.6 MHz (salah untuk AS923-3!)
  - AS923-3 RX2 harus = 923.2 MHz
  
  Solusi:
  1. Verify LMIC config untuk AS923-3:
     # Di LMIC library: lmic_util.c atau region config
     AS923_RX2_FREQ = 923200000  // Must be 923.2 MHz
  
  2. Check gateway config:
     /etc/lora/global_conf.json:
     "radio_1": {
       "freq": 923200000  // RX2 frequency
     }
  
  3. Verify ChirpStack region setting:
     UI > Gateways > Edit > Region: AS923-3

ISSUE D: Packet forwarder tidak running

  Simbol: Gateway online tapi not receiving packets
  
  Solusi:
  1. Check lora_pkt_fwd service:
     sudo systemctl status lora-packet-forwarder
     # Mungkin tidak ter-setup sebagai service
  
  2. Manual start dan monitor:
     sudo /usr/local/bin/lora_pkt_fwd -c /etc/lora/
     # See output untuk error
  
  3. Common error: "unable to open SPI device"
     - SPI not enabled: sudo raspi-config > Interface > SPI
     - Device file missing: ls /dev/spidev* (should show 0.0)
  
  4. Global config JSON error:
     jq . /etc/lora/global_conf.json
     # If invalid: error message akan help
     # Check tidak ada comment dalam JSON
```

### 6. Downlink / RX Tidak Work

**Gejala:**
- Uplink OK (data terkirim ke ChirpStack)
- Tapi downlink dari server tidak received di device
- Atau downlink pending tapi tidak deliver

**Penyebab:**

```
1. RX window tidak active:
   - Uplink sent tapi device tidak wait untuk RX window
   - Solusi: Ensure LMIC loop berjalan di main loop
   
   void loop() {
     os_runloop_once();  // PENTING: keep this active
   }

2. RX window terlalu pendek:
   - Default RX timeout 5 second
   - Jika network delay > 5s, miss window
   - Solusi: Extend RX timeout (LMIC config)

3. Gateway tidak send downlink:
   - ChirpStack menerima downlink request tapi gateway tidak relay
   - Check logs: journalctl -u chirpstack-gateway-bridge
   - Verify radio_1 (RX2) enable dan tx_enable=true

4. Frequency conflict:
   - RX2 frequency tidak match antara device dan gateway
   - Solution: AS923-3 RX2 = 923.2 MHz (bukan 916.6)
```

## Node-RED & Data Pipeline Issues

### 7. MQTT Topic Tidak Receive Data

**Gejala:**
- ChirpStack mengirim uplink (visible di console)
- Tapi Node-RED MQTT in node tidak trigger
- Atau trigger tapi data kosong

**Penyebab:**

```
ISSUE A: Topic subscription salah

  Simbol: MQTT in node tidak receive
  
  Solusi:
  1. Verify ChirpStack publish ke MQTT:
     Subscribe dari command line:
     mosquitto_sub -h localhost -u chirpstack -P chirpstack -t "as/v3/#" -v
     
     Kirim uplink dari device.
     Seharusnya topic muncul di terminal.
  
  2. Jika tidak terlihat:
     - ChirpStack config salah (MQTT tidak enabled)
     - Mosquitto service down
     - Auth credentials wrong
  
  3. Node-RED topic harus exact match:
     ChirpStack publish: as/v3/applications/1/devices/device-1/up
     Node-RED subscribe: as/v3/applications/+/devices/+/up (dengan wildcard)

ISSUE B: ChirpStack tidak publish

  Simbol: Uplink diterima ChirpStack (console shows), tapi tidak ke MQTT
  
  Solusi:
  1. Check ChirpStack config file:
     /etc/chirpstack/chirpstack.toml:
     [integration]
     backend="mqtt"
     [integration.mqtt]
     broker="tcp://localhost:1883"
     username="chirpstack"
     password="chirpstack"
  
  2. Restart service:
     sudo systemctl restart chirpstack-sqlite
  
  3. Monitor logs:
     journalctl -u chirpstack-sqlite -f
     # Look untuk: "connecting to mqtt broker"

ISSUE C: Authentication failed

  Simbol: MQTT in node status: "Authentication denied"
  
  Solusi:
  1. Verify Mosquitto password:
     sudo mosquitto_passwd /etc/mosquitto/passwd
     # Reenter password untuk user chirpstack
  
  2. Restart Mosquitto:
     sudo systemctl restart mosquitto
  
  3. Test manual:
     mosquitto_pub -h localhost -u chirpstack -P chirpstack -t test/msg -m "hello"
     # Should return: Connection refused (jika auth fail lebih jelas)
```

### 8. Payload Decoder Tidak Work

**Gejala:**
- Data terima di Node-RED tetapi tidak dekode
- Atau decode tapi value salah (NaN, 0, atau garbage)

**Solusi:**

```javascript
// Test decoder function
let payload = Buffer.from('...base64...', 'base64');

// Debug: print raw bytes
console.log("Raw payload:", payload);
console.log("Length:", payload.length);

// Verify byte order dan parsing
let v_r = ((payload[0] << 8) | payload[1]) / 10;
console.log("Voltage R:", v_r);

// Common mistake: forget division factor
// Payload encode: voltage * 10
// Decode: divide by 10
// Jika divide forget: akan muncul 2400 instead of 240
```

## InfluxDB & Grafana Issues

### 9. Data Tidak Tertulis ke InfluxDB

**Gejala:**
- Node-RED InfluxDB out node tidak error
- Tapi query di Grafana returns empty
- Atau Grafana dashboard kosong

**Penyebab:**

```
ISSUE A: InfluxDB service not running

  Solusi:
  sudo systemctl status influxdb
  sudo systemctl start influxdb
  
  Verify:
  curl http://localhost:8086/query?q=SHOW+DATABASES

ISSUE B: Database atau measurement salah

  Solusi:
  1. Verify database exist:
     influx -execute 'SHOW DATABASES'
     # pzem_data harus di list
  
  2. Create jika tidak ada:
     influx -execute 'CREATE DATABASE pzem_data'
  
  3. Node-RED config:
     Database: pzem_data (exactly match)
     Measurement: pzem_3phase

ISSUE C: InfluxDB tidak menerima write dari Node-RED

  Simbol: InfluxDB running tapi metric kosong
  
  Solusi:
  1. Test write manual:
     curl -i -XPOST http://localhost:8086/write?db=pzem_data \
     --data-binary 'pzem_3phase,phase=R voltage=240.5,current=10.2'
     # Response 204 = success
  
  2. Node-RED node config:
     - Connection: http://localhost:8086
     - Database: pzem_data
     - Measurement: pzem_3phase (di measurement field)
  
  3. Payload format Node-RED:
     msg.payload = {
       voltage: 240.5,
       current: 10.2,
       power: 2400,
       phase: "R"
     }
     msg.measurement = "pzem_3phase";

ISSUE D: Retention policy

  Simbol: Data tertulis tapi hilang setelah waktu tertentu
  
  Solusi:
  1. Check retention policy:
     influx -database pzem_data -execute 'SHOW RETENTION POLICIES'
  
  2. Default retention: 30 hari (infinite)
     Jika perlu simpan lebih lama:
     influx -database pzem_data -execute 'ALTER RETENTION POLICY "autogen" ON "pzem_data" DURATION 365d'
```

### 10. Grafana Dashboard Tidak Tampil Data

**Gejala:**
- Grafana loaded, dashboard visible
- Tapi panel shows "No data" atau "Loading..."
- Atau error message

**Penyebab:**

```
ISSUE A: Data source not connected

  Simbol: Dashboard error: "PluginId=graphite, HTTPError: 500"
  
  Solusi:
  1. Configuration > Data Sources > pzem_influx
  2. Click "Save & Test"
     - "Data source is working" = OK
     - Error = konfigurasi salah
  
  2. Verify URL: http://localhost:8086 (not 127.0.0.1)
  3. Verify database: pzem_data

ISSUE B: Metric name salah di dashboard

  Simbol: Panel query returns empty
  
  Solusi:
  1. Verify measurement exist:
     influx -database pzem_data -execute 'SHOW MEASUREMENTS'
  
  2. Di Grafana panel:
     FROM pzem_3phase (exactly match)
     SELECT voltage, current, etc (field names)

ISSUE C: Time range tidak cover data

  Simbol: Dashboard shows data tapi time picker tidak cover
  
  Solusi:
  1. Time picker (top right): select "Last 24 hours" atau "Last 7 days"
  2. Atau set custom time: "now-24h to now"

ISSUE D: Query syntax error

  Simbol: Panel shows red error
  
  Solusi:
  1. Edit panel > Metrics tab
  2. Verify query:
     FROM pzem_3phase
     WHERE time > now() - 1h
     GROUP BY time(1m)
  
  3. Test query di InfluxDB CLI:
     influx -database pzem_data -execute 'SELECT voltage FROM pzem_3phase LIMIT 10'
```

## Network & Communication Issues

### 11. Gateway Cannot Reach Internet (untuk cloud ChirpStack)

**Gejala:**
- Loopback test OK (node communicate dengan local gateway)
- Tapi cloud ChirpStack tidak terkoneksi

**Solusi:**

```bash
# Test connectivity
ping 8.8.8.8
# No response: network unreachable

# Check routing
ip route show
# Verify default route exist

# Check DNS
nslookup chirpstack.io
# Should resolve to IP

# If behind proxy
# Set http_proxy dan https_proxy environment variable
export http_proxy=http://proxy.example.com:8080
export https_proxy=http://proxy.example.com:8080
```

### 12. Latency Tinggi (>10 detik)

**Gejala:**
- Data terkirim ke ChirpStack tapi delay besar
- Notification Telegram lambat > 2 menit

**Penyebab & Solusi:**

```
1. ChirpStack processing slow
   - CPU usage tinggi di Pi
   - Solution: top command, identify bottleneck
   
   Optimize:
   - Reduce log level "debug" → "info"
   - Check database tidak corrupt (SQLite optimize)

2. MQTT broker slow
   - Mosquitto backlog penuh
   - Solution: mosquitto_sub test response time
   
   Optimize:
   - Increase max_connections di mosquitto.conf
   - Increase max_queued_messages

3. InfluxDB write slow
   - Database write queue penuh
   - Solution: influx show database stats
   
   Optimize:
   - Increase write buffer size
   - Batch write di Node-RED

4. Node-RED processing slow
   - Function node terlalu kompleks
   - Solution: Add timestamp nodes untuk measure delay
   
   Optimize:
   - Split complex flow ke multiple streams
   - Use async pattern
```

## General System Issues

### 13. Raspberry Pi Reboot Loop

**Gejala:**
- Pi reboot setiap beberapa detik
- Tidak sempat load OS

**Penyebab:**
- Power supply insufficient
- SD card corrupt
- Service crashing on startup

**Solusi:**

```bash
# Boot dengan minimal services
1. SSH ke Pi dengan USB-TTL adapter (jika available)
   # Or use Pi Console

2. Check messages:
   dmesg | tail -50
   # Look untuk: out of memory, kernel panic

3. Disable problematic service:
   sudo systemctl disable nodered
   sudo systemctl disable grafana-server
   sudo reboot

4. Check disk space:
   df -h
   # If root 100%: rm old logs
   sudo rm -rf /var/log/*.1 /var/log/*.2.gz

5. Verify power supply:
   vcgencmd get_throttled
   # 0 = OK, non-zero = throttled
```

### 14. Out of Disk Space

**Gejala:**
- Services stop unexpectedly
- apt install fails
- Disk 100% usage

**Solusi:**

```bash
# Find large files
du -sh /* | sort -rh | head -10

# Common culprits:
# /var/log/ - log files
sudo rm -rf /var/log/*.1 /var/log/*.gz

# /tmp/ - temporary
sudo rm -rf /tmp/*

# /home/ - old backups
ls -lah ~/

# Docker/container images (if using)
docker system prune -a

# Check after cleaning:
df -h

# Disk cleanup continued:
sudo apt clean
sudo apt autoclean
```

### 15. Memory Leak (Gradual Slowdown)

**Gejala:**
- Pi fast on startup
- Gradual slowdown over days/weeks
- High memory usage (free -h shows low "available")

**Penyebab:**
- Node-RED leak
- InfluxDB leak
- Mosquitto queue buildup

**Solusi:**

```bash
# Monitor memory
watch -n 1 free -h
top -b -n 1 | head -20

# Identify process
ps aux --sort=-%mem | head -10

# Restart leaking service
sudo systemctl restart nodered
sudo systemctl restart influxdb
sudo systemctl restart mosquitto

# Automate restart (via cron)
# 0 2 * * * sudo systemctl restart nodered >/dev/null 2>&1
# (Restart Node-RED at 2 AM daily)
```

## Performance Optimization

### 16. Meningkatkan Uplink Success Rate

**Target: PDR >90%**

```
1. Antenna placement
   - Height: Minimal 2m di atas atap
   - Direction: Omnidirectional LoRa antenna (5-8 dBi)
   - Distance from metal/congestion

2. TX power optimization
   - Set optimal TX power di ChirpStack
   - Not too high (cause interference)
   - Not too low (fail to reach gateway)
   - Typical: +17 to +20 dBm

3. SF (Spreading Factor) tuning
   - Longer range: SF7-SF12
   - Shorter range: SF7 (fastest)
   - ADR (Adaptive Data Rate): let ChirpStack decide

4. Network load
   - Reduce uplink frequency jika perlu
   - Not every second - use reasonable interval
   - For power monitoring: 1-5 minute interval OK
```

## Verification Checklist

Sebelum production deploy:

- [ ] ESP32 firmware upload successful
- [ ] PZEM sensors all three respond individually
- [ ] LoRaWAN join successful (ChirpStack shows device)
- [ ] Uplink received di ChirpStack console
- [ ] Payload decoded correctly
- [ ] Data written to InfluxDB (query returns data)
- [ ] Grafana dashboard shows data (not empty)
- [ ] Alarm threshold triggers Telegram notification
- [ ] System stable 24+ jam continuous operation
- [ ] Uptime >95%
- [ ] Latency <5 detik average
- [ ] Network PDR >85% (test di target location)

## Getting Help

Jika issue tidak terselesaikan:

1. Collect diagnostic data:
   ```bash
   # ChirpStack logs
   journalctl -u chirpstack-sqlite -n 100 > logs_chirpstack.txt
   
   # Gateway bridge logs
   journalctl -u chirpstack-gateway-bridge -n 100 > logs_gateway.txt
   
   # Mosquitto logs
   tail -n 100 /var/log/mosquitto/mosquitto.log > logs_mosquitto.txt
   
   # System info
   uname -a > system_info.txt
   df -h >> system_info.txt
   free -h >> system_info.txt
   ```

2. Create issue di GitHub dengan:
   - Step-by-step untuk reproduce
   - Logs collected di atas
   - Hardware config (Pin assignments, power supply)
   - Expected vs actual behavior

3. Konsultasi dengan:
   - ChirpStack community forum
   - MCCI LMIC GitHub issues
   - Raspberry Pi forums

---

Troubleshooting guide lengkap. Semoga membantu!
