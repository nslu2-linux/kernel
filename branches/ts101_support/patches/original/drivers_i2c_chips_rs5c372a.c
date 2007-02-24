--- linux-2.6.12.3/drivers/i2c/chips/rs5c372a.c	1969-12-31 19:00:00.000000000 -0500
+++ linux-2.6.12.3-qnap/drivers/i2c/chips/rs5c372a.c	2007-02-17 14:10:17.000000000 -0500
@@ -0,0 +1,297 @@
+#include <linux/config.h>
+#include <linux/module.h>
+#include <linux/version.h>
+#include <linux/kernel.h>
+#include <linux/poll.h>
+#include <linux/i2c.h>
+#include <linux/slab.h>
+#include <linux/init.h>
+#include <linux/rtc.h>
+#include <linux/string.h>
+#include <linux/miscdevice.h>
+#include <linux/time.h>
+
+#define RS5C372A_I2C_SLAVE_ADDR	0x32
+#define RS5C372A_RAM_SIZE		0x10
+#define I2C_DRIVERID_RS5C372A	0xEF
+#define RS5C372A_GETDATETIME	0
+#define RS5C372A_SETTIME		1
+#define RS5C372A_SETDATETIME	2
+#define BCD_TO_BIN(val) 		(((val) & 15) + ((val) >> 4) * 10)
+#define BIN_TO_BCD(val) 		((((val) / 10) << 4) + (val) % 10)
+#define HOURS_24(n)				BCD_TO_BIN((n) & 0x3F)
+
+struct i2c_driver rs5c372a_driver;
+struct i2c_client *rs5c372a_i2c_client = NULL;
+
+static unsigned short ignore[] = {I2C_CLIENT_END};
+static unsigned short normal_addr[] = {RS5C372A_I2C_SLAVE_ADDR, I2C_CLIENT_END};
+static struct i2c_client_address_data addr_data = {
+	normal_i2c : normal_addr,
+	normal_i2c_range : ignore,
+	probe : ignore,
+	probe_range : ignore,
+	ignore : ignore,
+	ignore_range : ignore,
+	force : ignore,
+};
+
+static int rs5c372a_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
+static int rs5c372a_open(struct inode *inode, struct file *file);
+static int rs5c372a_release(struct inode *inode, struct file *file);
+
+static struct file_operations rtc_fops = {
+	owner : THIS_MODULE,
+	ioctl : rs5c372a_ioctl,
+	open : rs5c372a_open,
+	release : rs5c372a_release,
+};
+
+static struct miscdevice rs5c372a_miscdev = {
+	RTC_MINOR,
+	"rtc",
+	&rtc_fops
+};
+
+static int rs5c372a_probe(struct i2c_adapter *adap);
+static int rs5c372a_detach(struct i2c_client *client);
+static int rs5c372a_command(struct i2c_client *client, unsigned int cmd, void *arg);
+
+struct i2c_driver rs5c372a_driver = {
+	name : "rs5c372a",
+	id : I2C_DRIVERID_RS5C372A,
+	flags : I2C_DF_NOTIFY,
+	attach_adapter : rs5c372a_probe,
+	detach_client : rs5c372a_detach,
+	command : rs5c372a_command
+};
+
+static spinlock_t rs5c372a_lock = SPIN_LOCK_UNLOCKED;
+
+static int rs5c372a_read(char *buf, int len)
+{
+	unsigned long flags;
+	struct i2c_msg msgs[1] = {
+		{rs5c372a_i2c_client->addr, I2C_M_RD, len, buf}
+	};
+	int ret;
+
+	spin_lock_irqsave(&rs5c372a_lock, flags);
+	ret = i2c_transfer(rs5c372a_i2c_client->adapter, msgs, 1);
+	spin_unlock_irqrestore(&rs5c372a_lock, flags);
+	return ret;
+}
+
+static int rs5c372a_write(char *buf, int len)
+{
+	unsigned long flags;
+	struct i2c_msg msgs[1] = {
+		{rs5c372a_i2c_client->addr, 0, len, buf}
+	};
+	int ret;
+
+	spin_lock_irqsave(&rs5c372a_lock, flags);
+	ret = i2c_transfer(rs5c372a_i2c_client->adapter, msgs, 1);
+	spin_unlock_irqrestore(&rs5c372a_lock, flags);
+	return ret;
+}
+
+static int rs5c372a_attach(struct i2c_adapter *adap, int addr, int kind)
+{
+	int ret = 0;
+	//int i;
+	struct i2c_client *client;
+	//struct rtc_time rtctm;
+	unsigned char buf[RS5C372A_RAM_SIZE + 1];
+
+	if (!(client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
+		return -ENOMEM;
+	}
+	memset(client, 0, sizeof(struct i2c_client));
+	strncpy(client->name, "rs5c372a", I2C_NAME_SIZE);
+	client->flags = I2C_CLIENT_ALLOW_USE | I2C_DF_NOTIFY;
+	client->addr = addr;
+	client->adapter = adap;
+	client->driver = &rs5c372a_driver;
+	ret = i2c_attach_client(client);
+	if (ret < 0) {
+		kfree(client);
+		return ret;
+	}
+	rs5c372a_i2c_client = client;
+	ret = rs5c372a_read(buf, RS5C372A_RAM_SIZE + 1);
+	if (ret >= 0) {
+		//for (i = 0; i < (RS5C372A_RAM_SIZE + 1); i++)
+		//	printk("rs5c372a register%d before: 0x%x\n", i, buf[i]);
+		buf[1] = (buf[0] | (1 << 5));
+		buf[0] = 0xf0;
+		ret = rs5c372a_write(buf, 2);
+		if (ret >= 0) {
+			//ret = rs5c372a_read(buf, RS5C372A_RAM_SIZE + 1);
+			//if (ret >= 0) {
+				ret = 0;
+			//	for (i = 0; i < (RS5C372A_RAM_SIZE + 1); i++)
+			//		printk("rs5c372a register%d after: 0x%x\n", i, buf[i]);
+			//}
+		}
+	}
+	//rs5c372a_command(rs5c372a_i2c_client, RS5C372A_GETDATETIME, (void *)&rtctm);
+	return ret;
+}
+
+static int rs5c372a_probe(struct i2c_adapter *adap)
+{
+	return i2c_probe(adap, &addr_data, rs5c372a_attach);
+}
+
+static int rs5c372a_detach(struct i2c_client *client)
+{
+	if (client) {
+		i2c_detach_client(client);
+		kfree(client);
+		client = rs5c372a_i2c_client = NULL;
+	}
+	return 0;
+}
+
+static void rs5c372a_convert_to_time(struct rtc_time *dt, char *buf)
+{
+	dt->tm_sec = BCD_TO_BIN(buf[1] & 0x7f);
+	dt->tm_min = BCD_TO_BIN(buf[2] & 0x7f);
+	dt->tm_hour = HOURS_24(buf[3]);
+	dt->tm_wday = BCD_TO_BIN(buf[4] & 0x07);
+	dt->tm_mday = BCD_TO_BIN(buf[5] & 0x3f);
+	dt->tm_mon = BCD_TO_BIN(buf[6] & 0x1f) - 1;
+	dt->tm_year = BCD_TO_BIN(buf[7]) + 100;
+}
+
+static int rs5c372a_get_datetime(struct i2c_client *client, struct rtc_time *dt)
+{
+	unsigned char buf[RS5C372A_RAM_SIZE + 1];
+	int ret = -EIO;
+
+	//ret = rs5c372a_read(buf, RS5C372A_RAM_SIZE + 1);
+	ret = rs5c372a_read(buf, 8);
+	if (ret >= 0) {
+		rs5c372a_convert_to_time(dt, buf);
+		ret = 0;
+	}
+	return ret;
+}
+
+static int rs5c372a_set_datetime(struct i2c_client *client, struct rtc_time *dt, int datetoo)
+{
+	unsigned char buf[RS5C372A_RAM_SIZE + 1];
+	int ret, len = 4;
+
+	buf[0] = 0;
+	buf[1] = (BIN_TO_BCD(dt->tm_sec));
+	buf[2] = (BIN_TO_BCD(dt->tm_min));
+	buf[3] = (BIN_TO_BCD(dt->tm_hour));
+	if (datetoo) {
+		len = 8;
+		buf[4] = (BIN_TO_BCD(dt->tm_wday));
+		buf[5] = (BIN_TO_BCD(dt->tm_mday));
+		buf[6] = (BIN_TO_BCD(dt->tm_mon + 1));
+		buf[7] = (BIN_TO_BCD(dt->tm_year - 100));
+	}
+	ret = rs5c372a_write(buf, len);
+	if (ret >= 0)
+		ret = 0;
+	return ret;
+}
+
+static int rs5c372a_command(struct i2c_client *client, unsigned int cmd, void *arg)
+{
+	switch (cmd) {
+	case RS5C372A_GETDATETIME:
+		return rs5c372a_get_datetime(client, arg);
+	case RS5C372A_SETTIME:
+		return rs5c372a_set_datetime(client, arg, 0);
+	case RS5C372A_SETDATETIME:
+		return rs5c372a_set_datetime(client, arg, 1);
+	default:
+		return -EINVAL;
+	}
+	return 0;
+}
+
+static int rs5c372a_open(struct inode *inode, struct file *file)
+{
+	return 0;
+}
+
+static int rs5c372a_release(struct inode *inode, struct file *file)
+{
+	return 0;
+}
+
+static int rs5c372a_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
+{
+	unsigned long flags;
+	struct rtc_time wtime;
+	int status = 0;
+
+	switch (cmd) {
+	default:
+	case RTC_UIE_ON:
+	case RTC_UIE_OFF:
+	case RTC_PIE_ON:
+	case RTC_PIE_OFF:
+	case RTC_AIE_ON:
+	case RTC_AIE_OFF:
+	case RTC_ALM_SET:
+	case RTC_ALM_READ:
+	case RTC_IRQP_READ:
+	case RTC_IRQP_SET:
+	case RTC_EPOCH_SET:
+	case RTC_WKALM_SET:
+	case RTC_WKALM_RD:
+		status = -EINVAL;
+		break;
+	case RTC_EPOCH_READ:
+		return put_user(1970, (unsigned long *)arg);
+	case RTC_RD_TIME:
+		spin_lock_irqsave(&rs5c372a_lock, flags);
+		rs5c372a_command(rs5c372a_i2c_client, RS5C372A_GETDATETIME, &wtime);
+		spin_unlock_irqrestore(&rs5c372a_lock, flags);
+		if (copy_to_user((void *)arg, &wtime, sizeof(struct rtc_time)))
+			status = -EFAULT;
+		break;
+	case RTC_SET_TIME:
+		if (!capable(CAP_SYS_TIME)) {
+			status = -EACCES;
+			break;
+		}
+		if (copy_from_user(&wtime, (struct rtc_time *)arg, sizeof(struct rtc_time))) {
+			status = -EFAULT;
+			break;
+		}
+		spin_lock_irqsave(&rs5c372a_lock, flags);
+		rs5c372a_command(rs5c372a_i2c_client, RS5C372A_SETDATETIME, &wtime);
+		spin_unlock_irqrestore(&rs5c372a_lock, flags);
+		break;
+	}
+	return status;
+}
+
+static __init int rs5c372a_init(void)
+{
+	int retval = 0;
+
+	retval = i2c_add_driver(&rs5c372a_driver);
+	if (retval == 0) {
+		misc_register(&rs5c372a_miscdev);
+		printk("I2C: Initialize rs5c372a RTC ok\n");
+	}
+	return retval;
+}
+
+static __exit void rs5c372a_exit(void)
+{
+	misc_deregister(&rs5c372a_miscdev);
+	i2c_del_driver(&rs5c372a_driver);
+}
+
+module_init(rs5c372a_init);
+module_exit(rs5c372a_exit);
