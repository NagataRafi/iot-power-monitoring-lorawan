# Panduan Konfigurasi Software Lengkap

Instruksi detail setup semua komponen software di Raspberry Pi gateway.

## Pre-requisites

```bash
# Update sistem
sudo apt update && sudo apt upgrade -y
sudo apt install -y git build-essential pkg-config libmosquitto-dev curl wget

# Check OS version
uname -a  # Harus Linux pi 5.x.x aarch64 (64-bit)
lsb_release -a  # Raspberry Pi OS Bookworm atau Bullseye
```

## 1. ChirpStack Installation

### Download & Install Service

```bash
# Add ChirpStack repository
sudo apt install -y curl gpg
curl https://repos.chirpstack.io/chirpstack-repo.gpg | sudo apt-key add -
echo "deb https://repos.chirpstack.io/chirpstack ubuntu focal main" | sudo tee /etc/apt/sources.list.d/chirpstack.list

# Update dan install
sudo apt update
sudo apt install -y chirpstack chirpstack-gateway-bridge

# Verify installation
which chirpstack
which chirpstack-gateway-bridge

# Check service name (CRITICAL)
systemctl list-unit-files | grep chirpstack
```

Expected output:
```
chirpstack-gateway-bridge.service       enabled
chirpstack-sqlite.service               enabled
```

PENTING: Service name adalah `chirpstack-sqlite` (bukan `chirpstack`)

### Konfigurasi ChirpStack

File: `/etc/chirpstack/chirpstack.toml`

```bash
sudo nano /etc/chirpstack/chirpstack.toml
```

Content:

```toml
[general]
log_level="info"
log_format="json"

[database]
# SQLite database path
dsn="sqlite:////var/lib/chirpstack/chirpstack.sqlite"

[api]
# API server bind address
bind="0.0.0.0:8080"
# API authentication secret (minimum 16 characters)
secret="your-super-secret-key-min16char"

[regions]
# Daftar region yang aktif (AS923 untuk Indonesia)
[regions.as923_1]
enabled=true
name="AS923-1"

[regions.as923_3]
enabled=true
name="AS923-3"

[integration]
# Integration backend type
backend="mqtt"

[integration.mqtt]
# MQTT broker address
broker="tcp://localhost:1883"
# Client ID
client_id="chirpstack"
# Authentication
auth_type="username_password"
username="chirpstack"
password="chirpstack"
# Topic routing
topic_prefix="as/v3/"

[monitoring]
# Monitoring backend
backend="prometheus"
bind="0.0.0.0:8081"
```

### Konfigurasi ChirpStack Gateway Bridge

File: `/etc/chirpstack-gateway-bridge/chirpstack-gateway-bridge.toml`

```bash
sudo nano /etc/chirpstack-gateway-bridge/chirpstack-gateway-bridge.toml
```

Content:

```toml
[general]
log_level="info"
log_format="json"

[backend]
# Backend type untuk WM1302 SPI
type="semtech"
bind="0.0.0.0:3001"

[backend.semtech]
# Tidak ada konfigurasi tambahan untuk SPI

[integration]
backend="mqtt"

[integration.mqtt]
# MQTT broker address
broker="tcp://localhost:1883"
# MQTT client ID
client_id="chirpstack-gateway-bridge"
# Authentication
auth_type="username_password"
username="chirpstack"
password="chirpstack"
# Topic prefix
topic_prefix="as/v3/"

[monitoring]
backend="prometheus"
bind="0.0.0.0:8082"
```

### Enable & Start Services

```bash
sudo systemctl daemon-reload
sudo systemctl enable chirpstack-sqlite
sudo systemctl enable chirpstack-gateway-bridge
sudo systemctl start chirpstack-sqlite
sudo systemctl start chirpstack-gateway-bridge

# Verify status
sudo systemctl status chirpstack-sqlite
sudo systemctl status chirpstack-gateway-bridge

# Check logs
journalctl -u chirpstack-sqlite -f
```

### Access ChirpStack Console

URL: `http://192.168.100.55:8080`

Login credentials (default):
- Username: admin
- Password: admin

