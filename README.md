# MPU6050 Linux Kernel Driver

Linux kernel I2C driver for the MPU6050 IMU sensor on Raspberry Pi. The driver reads accelerometer and gyroscope data and exposes it through sysfs. A Python-based headless logger is included for real-time data capture.

## Features

- I2C-based kernel driver
- Accelerometer (X, Y, Z) and gyroscope data
- Fixed-point conversion (mg, dps)
- Thread-safe access using mutex
- Sysfs interface for user-space access

## Files

- `mpu6050.c` — Kernel module source code
- `Makefile` — Build configuration
- `imu_logger.py` — Python data logger (outputs CSV)

## Requirements

- Raspberry Pi 3B+
- MPU6050 sensor module (I2C address `0x68`)
- Raspberry Pi OS with kernel headers installed
- Python 3

Install dependencies:

```bash
sudo apt update
sudo apt install -y raspberrypi-kernel-headers build-essential i2c-tools
```

## Build

```bash
make clean
make
```

## Load Driver

```bash
sudo insmod mpu6050_driver.ko
```

## Attach Device

```bash
echo mpu6050 0x68 | sudo tee /sys/bus/i2c/devices/i2c-1/new_device
```

## Read Data

```bash
cat /sys/kernel/mpu6050/ax_mg
cat /sys/kernel/mpu6050/ay_mg
cat /sys/kernel/mpu6050/az_mg
cat /sys/kernel/mpu6050/ax_raw
cat /sys/kernel/mpu6050/gx_dps
cat /sys/kernel/mpu6050/latency_ns
```

## Run Logger

```bash
python3 imu_logger.py
```

The logger reads accelerometer data every 100ms and saves it to `imu_data.csv`:

```
time,ax,ay,az
1714556123.45,12,-5,1003
```

## Remove Driver

```bash
sudo rmmod mpu6050_driver
echo 0x68 | sudo tee /sys/bus/i2c/devices/i2c-1/delete_device
```

## Sysfs Attributes

| File | Description | Unit |
|------|-------------|------|
| `ax_mg` | Accelerometer X | milli-g |
| `ay_mg` | Accelerometer Y | milli-g |
| `az_mg` | Accelerometer Z | milli-g |
| `ax_raw` | Accelerometer X raw ADC | — |
| `ay_raw` | Accelerometer Y raw ADC | — |
| `az_raw` | Accelerometer Z raw ADC | — |
| `gx_raw` | Gyroscope X raw ADC | — |
| `gx_dps` | Gyroscope X | degrees/sec |
| `latency_ns` | I2C read latency | nanoseconds |
