# MPU6050 Linux Kernel Driver

Linux kernel I2C driver for the MPU6050 IMU sensor on Raspberry Pi. The driver reads accelerometer and gyroscope data and exposes it through sysfs. A Python-based headless logger is included for real-time data capture.

## Features

- I2C-based kernel driver  
- Accelerometer (X, Y, Z) and gyroscope data  
- Fixed-point conversion (mg, dps)  
- Thread-safe access using mutex  
- Sysfs interface for user-space access  

## Build


make clean
make


## Load Driver


sudo insmod mpu6050_driver.ko


## Attach Device


echo mpu6050 0x68 | sudo tee /sys/bus/i2c/devices/i2c-1/new_device


## Read Data


cat /sys/kernel/mpu6050/ax_mg
cat /sys/kernel/mpu6050/ay_mg
cat /sys/kernel/mpu6050/az_mg


## Run Logger


python3 imu_logger.py


## Remove Driver


sudo rmmod mpu6050_driver
echo 0x68 | sudo tee /sys/bus/i2c/devices/i2c-1/delete_device