HARUS GANTI password pada first login.

## 2. Semtech Packet Forwarder (WM1302)

### Clone & Build sx1302_hal

```bash
cd ~/
git clone https://github.com/Lora-net/sx1302_hal.git
cd sx1302_hal
git checkout v2.1.0

# Build
make clean
make

# Copy binary
sudo cp lora_pkt_fwd/lora_pkt_fwd /usr/local/bin/
sudo chmod +x /usr/local/bin/lora_pkt_fwd
```

### Konfigurasi Global Config

File: `/etc/lora/global_conf.json`

```bash
sudo mkdir -p /etc/lora
sudo nano /etc/lora/global_conf.json
```

Content (PASTIKAN JSON valid, NO comments):

```json
{
  "SX130x_conf": {
    "spidev": "/dev/spidev0.0",
    "lorawan_public": true,
    "clksrc": 0,
    "antenna_gain": 0,
    "fine_timestamp": {
      "enable": false
    },
    "sx1261_conf": {
      "spi_path": "/dev/spidev0.1",
      "lorawan_public": true,
      "single_input_mode": false,
      "rssi_offset": 0,
      "tx_enable": false
    }
  },
  "radio_0": {
    "enable": true,
    "type": "SX1302",
    "freq": 923400000,
    "rssi_offset": -215,
    "rssi_tcomp": {
      "coeff_a": 0,
      "coeff_b": 0,
      "coeff_c": 20.41,
      "coeff_d": 2162.56,
      "freq_ref": 868000000
    },
    "tx_enable": true,
    "tx_freq_min": 923200000,
    "tx_freq_max": 923600000,
    "tx_gain_table": [
      {"rf_power": 12, "pa_gain": 0, "pwr_idx": 15}
    ]
  },
  "radio_1": {
    "enable": true,
    "type": "SX1250",
    "freq": 923200000,
    "rssi_offset": -215,
    "tx_enable": false,
    "single_input_mode": true
  },
  "chan_FSK": {
    "enable": false
  },
  "chan_Lora_std": {
    "enable": true,
    "rf_chain": 1,
    "if": 0,
    "bandwidth": 250000,
    "datarate": 7,
    "implicit": false
  },
  "tx_lut": [
    {"rf_power": -6, "pa_gain": 0, "pwr_idx": 0},
    {"rf_power": -3, "pa_gain": 0, "pwr_idx": 1},
    {"rf_power": 0, "pa_gain": 0, "pwr_idx": 2},
    {"rf_power": 3, "pa_gain": 1, "pwr_idx": 3},
    {"rf_power": 6, "pa_gain": 1, "pwr_idx": 4},
    {"rf_power": 10, "pa_gain": 1, "pwr_idx": 5},
    {"rf_power": 11, "pa_gain": 1, "pwr_idx": 6},
    {"rf_power": 12, "pa_gain": 1, "pwr_idx": 7},
    {"rf_power": 13, "pa_gain": 2, "pwr_idx": 9},
    {"rf_power": 14, "pa_gain": 2, "pwr_idx": 10},
    {"rf_power": 16, "pa_gain": 2, "pwr_idx": 11},
    {"rf_power": 20, "pa_gain": 3, "pwr_idx": 15}
  ]
}
```

PENTING: 
- Tidak boleh ada comment (`//` atau `/* */`)
- JSON harus valid (test dengan `jq` sebelum deploy)

Test JSON:

```bash
jq . /etc/lora/global_conf.json
# Jika valid: output struktur JSON
# Jika invalid: error message jelas
```

### Startup Script (Manual Test)

```bash
# Test manual (untuk debug)
sudo /usr/local/bin/lora_pkt_fwd -c /etc/lora/

# Expected output:
# [2026-01-15 10:20:30] Gateway concentrator started, "radio_0" configured at SX1302
# [2026-01-15 10:20:31] Gateway ready
```

Jika error "ERROR: unable to open SPI device", berarti SPI tidak enabled atau pin salah.

## 3. Mosquitto MQTT Broker

### Install

