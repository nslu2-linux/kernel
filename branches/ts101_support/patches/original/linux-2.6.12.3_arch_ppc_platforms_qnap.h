--- linux-2.6.12.3/arch/ppc/platforms/qnap.h	1969-12-31 19:00:00.000000000 -0500
+++ linux-2.6.12.3-qnap/arch/ppc/platforms/qnap.h	2007-02-17 14:10:23.000000000 -0500
@@ -0,0 +1,56 @@
+#ifndef __PPC_PLATFORMS_QNAP_H
+#define __PPC_PLATFORMS_QNAP_H
+
+// FIXME, what should we do if external multiport hub is connected?
+#define USB_MASS_STORAGE_DEVICE_NUMBER			3
+
+#define USB_MASS_STORAGE_DEVICE_FAILED			-1
+#define USB_MASS_STORAGE_DEVICE_DISCONNECTED	0
+#define	USB_MASS_STORAGE_DEVICE_CONNECTED		1
+#define USB_MASS_STORAGE_DEVICE_READY			2
+
+#define USB_MASS_STORAGE_DEVICE_FRONT_PORT		0
+#define	USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT	1
+#define USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT	2
+
+#define USB_HIGH_SPEED	1
+#define USB_LOW_SPEED	2
+#define USB_FULL_SPEED	3
+
+#define SATA1	1
+#define SATA2	2
+#define MAX_USB_ENTRY   3
+
+#define USB_PORT        1
+#define USB_HUB_PORT1   1<<4
+#define USB_HUB_PORT2   2<<4
+#define USB_HUB_PORT3   4<<4
+#define USB_HUB_PORT4   8<<4
+#define USB_HUB_PORT5   1<<8
+#define USB_HUB_PORT6   2<<8
+#define USB_HUB_PORT7   3<<8
+
+struct qnap_usb_mass_storage_device_info {
+	int status;
+	int port;
+	int size; // total disk size in MB
+	char name[32]; // scsi device name
+	int partno;
+	int cnt_of_device;
+	int cnt_of_avail_device;
+};
+
+extern int dev_exist[2];	// 2 == MAX_ENTRY
+
+extern int usbdev_exist[MAX_USB_ENTRY];
+extern int usbdev_status_changed[MAX_USB_ENTRY];
+extern int usbdev_speedtype[MAX_USB_ENTRY];
+
+extern struct qnap_usb_mass_storage_device_info qnap_usb_front_port;
+
+extern int qnap_get_ts101_sata_port(struct scsi_device *scsidev);
+extern int qnap_get_ts101_usb_port(struct device *dev);
+extern int qnap_get_ts101_usb_port_ex(struct device *dev, int *hub_port, int *speed);
+extern int send_message_to_app(unsigned char message);
+
+#endif /* __PPC_PLATFORMS_QNAP_H */
