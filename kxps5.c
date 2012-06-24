/*
 Basic kxps5 driver 

 Doug Szumski  <d.s.szumski@gmail.com>  2010-08-12
 based on ds1621.c byChristian W. Zuckschwerdt  <zany@triq.net>  2000-11-23

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 22 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/time.h>

/* Possible chip addresses, can also be 0x19 */
static const unsigned short normal_i2c[] = { 0x18, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(kxps5);

/*KXPS5 chip control register locations*/
#define KXPS5_CREGB		0x0D
#define KXPS5_CREGC		0x0C

/*KXPS5 chip register settings */
#define KXPS5_CREGB_SET		0x42
#define KXPS5_CREGC_SET		0x00

/* KXPS5 acceleration registers */
static const u8 KXPS5_REG_ACCEL[6] = {
	0x00,		/* x acceleration hi */
	0x02,		/* y acceleration hi */
	0x04,		/* z acceleration hi */
};

/* Each client has this additional data */
struct kxps5_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	struct timeval last_updated; 
	char valid;	/* !=0 if following fields are valid */
	u16 accel[3];	/*Acceleration vector*/
	unsigned long dt; /*Microseconds since last acceleration update*/
	u8 conf_reg_b;	/*register B config */		
};


static int kxps5_read_accel(struct i2c_client *client, u8 reg)
{
	u8 lo, hi;
	hi = i2c_smbus_read_byte_data(client, reg);
	lo = i2c_smbus_read_byte_data(client, reg+1);
	return (((hi << 8) | lo) >> 4);
}

static void kxps5_init_client(struct i2c_client *client)
{
	struct kxps5_data *data = i2c_get_clientdata(client);	
	/* u8 conf_reg_b, new_conf_reg_b;
	new_conf_reg_b = conf_reg_b = i2c_smbus_read_byte_data(client, KXPS5_CREGB);
	new_conf_reg_b &= ~KXPS5_CREGB_SET;
	if (conf_reg_b != new_conf_reg_b)
		i2c_smbus_write_byte_data(client, KXPS5_CREGB, new_conf_reg_b); */
	i2c_smbus_write_byte_data(client, KXPS5_CREGB, KXPS5_CREGB_SET);
	data->last_updated.tv_sec = 0;
	data->last_updated.tv_usec = 0;
}

static struct kxps5_data *kxps5_update_client(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxps5_data *data = i2c_get_clientdata(client);	
	struct timeval stamp_time;     
	struct timeval temp;  
	int i;
        
	/*Note: takes between 0 and 300uS to read sensor. Avg ~100uS */

	mutex_lock(&data->update_lock);

	/*Update the acceleration vector by looping over the registers */
	dev_dbg(&client->dev, "Starting KXPS5_REG_ACCEL update\n");
	for (i = 0; i < ARRAY_SIZE(data->accel); i++)
		data->accel[i] = kxps5_read_accel(client,
						 KXPS5_REG_ACCEL[i]);
	
	/*Record time since the epoch, probably ~uS precision*/
	do_gettimeofday(&stamp_time);	
	
	/*Calculate time interval since last update. Returns time since the
	 epoch on first call because last_updated is initialised to zero
	 upon loading the module*/
	if (( stamp_time.tv_usec - data->last_updated.tv_usec ) <0) {
		temp.tv_sec = stamp_time.tv_sec - data->last_updated.tv_sec - 1;
		temp.tv_usec = 1000000 + stamp_time.tv_usec - data->last_updated.tv_usec;
	} else {
		temp.tv_sec = stamp_time.tv_sec - data->last_updated.tv_sec;
		temp.tv_usec = stamp_time.tv_usec - data->last_updated.tv_usec;
	}

	/*Update time interval. Return 0 if >= 1 second or undefined*/
	if ( (temp.tv_sec >= 1) || (temp.tv_usec >= 1000000) ) {
		data->dt = 0;
	} else {
		data->dt = temp.tv_usec;
		/*printk("PASSED %lu %9lu\n", temp.tv_sec, temp.tv_usec);*/
	}
	

	data->last_updated = stamp_time;
	data->valid = 1;	
	mutex_unlock(&data->update_lock);

	return data;
}

static ssize_t show_accel(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct kxps5_data *data = kxps5_update_client(dev);
	return sprintf(buf, "(%d,%d,%d,%6lu)\n", data->accel[attr->index],
			data->accel[(attr->index)+1],
			 data->accel[(attr->index+2)], data->dt );
}

static SENSOR_DEVICE_ATTR(accel_input, S_IRUGO, show_accel, NULL, 0);

static struct attribute *kxps5_attributes[] = {
	&sensor_dev_attr_accel_input.dev_attr.attr,
	NULL
};

static const struct attribute_group kxps5_group = {
	.attrs = kxps5_attributes,
};

/* Return 0 if detection is successful, -ENODEV otherwise */
static int kxps5_detect(struct i2c_client *client, int kind,
			 struct i2c_board_info *info)
{
	strlcpy(info->type, "kxps5", I2C_NAME_SIZE);
	/* Insert final vector half read register check zeros */
	return 0;
}

static int kxps5_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct kxps5_data *data;
	int err;

	data = kzalloc(sizeof(struct kxps5_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	/* Initialize the kxps5 chip */
	kxps5_init_client(client);

	/* Register sysfs hooks */
	if ((err = sysfs_create_group(&client->dev.kobj, &kxps5_group)))
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove_files;
	}

	return 0;

      exit_remove_files:
	sysfs_remove_group(&client->dev.kobj, &kxps5_group);
      exit_free:
	kfree(data);
      exit:
	return err;
}

static int kxps5_remove(struct i2c_client *client)
{
	struct kxps5_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &kxps5_group);

	kfree(data);

	return 0;
}

static const struct i2c_device_id kxps5_id[] = {
	{ "kxps5", kxps5 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, kxps5_id);

/* This is the driver that will be inserted */
static struct i2c_driver kxps5_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "kxps5",
	},
	.probe		= kxps5_probe,
	.remove		= kxps5_remove,
	.id_table	= kxps5_id,
	.detect		= kxps5_detect,
	.address_data	= &addr_data,
};

static int __init kxps5_init(void)
{
	return i2c_add_driver(&kxps5_driver);
}

static void __exit kxps5_exit(void)
{
	i2c_del_driver(&kxps5_driver);
}

MODULE_AUTHOR("Doug Szumski <d.s.szumski@gmail.com>");
MODULE_DESCRIPTION("kxps5 driver");
MODULE_LICENSE("GPL");

module_init(kxps5_init);
module_exit(kxps5_exit);