```bash
sudo apt install -y mosquitto mosquitto-clients

# Verify
which mosquitto
```

### Konfigurasi

File: `/etc/mosquitto/mosquitto.conf`

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Add/uncomment:

```conf
# Allow connections on all interfaces
listener 1883 0.0.0.0

# Enable authentication
allow_anonymous false
password_file /etc/mosquitto/passwd

# Message persistence
persistence true
persistence_location /var/lib/mosquitto/

# Logging
log_dest file /var/log/mosquitto/mosquitto.log
log_dest stderr
log_type all
```

### Create MQTT User

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd chirpstack
# Prompt password: chirpstack

sudo chown mosquitto:mosquitto /etc/mosquitto/passwd
sudo chmod 600 /etc/mosquitto/passwd
```

### Start Service

```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
sudo systemctl status mosquitto
```

### Test MQTT Connection

Terminal 1 (Subscribe):
```bash
mosquitto_sub -h localhost -u chirpstack -P chirpstack -t "as/v3/#" -v
```

Terminal 2 (Publish):
```bash
mosquitto_pub -h localhost -u chirpstack -P chirpstack -t "test/message" -m "Hello"
```

Terminal 1 should receive message.

## 4. InfluxDB 1.6.7

### Download & Install (ARM v7)

```bash
# Download
cd ~/Downloads
wget https://dl.influxdata.com/influxdb/releases/influxdb-1.6.7_linux_armv7.tar.gz

# Extract
tar xvfz influxdb-1.6.7_linux_armv7.tar.gz

# Copy binaries
sudo cp influxdb-1.6.7-1/usr/bin/influxd /usr/local/bin/
sudo cp influxdb-1.6.7-1/usr/bin/influx /usr/local/bin/

# Create data directory
sudo mkdir -p /var/lib/influxdb
sudo mkdir -p /var/log/influxdb
sudo chown -R influxdb:influxdb /var/lib/influxdb /var/log/influxdb
```

### Konfigurasi

File: `/etc/influxdb/influxdb.conf`

```bash
sudo nano /etc/influxdb/influxdb.conf
```

Key settings:

```conf
[meta]
  dir = "/var/lib/influxdb/meta"

[data]
  dir = "/var/lib/influxdb/data"
  wal-dir = "/var/lib/influxdb/wal"

[http]
  enabled = true
  bind-address = ":8086"

[logging]
  level = "info"
```

### Systemd Service

```bash
sudo tee /etc/systemd/system/influxdb.service > /dev/null <<EOF
[Unit]
Description=InfluxDB
After=network.target

[Service]
Type=simple
User=influxdb
Group=influxdb
ExecStart=/usr/local/bin/influxd -config /etc/influxdb/influxdb.conf
Restart=on-failure
RestartSec=10s

[Install]
WantedBy=multi-user.target
EOF

sudo usermod -aG influxdb influxdb 2>/dev/null || true
sudo systemctl daemon-reload
sudo systemctl enable influxdb
sudo systemctl start influxdb
```

### Initialize Database

```bash
# Wait untuk service jalan
sleep 5

# Create database
influx -execute 'CREATE DATABASE pzem_data'

# Verify
influx -execute 'SHOW DATABASES'
# Output: name: databases, values: pzem_data
```

### Test Data Write

```bash
influx -database pzem_data -execute 'INSERT pzem_3phase,phase=R voltage=240.5,current=10.2,power=2400.0'

# Query
influx -database pzem_data -execute 'SELECT * FROM pzem_3phase'
```

## 5. Grafana

### Install

```bash
# Download ARM v7
cd ~/Downloads
wget https://dl.grafana.com/oss/release/grafana_13.0.1_armv7.deb

# Install
sudo dpkg -i grafana_13.0.1_armv7.deb

# Alternatively, menggunakan apt (jika supported)
# sudo apt install -y grafana
```

### Enable & Start

```bash
sudo systemctl daemon-reload
sudo systemctl enable grafana-server
sudo systemctl start grafana-server

# Verify
sudo systemctl status grafana-server
```

### Initial Setup

1. Open browser: `http://192.168.100.55:3000`
2. Login: admin / admin
3. Change password (mandatory)

