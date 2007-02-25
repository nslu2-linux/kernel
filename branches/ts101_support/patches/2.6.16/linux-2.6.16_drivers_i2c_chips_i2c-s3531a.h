--- linux-2.6.16.vanilla/drivers/i2c/chips/i2c-s3531a.h	1970-01-01 01:00:00.000000000 +0100
+++ linux-2.6.16/drivers/i2c/chips/i2c-s3531a.h	2007-02-25 05:09:57.000000000 +0100
@@ -0,0 +1,58 @@
+/*
+ *  linux/include/linux/i2c-s3531a.h
+ *
+ *  Copyright (C) 2001-2005 Atmark Techno, Inc.
+ */
+
+#define I2C_DRIVERID_S3531A  46
+
+#define RTC_SYNC	     0
+#define RTC_GETTIME	     1
+#define RTC_GETDATETIME	     2
+#define RTC_SETTIME	     3
+#define RTC_SETDATETIME	     4
+#define RTC_GETSTATUS	     5
+#define RTC_SETSTATUS	     6
+#define RTC_GETDEVTYPE       7
+#define RTC_GETSTATUS1	     8
+#define RTC_SETSTATUS1	     9
+#define RTC_GETSTATUS2	     RTC_GETSTATUS
+#define RTC_SETSTATUS2	     RTC_SETSTATUS
+
+#define DEVTYPE_S3531A  1
+#define DEVTYPE_S353X0A 80
+
+#define STATUS_POWER           0x01
+#define STATUS_12_24           0x02
+#define STATUS_INT1AE          0x04
+#define STATUS_INT1ME          0x10
+#define STATUS_INT1FE          0x40
+
+#define S353X0A_STATUS1_POC    0x01
+#define S353X0A_STATUS1_BLD    0x02
+#define S353X0A_STATUS1_INT2   0x04
+#define S353X0A_STATUS1_INT1   0x08
+#define S353X0A_STATUS1_SC2    0x10
+#define S353X0A_STATUS1_SC1    0x20
+#define S353X0A_STATUS1_12_24  0x40
+#define S353X0A_STATUS1_RESET  0x80
+#define S353X0A_STATUS2_TEST   0x01
+#define S353X0A_STATUS2_INT2AE 0x02
+#define S353X0A_STATUS2_INT2ME 0x04
+#define S353X0A_STATUS2_INT2FE 0x08
+#define S353X0A_STATUS2_32KE   0x10
+#define S353X0A_STATUS2_INT1AE 0x20
+#define S353X0A_STATUS2_INT1ME 0x40
+#define S353X0A_STATUS2_INT1FE 0x80
+
+#define I2C_S3531A_ADDRESS   0x30
+
+struct s3531a_time {
+  int year; /* Year - 2000.        */
+  int mon;  /* Month.       [1-12] */
+  int mday; /* Day.         [1-31] */
+  int wday; /* Day of week. [0-6]  */
+  int hour; /* Hours.       [0-23] */
+  int min;  /* Minutes.     [0-59] */
+  int sec;  /* Seconds.     [0-59] */
+};
