import time

AX = "/sys/kernel/mpu6050/ax_mg"
AY = "/sys/kernel/mpu6050/ay_mg"
AZ = "/sys/kernel/mpu6050/az_mg"

LOG_FILE = "imu_data.csv"

def read_value(path):
    try:
        with open(path, 'r') as f:
            return int(f.read().strip())
    except:
        return 0

# create CSV with header
with open(LOG_FILE, "w") as f:
    f.write("time,ax,ay,az\n")

print("Logging started... Press Ctrl+C to stop")

try:
    while True:
        t = time.time()

        ax = read_value(AX)
        ay = read_value(AY)
        az = read_value(AZ)

        print(f"{t:.2f}  X:{ax}  Y:{ay}  Z:{az}")

        with open(LOG_FILE, "a") as f:
            f.write(f"{t},{ax},{ay},{az}\n")

        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nLogging stopped.")