### Add InfluxDB Data Source

1. Configuration > Data Sources > Add data source
2. Type: InfluxDB
3. Settings:
   - Name: pzem_influx
   - URL: http://localhost:8086
   - Database: pzem_data
   - User: (kosong)
   - Password: (kosong)
4. Save & Test

### Import Dashboard

Dashboard JSON tersedia di repository. Import via:
1. Grafana > Create > Import
2. Upload JSON atau paste JSON
3. Select data source: pzem_influx
4. Import

## 6. Node-RED

### Install

```bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/install.sh)
```

### Enable & Start

```bash
sudo systemctl enable nodered
sudo systemctl start nodered

# Check logs
sudo journalctl -u nodered -f
```

### Access Editor

URL: `http://192.168.100.55:1880`

### Basic Flow Configuration

#### A. MQTT Input (ChirpStack uplink)

Node type: MQTT in
- Server: localhost:1883
- Topic: as/v3/+/devices/+/up
- Username: chirpstack
- Password: chirpstack

#### B. Payload Decoder

Node type: Function

```javascript
// Decode 26-byte payload dari ESP32
let payload = msg.payload.data;

// Convert base64 ke array jika perlu
if (typeof payload === 'string') {
  payload = Buffer.from(payload, 'base64');
}

// Parse big-endian format
let data = {
  voltage_r: ((payload[0] << 8) | payload[1]) / 10,
  voltage_s: ((payload[2] << 8) | payload[3]) / 10,
  voltage_t: ((payload[4] << 8) | payload[5]) / 10,
  current_r: ((payload[6] << 8) | payload[7]) / 100,
  current_s: ((payload[8] << 8) | payload[9]) / 100,
  current_t: ((payload[10] << 8) | payload[11]) / 100,
  power_r: ((payload[12] << 24) | (payload[13] << 16) | (payload[14] << 8) | payload[15]),
  power_s: ((payload[16] << 24) | (payload[17] << 16) | (payload[18] << 8) | payload[19]),
  power_t: ((payload[20] << 24) | (payload[21] << 16) | (payload[22] << 8) | payload[23]),
  frequency: payload[24] / 10,
  power_factor: payload[25] / 100
};

msg.payload = data;
return msg;
```

#### C. InfluxDB Output

Node type: InfluxDB out
- Server: localhost:8086
- Database: pzem_data
- Measurement: pzem_3phase
- Measurement name: pzem_3phase
- Tag keys: phase (R, S, T)
- Field keys: voltage, current, power, frequency, power_factor

#### D. Telegram Notification

Node type: Telegram bot
- Token: [Telegram Bot API Token]
- Chat ID: [Your Chat ID]

Trigger pada threshold alert.

### Export/Import Flow

Save flow sebagai JSON untuk backup:

```bash
# Export dari Node-RED dashboard
Admin menu > Export > Download
# Atau direct file
cp ~/.node-red/flows.json ~/flows_backup.json
```

## 7. Systemd Startup Script (Optional Automation)

File: `/home/pi/startup_gateway.sh`

```bash
#!/bin/bash
# Startup script untuk semua services

echo "Starting LoRaWAN Gateway services..."

# ChirpStack
echo "Starting ChirpStack..."
sudo systemctl start chirpstack-sqlite
sudo systemctl start chirpstack-gateway-bridge
sleep 3

# MQTT
echo "Starting Mosquitto..."
sudo systemctl start mosquitto
sleep 2

# InfluxDB
echo "Starting InfluxDB..."
sudo systemctl start influxdb
sleep 3

# Grafana
echo "Starting Grafana..."
sudo systemctl start grafana-server
sleep 3

# Node-RED
echo "Starting Node-RED..."
sudo systemctl start nodered
sleep 3

echo "All services started. Waiting for initialization..."
sleep 5

echo "Checking service status..."
for service in chirpstack-sqlite mosquitto influxdb grafana-server nodered
do
  if sudo systemctl is-active --quiet $service; then
    echo "✓ $service running"
  else
    echo "✗ $service FAILED - check logs"
    sudo systemctl status $service
  fi
done

echo "Gateway ready!"
echo ""
echo "Access points:"
echo "  ChirpStack: http://192.168.100.55:8080"
echo "  Grafana: http://192.168.100.55:3000"
echo "  Node-RED: http://192.168.100.55:1880"
```

