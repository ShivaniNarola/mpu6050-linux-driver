#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/ktime.h>
#include <linux/mutex.h>

// Registers
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_WHO_AM_I     0x75

#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_ACCEL_YOUT_H 0x3D
#define MPU6050_REG_ACCEL_ZOUT_H 0x3F

#define MPU6050_REG_GYRO_XOUT_H  0x43

#define ACCEL_SENS_2G 16384
#define GYRO_SENS_250DPS 131

static struct i2c_client *mpu_client;
static struct kobject *mpu_kobj;

// data
static int ax_raw, ay_raw, az_raw;
static int gx_raw;

static int ax_mg, ay_mg, az_mg;
static int gx_dps;

static s64 last_latency;

static DEFINE_MUTEX(mpu_lock);

// optimized read
static int mpu_read_word(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_word_swapped(client, reg);
}

// conversions
static int accel_to_mg(int raw)
{
    return (raw * 1000) / ACCEL_SENS_2G;
}

static int gyro_to_dps(int raw)
{
    return raw / GYRO_SENS_250DPS;
}

// update data
static int update_data(void)
{
    ktime_t t1, t2;

    mutex_lock(&mpu_lock);

    t1 = ktime_get();

    ax_raw = (s16)mpu_read_word(mpu_client, MPU6050_REG_ACCEL_XOUT_H);
    ay_raw = (s16)mpu_read_word(mpu_client, MPU6050_REG_ACCEL_YOUT_H);
    az_raw = (s16)mpu_read_word(mpu_client, MPU6050_REG_ACCEL_ZOUT_H);

    gx_raw = (s16)mpu_read_word(mpu_client, MPU6050_REG_GYRO_XOUT_H);

    t2 = ktime_get();

    ax_mg = accel_to_mg(ax_raw);
    ay_mg = accel_to_mg(ay_raw);
    az_mg = accel_to_mg(az_raw);

    gx_dps = gyro_to_dps(gx_raw);

    last_latency = ktime_to_ns(ktime_sub(t2, t1));

    mutex_unlock(&mpu_lock);

    return 0;
}

// macro for sysfs
#define CREATE_SHOW(name, val) \
static ssize_t name##_show(struct kobject *kobj, \
struct kobj_attribute *attr, char *buf) \
{ \
    update_data(); \
    return sprintf(buf, "%d\n", val); \
}

// accel raw
CREATE_SHOW(ax_raw, ax_raw)
CREATE_SHOW(ay_raw, ay_raw)
CREATE_SHOW(az_raw, az_raw)

// accel mg
CREATE_SHOW(ax_mg, ax_mg)
CREATE_SHOW(ay_mg, ay_mg)
CREATE_SHOW(az_mg, az_mg)

// gyro
CREATE_SHOW(gx_raw, gx_raw)
CREATE_SHOW(gx_dps, gx_dps)

// latency
static ssize_t latency_show(struct kobject *kobj,
struct kobj_attribute *attr, char *buf)
{
    update_data();
    return sprintf(buf, "%lld\n", last_latency);
}

// attributes
#define CREATE_ATTR(name) \
static struct kobj_attribute name##_attr = \
__ATTR(name, 0444, name##_show, NULL);

CREATE_ATTR(ax_raw)
CREATE_ATTR(ay_raw)
CREATE_ATTR(az_raw)

CREATE_ATTR(ax_mg)
CREATE_ATTR(ay_mg)
CREATE_ATTR(az_mg)

CREATE_ATTR(gx_raw)
CREATE_ATTR(gx_dps)

static struct kobj_attribute latency_attr =
__ATTR(latency_ns, 0444, latency_show, NULL);

// probe
static int mpu_probe(struct i2c_client *client)
{
    int id, ret;

    pr_info("mpu6050: driver start\n");

    mpu_client = client;

    id = i2c_smbus_read_byte_data(client, MPU6050_REG_WHO_AM_I);
    if (id != 0x68) {
        pr_err("mpu6050: wrong device id = 0x%x\n", id);
        return -ENODEV;
    }

    ret = i2c_smbus_write_byte_data(client, MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret < 0)
        return ret;

    msleep(100);

    mpu_kobj = kobject_create_and_add("mpu6050", kernel_kobj);
    if (!mpu_kobj)
        return -ENOMEM;

    // sysfs with error handling
    if (sysfs_create_file(mpu_kobj, &ax_raw_attr.attr)) goto err;
    if (sysfs_create_file(mpu_kobj, &ay_raw_attr.attr)) goto err;
    if (sysfs_create_file(mpu_kobj, &az_raw_attr.attr)) goto err;

    if (sysfs_create_file(mpu_kobj, &ax_mg_attr.attr)) goto err;
    if (sysfs_create_file(mpu_kobj, &ay_mg_attr.attr)) goto err;
    if (sysfs_create_file(mpu_kobj, &az_mg_attr.attr)) goto err;

    if (sysfs_create_file(mpu_kobj, &gx_raw_attr.attr)) goto err;
    if (sysfs_create_file(mpu_kobj, &gx_dps_attr.attr)) goto err;

    if (sysfs_create_file(mpu_kobj, &latency_attr.attr)) goto err;

    pr_info("mpu6050: ready (multi-axis)\n");
    return 0;

err:
    kobject_put(mpu_kobj);
    return -1;
}

// remove
static void mpu_remove(struct i2c_client *client)
{
    if (mpu_kobj)
        kobject_put(mpu_kobj);

    pr_info("mpu6050: removed\n");
}

// driver
static const struct i2c_device_id mpu_ids[] = {
    { "mpu6050", 0 },
    {}
};

static struct i2c_driver mpu_driver = {
    .driver = {
        .name = "mpu6050_driver",
    },
    .probe = mpu_probe,
    .remove = mpu_remove,
    .id_table = mpu_ids,
};

module_i2c_driver(mpu_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shivani");
MODULE_DESCRIPTION("MPU6050 multi-axis Linux driver");
