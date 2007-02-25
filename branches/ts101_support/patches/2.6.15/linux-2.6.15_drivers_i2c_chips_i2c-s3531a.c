--- linux-2.6.15.vanilla/drivers/i2c/chips/i2c-s3531a.c	1970-01-01 01:00:00.000000000 +0100
+++ linux-2.6.15/drivers/i2c/chips/i2c-s3531a.c	2007-02-25 02:49:46.000000000 +0100
@@ -0,0 +1,602 @@
+/*
+ *  linux/drivers/i2c/chips/i2c-s3531a.c
+ *
+ *  Copyright QNAP Systems, Inc.
+ */
+
+#include <linux/module.h>
+#include <linux/i2c.h>
+#include <linux/slab.h>
+#include <linux/string.h>
+#include <linux/bcd.h>
+#include <linux/delay.h>
+#include <linux/sched.h>
+#include <linux/ioctl.h>
+#include <linux/config.h>
+#include <linux/version.h>
+#include <linux/kernel.h>
+#include <linux/poll.h>
+#include <linux/init.h>
+#include <linux/rtc.h>
+#include <linux/miscdevice.h>
+#include <linux/time.h>
+
+#include "i2c-s3531a.h"
+
+#define REVERSE(x) ((((x) << 7) & 0x80) | (((x) << 5) & 0x40) | \
+                    (((x) << 3) & 0x20) | (((x) << 1) & 0x10) | \
+                    (((x) >> 1) & 0x08) | (((x) >> 3) & 0x04) | \
+                    (((x) >> 5) & 0x02) | (((x) >> 7) & 0x01))
+
+#define AUTHOR "Copyright QNAP Systems, Inc."
+#define DESCRIPTION "i2c S-3531A/S-353X0A driver"
+#define LICENSE "GPL v2"
+
+#define CHRDEV  "i2c-s3531a"
+#define VERSION CHRDEV ": " DESCRIPTION ", (C) 2001-2005 " AUTHOR
+
+static int devtype = 0;
+static spinlock_t s3531a_lock = SPIN_LOCK_UNLOCKED;
+struct i2c_client *s3531a_i2c_client = NULL;
+
+static struct i2c_driver s3531a_driver;
+
+static unsigned short ignore[] = { I2C_CLIENT_END };
+static unsigned short normal_addr[] = { I2C_S3531A_ADDRESS, I2C_CLIENT_END };
+
+static struct i2c_client_address_data addr_data = {
+  normal_i2c:       normal_addr,
+  normal_i2c_range: ignore,
+  probe:            ignore,
+  probe_range:	    ignore,
+  ignore:	    ignore,
+  ignore_range:	    ignore,
+  force:	    ignore,
+};
+
+static int s3531a_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
+static int s3531a_open(struct inode *inode, struct file *file);
+static int s3531a_release(struct inode *inode, struct file *file);
+
+
+static struct file_operations rtc_fops = {
+        owner : THIS_MODULE,
+        ioctl : s3531a_ioctl,
+        open : s3531a_open,
+        release : s3531a_release,
+};
+
+static struct miscdevice s3531a_miscdev = {
+        RTC_MINOR,
+        "rtc",
+        &rtc_fops
+};
+
+#define GET_STATUS1(x) (((unsigned int)(i2c_get_clientdata(x)))&0xff)
+#define SET_STATUS1(v,x) \
+        i2c_set_clientdata(x,(void*)((((unsigned int)(i2c_get_clientdata(x)))&~0xff)|v))
+#define GET_STATUS2(x) ((((unsigned int)(i2c_get_clientdata(x)))>>8)&0xff)
+#define SET_STATUS2(v,x) \
+        i2c_set_clientdata(x,(void*)((((unsigned int)(i2c_get_clientdata(x)))&~0xff00)|(v<<8)))
+
+static inline int
+s3531a_sync(struct i2c_client *client)
+{
+  unsigned char buf[3];
+  struct i2c_msg msgs[1] = {
+    { client->addr | 0x3, I2C_M_RD, 3, buf }
+  };
+  unsigned char sec;
+  unsigned long end = jiffies + HZ * 2;
+
+  if (i2c_transfer(client->adapter, msgs, 1) == 1) {
+    sec = buf[2];
+    while (i2c_transfer(client->adapter, msgs, 1) == 1) {
+      if (sec != buf[2]) {
+	return 0;
+      }
+      if (jiffies >= end) {
+	break;
+      }
+    }
+  }
+
+  return -1;
+}
+
+static int
+s3531a_attach(struct i2c_adapter *adap, int addr, int kind)
+{
+  struct i2c_client *c;
+  unsigned char buf[3];
+  struct i2c_msg msgs[1] = {
+    { addr | 0x1, I2C_M_RD, 1, buf }
+  };
+  
+  c = kmalloc(sizeof(*c), GFP_KERNEL);
+  if (!c)
+    return -ENOMEM;
+  
+  memset(c, 0, sizeof(struct i2c_client));
+  strcpy(c->name, "S-3531A/S-353X0A");
+  c->flags   = 0;
+  c->addr    = addr;
+  c->adapter = adap;
+  c->driver  = &s3531a_driver;
+  i2c_set_clientdata(c, NULL);
+
+  if (s3531a_sync(c)) {
+    printk("i2c-s3531a: RTC stopped.\n");
+    msgs[0].addr = addr | 0x0;
+    msgs[0].flags = I2C_M_RD;
+    msgs[0].len = 1;
+    if (i2c_transfer(c->adapter, msgs, 1) == 1) {
+      msgs[0].flags = 0;
+      buf[0] |= S353X0A_STATUS1_RESET;
+      if (i2c_transfer(c->adapter, msgs, 1) == 1) {
+	printk("i2c-s3531a: S-3531A/S-353x0A Reset.\n");
+	devtype = DEVTYPE_S3531A;
+      }
+    }
+    if (!devtype) {
+      printk("i2c-s3531a: S-3531A Reset.\n");
+      devtype = DEVTYPE_S3531A;
+    }
+  }
+  else {
+    devtype = DEVTYPE_S3531A;
+  }
+  
+  SET_STATUS1(0, c);
+  msgs[0].addr = addr | 0x1;
+  msgs[0].flags = I2C_M_RD;
+  msgs[0].len = 1;
+  if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+    kfree(c);
+    return -EINVAL;
+  }
+  msgs[0].flags = 0;
+  buf[0] = (buf[0] & ~(S353X0A_STATUS2_INT2ME | S353X0A_STATUS2_32KE |
+		       S353X0A_STATUS2_INT1ME | S353X0A_STATUS2_INT1FE)) |
+    S353X0A_STATUS2_INT1AE;
+  if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+    kfree(c);
+    return -EINVAL;
+  }
+  msgs[0].flags = I2C_M_RD;
+  if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+    kfree(c);
+    return -EINVAL;
+  }
+  if (!(buf[0] & (S353X0A_STATUS2_INT2ME | S353X0A_STATUS2_32KE |
+		  S353X0A_STATUS2_INT1ME | S353X0A_STATUS2_INT1FE)) &&
+      (buf[0] & S353X0A_STATUS2_INT1AE)) {
+    SET_STATUS2(buf[0], c);
+    msgs[0].addr = addr | 0x4;
+    msgs[0].flags = 0;
+    msgs[0].len = 3;
+    buf[0] = buf[1] = buf[2] = 0;
+    if (i2c_transfer(c->adapter, msgs, 1) == 1) {
+      msgs[0].flags = I2C_M_RD;
+      buf[0] = buf[1] = buf[2] = 0xff;
+      if (i2c_transfer(c->adapter, msgs, 1) == 1) {
+	if (!buf[0] && !buf[1] && !buf[2]) {
+	  devtype = DEVTYPE_S353X0A;
+	}
+      }
+    }
+    msgs[0].len = 1;
+    buf[0] = GET_STATUS2(c);
+  }
+  if (devtype == DEVTYPE_S3531A) {
+    buf[0] &= ~(STATUS_INT1AE | STATUS_INT1ME | STATUS_INT1FE);
+    buf[0] |= STATUS_12_24;
+  }
+  else if (devtype == DEVTYPE_S353X0A) {
+    buf[0] &= ~(S353X0A_STATUS2_INT2AE | S353X0A_STATUS2_INT2ME |
+		S353X0A_STATUS2_INT2FE | S353X0A_STATUS2_32KE |
+		S353X0A_STATUS2_INT1AE | S353X0A_STATUS2_INT1ME |
+		S353X0A_STATUS2_INT1FE);
+  }
+  msgs[0].addr = addr | 0x1;
+  msgs[0].flags = 0;
+  if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+    kfree(c);
+    return -EINVAL;
+  }
+  msgs[0].flags = I2C_M_RD;
+  if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+    kfree(c);
+    return -EINVAL;
+  }
+  SET_STATUS2(buf[0], c);
+  if (devtype == DEVTYPE_S353X0A) {
+    msgs[0].addr = addr | 0x0;
+    msgs[0].flags = I2C_M_RD;
+    if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+      kfree(c);
+      return -EINVAL;
+    }
+    if ((buf[0] & (S353X0A_STATUS1_POC | S353X0A_STATUS1_BLD)) ||
+	(GET_STATUS2(c) & S353X0A_STATUS2_TEST)) {
+      msgs[0].flags = 0;
+      buf[0] |= S353X0A_STATUS1_RESET;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+      printk("i2c-s3531a: S-353x0A Reset.\n");
+      msgs[0].flags = I2C_M_RD;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+    }
+    SET_STATUS1(buf[0], c);
+    if (GET_STATUS2(c) & S353X0A_STATUS2_TEST) {
+      msgs[0].addr = addr | 0x1;
+      msgs[0].flags = 0;
+      buf[0] = GET_STATUS2(c) & ~S353X0A_STATUS2_TEST;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+      msgs[0].flags = I2C_M_RD;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+      SET_STATUS2(buf[0], c);
+    }
+    if (!(GET_STATUS1(c) & S353X0A_STATUS1_12_24)) {
+      msgs[0].addr = addr | 0x0;
+      msgs[0].flags = 0;
+      buf[0] = GET_STATUS1(c) | S353X0A_STATUS1_12_24;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+      msgs[0].flags = I2C_M_RD;
+      if (i2c_transfer(c->adapter, msgs, 1) != 1) {
+	kfree(c);
+	return -EINVAL;
+      }
+      SET_STATUS1(buf[0], c);
+    }
+  }
+  printk("i2c-s3531a: Device Type [%s]\n",
+	 (devtype == DEVTYPE_S3531A) ? "S-3531A" : "S-353x0A");
+	
+  s3531a_i2c_client = c;
+  
+  return i2c_attach_client(c);
+}
+
+static int
+s3531a_probe(struct i2c_adapter *adap)
+{
+  return i2c_probe(adap, &addr_data, s3531a_attach);
+}
+
+static int
+s3531a_detach(struct i2c_client *client)
+{
+  i2c_detach_client(client);
+  kfree(client);
+  return 0;
+}
+
+static int
+s3531a_get_datetime(struct i2c_client *client, struct s3531a_time *dt,
+		    int datetoo)
+{
+  unsigned char buf[7];
+  struct i2c_msg msgs[1] = {
+    { client->addr | 0x2 | !datetoo, I2C_M_RD, datetoo ? 7 : 3, buf  }
+  };
+  int ret, i = 0;
+
+int j=0;
+  
+  ret = i2c_transfer(client->adapter, msgs, 1);
+//	for(j=0;j<7; j++)
+//		printk("%d: %X\n", j, buf[j]);
+  if (ret == 1) {
+    if (datetoo) {
+      dt->year = REVERSE(buf[i]);
+      BCD_TO_BIN(dt->year);
+      i++;
+      dt->mon  = REVERSE(buf[i]);
+      BCD_TO_BIN(dt->mon);
+      i++;
+      dt->mday = REVERSE(buf[i]);
+      BCD_TO_BIN(dt->mday);
+      i++;
+      dt->wday = REVERSE(buf[i]);
+      BCD_TO_BIN(dt->wday);
+      i++;
+    }
+    dt->hour = REVERSE(buf[i]) & 0x3f;
+    BCD_TO_BIN(dt->hour);
+    i++;
+    dt->min  = REVERSE(buf[i]);
+    BCD_TO_BIN(dt->min);
+    i++;
+    dt->sec  = REVERSE(buf[i]);
+    BCD_TO_BIN(dt->sec);
+//printk("Sec = %X, %d\n", buf[i], dt->sec);
+    
+    ret = 0;
+  }
+  
+  return ret;
+}
+
+static int
+s3531a_set_datetime(struct i2c_client *client, struct s3531a_time *dt,
+		    int datetoo)
+{
+  unsigned char buf[7];
+  struct i2c_msg msgs[1] = {
+    { client->addr | 0x2 | !datetoo, 0, datetoo ? 7 : 3, buf  }
+  };
+  int ret, i = 0;
+  
+  if (datetoo) {
+    buf[i] = dt->year;
+    BIN_TO_BCD (buf[i]);
+    buf[i] = REVERSE(buf[i]);
+    i++;
+    buf[i] = dt->mon;
+    BIN_TO_BCD (buf[i]);
+    buf[i] = REVERSE(buf[i]);
+    i++;
+    buf[i] = dt->mday;
+    BIN_TO_BCD (buf[i]);
+    buf[i] = REVERSE(buf[i]);
+    i++;
+    buf[i] = dt->wday;
+    BIN_TO_BCD (buf[i]);
+    buf[i] = REVERSE(buf[i]);
+    i++;
+  }
+  buf[i] = dt->hour;
+  BIN_TO_BCD (buf[i]);
+  buf[i] = REVERSE(buf[i]);
+  i++;
+  buf[i] = dt->min;
+  BIN_TO_BCD (buf[i]);
+  buf[i]   = REVERSE(buf[i]);
+  i++;
+  buf[i] = dt->sec;
+  BIN_TO_BCD (buf[i]);
+  buf[i] = REVERSE(buf[i]);
+  
+  ret = i2c_transfer(client->adapter, msgs, 1);
+  if (ret == 1) {
+    ret = 0;
+  }
+  
+  return ret;
+}
+
+static int
+s3531a_get_status1(struct i2c_client *client, unsigned char *status)
+{
+  *status = GET_STATUS1(client);
+  return 0;
+}
+
+static int
+s3531a_set_status1(struct i2c_client *client, unsigned char *status)
+{
+  unsigned char buf[1];
+  struct i2c_msg msgs[1] = {
+    { client->addr | 0x0, 0, 1, buf }
+  };
+  int ret;
+  
+  buf[0] = *status;
+  ret = i2c_transfer(client->adapter, msgs, 1);
+  if (ret == 1) {
+    msgs[0].flags = I2C_M_RD;
+    ret = i2c_transfer(client->adapter, msgs, 1);
+    if (ret == 1) {
+      SET_STATUS1(buf[0], client);
+      ret = 0;
+    }
+  }
+  
+  return ret;
+}
+
+static int
+s3531a_get_status2(struct i2c_client *client, unsigned char *status)
+{
+  *status = GET_STATUS2(client);
+  return 0;
+}
+
+static int
+s3531a_set_status2(struct i2c_client *client, unsigned char *status)
+{
+  unsigned char buf[1];
+  struct i2c_msg msgs[1] = {
+    { client->addr | 0x1, 0, 1, buf }
+  };
+  int ret;
+  
+  buf[0] = *status;
+  ret = i2c_transfer(client->adapter, msgs, 1);
+  if (ret == 1) {
+    msgs[0].flags = I2C_M_RD;
+    ret = i2c_transfer(client->adapter, msgs, 1);
+    if (ret == 1) {
+      SET_STATUS2(buf[0], client);
+      ret = 0;
+    }
+  }
+  
+  return ret;
+}
+
+static int
+s3531a_command(struct i2c_client *client, unsigned int cmd, void *arg)
+{
+  switch (cmd) {
+  case RTC_SYNC:
+    return s3531a_sync(client);
+
+  case RTC_GETTIME:
+    return s3531a_get_datetime(client, arg, 0);
+    
+  case RTC_GETDATETIME:
+    return s3531a_get_datetime(client, arg, 1);
+    
+  case RTC_SETTIME:
+    return s3531a_set_datetime(client, arg, 0);
+    
+  case RTC_SETDATETIME:
+    return s3531a_set_datetime(client, arg, 1);
+
+  case RTC_GETDEVTYPE:
+    *(int *)arg = devtype;
+    return 0;
+    
+  case RTC_GETSTATUS1:
+    return s3531a_get_status1(client, arg);
+    
+  case RTC_SETSTATUS1:
+    return s3531a_set_status1(client, arg);
+    
+  case RTC_GETSTATUS2:
+    return s3531a_get_status2(client, arg);
+    
+  case RTC_SETSTATUS2:
+    return s3531a_set_status2(client, arg);
+  }
+  
+  return -EINVAL;
+}
+
+static int s3531a_open(struct inode *inode, struct file *file)
+{
+        return 0;
+}
+
+static int s3531a_release(struct inode *inode, struct file *file)
+{
+        return 0;
+}
+
+static int s3531a_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
+{
+        unsigned long flags;
+	struct s3531a_time dt;
+        struct rtc_time wtime;
+        int status = 0;
+
+        switch (cmd) {
+        default:
+        case RTC_UIE_ON:
+        case RTC_UIE_OFF:
+        case RTC_PIE_ON:
+        case RTC_PIE_OFF:
+        case RTC_AIE_ON:
+        case RTC_AIE_OFF:
+        case RTC_ALM_SET:
+        case RTC_ALM_READ:
+        case RTC_IRQP_READ:
+        case RTC_IRQP_SET:
+        case RTC_EPOCH_SET:
+        case RTC_WKALM_SET:
+        case RTC_WKALM_RD:
+                status = -EINVAL;
+                break;
+        case RTC_EPOCH_READ:
+                return put_user(1970, (unsigned long *)arg);
+        case RTC_RD_TIME:
+                spin_lock_irqsave(&s3531a_lock, flags);
+
+                s3531a_command(s3531a_i2c_client, RTC_GETDATETIME, &dt);
+		wtime.tm_year = dt.year+100;
+		wtime.tm_mon = dt.mon-1;
+  		wtime.tm_mday = dt.mday;
+		wtime.tm_wday = dt.wday;
+		wtime.tm_hour = dt.hour;
+  		wtime.tm_min = dt.min;
+  		wtime.tm_sec = dt.sec;
+
+                spin_unlock_irqrestore(&s3531a_lock, flags);
+                if (copy_to_user((void *)arg, &wtime, sizeof(struct rtc_time)))
+                        status = -EFAULT;
+                break;
+        case RTC_SET_TIME:
+                if (!capable(CAP_SYS_TIME)) {
+                        status = -EACCES;
+                        break;
+                }
+                if (copy_from_user(&wtime, (struct rtc_time *)arg, sizeof(struct rtc_time))) {
+                        status = -EFAULT;
+                        break;
+                }
+                spin_lock_irqsave(&s3531a_lock, flags);
+
+		dt.year = (wtime.tm_year-100)%100;
+		dt.mon = wtime.tm_mon+1;
+		dt.mday = wtime.tm_mday;
+		dt.wday = wtime.tm_wday;
+		dt.hour = wtime.tm_hour;
+		dt.min = wtime.tm_min;
+		dt.sec = wtime.tm_sec;
+
+              	s3531a_command(s3531a_i2c_client, RTC_SETDATETIME, &dt);
+                spin_unlock_irqrestore(&s3531a_lock, flags);
+                break;
+        }
+        return status;
+}
+
+
+static struct i2c_driver s3531a_driver = {
+  name:		  "S-3531A/S-353X0A",
+  id:		  I2C_DRIVERID_S3531A,
+  flags:	  I2C_DF_NOTIFY,
+  attach_adapter: s3531a_probe,
+  detach_client:  s3531a_detach,
+  command:	  s3531a_command
+};
+
+static int __init s3531a_init(void)
+{
+  int ret;
+  
+  ret = i2c_add_driver(&s3531a_driver);
+  if (ret) {
+    printk (KERN_WARNING CHRDEV ": Unable to register %s driver.\n",
+	    s3531a_driver.name);
+    return ret;
+  }
+  ret = misc_register(&s3531a_miscdev);
+  if(ret) {
+    printk (KERN_WARNING CHRDEV ": Unable to register rtc misc device.\n");
+    return ret;
+  }
+
+  
+  printk (KERN_INFO VERSION "\n");
+  return 0;
+}
+
+static void __exit s3531a_exit(void)
+{
+  misc_deregister(&s3531a_miscdev);
+  i2c_del_driver(&s3531a_driver);
+}
+
+MODULE_AUTHOR(AUTHOR);
+MODULE_DESCRIPTION(DESCRIPTION);
+MODULE_LICENSE(LICENSE);
+
+module_init(s3531a_init);
+module_exit(s3531a_exit);