Make executable:

```bash
chmod +x ~/startup_gateway.sh

# Run manually
~/startup_gateway.sh

# Or add to crontab untuk autostart on reboot
@reboot ~/startup_gateway.sh
```

## Verifikasi Instalasi Lengkap

```bash
#!/bin/bash
# Verification script

echo "=== ChirpStack ==="
curl -s http://localhost:8080/api/version || echo "ChirpStack not responding"

echo -e "\n=== MQTT ==="
mosquitto_sub -h localhost -u chirpstack -P chirpstack -t "test" -W 1 &>/dev/null && echo "MQTT OK" || echo "MQTT FAIL"

echo -e "\n=== InfluxDB ==="
curl -s http://localhost:8086/query?q=SHOW+DATABASES | grep -q pzem_data && echo "InfluxDB OK" || echo "InfluxDB FAIL"

echo -e "\n=== Grafana ==="
curl -s -u admin:admin http://localhost:3000/api/v1/org | grep -q "ok" && echo "Grafana OK" || echo "Grafana FAIL"

echo -e "\n=== Node-RED ==="
curl -s http://localhost:1880/ | grep -q "Node-RED" && echo "Node-RED OK" || echo "Node-RED FAIL"

echo -e "\n=== Service Status ==="
for service in chirpstack-sqlite chirpstack-gateway-bridge mosquitto influxdb grafana-server nodered
do
  status=$(sudo systemctl is-active $service 2>/dev/null)
  if [ "$status" = "active" ]; then
    echo "✓ $service: running"
  else
    echo "✗ $service: $status"
  fi
done
```

Save sebagai `~/verify_services.sh` dan run: `bash ~/verify_services.sh`

## Networking & Security (Optional)

### Firewall Configuration

```bash
# Allow hanya port yang diperlukan
sudo ufw default deny incoming
sudo ufw default allow outgoing

# ChirpStack
sudo ufw allow 8080/tcp

# Grafana
sudo ufw allow 3000/tcp

# Node-RED
sudo ufw allow 1880/tcp

# InfluxDB (localhost only)
sudo ufw allow from 127.0.0.1 to 127.0.0.1 port 8086/tcp

# MQTT (localhost only recommended)
sudo ufw allow from 127.0.0.1 to 127.0.0.1 port 1883/tcp

sudo ufw enable
```

### Network Configuration

File: `/etc/network/interfaces` atau `dhcpcd.conf`

For static IP:

```bash
sudo nano /etc/dhcpcd.conf
```

Add:

```conf
interface eth0
static ip_address=192.168.100.55/24
static routers=192.168.100.1
static domain_name_servers=8.8.8.8 1.1.1.1
```

Reboot:

```bash
sudo reboot
```

## Backup & Recovery

### Backup Data

```bash
# Backup InfluxDB
sudo influx backup /home/pi/influx_backup

# Backup ChirpStack database
sudo cp /var/lib/chirpstack/chirpstack.sqlite ~/chirpstack_backup.sqlite

# Backup configurations
tar czf ~/gateway_config_backup.tar.gz \
  /etc/chirpstack/ \
  /etc/chirpstack-gateway-bridge/ \
  /etc/mosquitto/ \
  /etc/influxdb/ \
  ~/.node-red/flows.json \
  /etc/lora/global_conf.json
```

### Restore

```bash
# Restore InfluxDB
sudo influx restore /home/pi/influx_backup

# Restore ChirpStack database
sudo cp ~/chirpstack_backup.sqlite /var/lib/chirpstack/chirpstack.sqlite

# Restore configurations
tar xzf ~/gateway_config_backup.tar.gz -C /
```

---

Panduan lengkap software setup selesai. Untuk troubleshooting detail, lihat file TROUBLESHOOTING.md
