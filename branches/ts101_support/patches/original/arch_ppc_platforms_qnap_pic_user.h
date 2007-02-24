--- linux-2.6.12.3/arch/ppc/platforms/qnap_pic_user.h	1969-12-31 19:00:00.000000000 -0500
+++ linux-2.6.12.3-qnap/arch/ppc/platforms/qnap_pic_user.h	2007-02-17 14:10:23.000000000 -0500
@@ -0,0 +1,91 @@
+#ifndef __PPC_PLATFORMS_QNAP_PIC_USER_H
+#define __PPC_PLATFORMS_QNAP_PIC_USER_H
+
+#define QUEUE_BUFSIZE					16
+#define PIC_MINOR						80
+#define PIC_DEV							"/dev/pic"
+
+struct qnap_pic_ioctl {
+	unsigned char pic_data[QUEUE_BUFSIZE];
+	int count;
+};
+
+#define IOCTL_MSG_MAGIC					'Q'
+#define IOCTL_MSG_GET_MESSAGE			_IOR(IOCTL_MSG_MAGIC, 1000, struct qnap_pic_ioctl)
+#define IOCTL_MSG_SEND_MESSAGE			_IOW(IOCTL_MSG_MAGIC, 1001, struct qnap_pic_ioctl)
+#define IOCTL_MSG_SEND_RAW_COMMAND		_IOW(IOCTL_MSG_MAGIC, 1002, struct qnap_pic_ioctl)
+
+#define QNAP_PIC_BOOT_COMPLETE			0
+#define QNAP_PIC_NO_HD				1
+#define QNAP_PIC_WRONG_HD_FORMAT		2
+#define QNAP_PIC_HD_BAD_BLOCK			3
+#define QNAP_PIC_HD_FULL			4
+#define QNAP_PIC_FIRMWARE_UPDATE		5
+#define QNAP_PIC_POWER_OFF			6
+#define QNAP_PIC_HD_STANDBY			7
+#define QNAP_PIC_USB_COPY			8
+#define QNAP_PIC_SET_DEFAULT			9
+#define QNAP_PIC_POWER_RECOVERY			10
+
+#define QNAP_PIC_TOTAL_EVENT			11
+#define QNAP_PIC_EVENT_OFF			0
+#define QNAP_PIC_EVENT_ON			1
+
+#define QNAP_PIC_POWER_BUTTON			0x40
+#define QNAP_PIC_USB_COPY_BUTTON		0x68
+#define QNAP_PIC_SET_DEFAULT_BUTTON		0x6A
+#define QNAP_PIC_FAN_ENABLE			0x71
+#define QNAP_PIC_FAN_DISABLE			0x72
+#define QNAP_PIC_FAN_ERROR			0x73
+#define QNAP_PIC_FAN_NORMAL			0x74
+#define QNAP_PIC_POWER_RECOVERY_ON		0x79
+#define QNAP_PIC_POWER_RECOVERY_OFF		0x7A
+#define QNAP_NET_NIC_UP				0x81
+#define QNAP_NET_NIC_DOWN			0x82
+
+/*add by KenChen for GIGA Lan up notification 20060329
+#define GIGA				1000
+#define QNAP_GIGA_LAN_UP				0x7C
+#define QNAP_NOT_GIGA_LAN_UP				0x7D
+end here*/
+
+//Ricky added some hotswap command
+#define QNAP_ESATA_UP				0x83
+#define QNAP_ESATA_DOWN				0x84
+#define QNAP_USB_FRONT_UP			0x85
+#define QNAP_USB_FRONT_DOWN			0x86
+#define QNAP_USB_B_UPPER_UP			0x87
+#define QNAP_USB_B_UPPER_DOWN			0x88
+#define QNAP_USB_B_LOWER_UP			0x89
+#define QNAP_USB_B_LOWER_DOWN			0x8A
+#define QNAP_USB_PRINTER_UP			0x8B
+#define QNAP_USB_PRINTER_DOWN			0x8C
+#define QNAP_SATA_UP                           	0x8D
+#define QNAP_SATA_DOWN                         	0x8E
+//End
+
+#define MD_REBUILDING				0x91
+#define MD_REBUILDING_DONE			0x92
+#define MD_REBUILDING_SKIP			0x93
+
+#define QNAP_PIC_SOFTWARE_SHUTDOWN   		0x41
+#define QNAP_PIC_POWER_RECOVERY_STATUS	   	0x46
+#define QNAP_PIC_POWER_LED_OFF		   	0x4B
+#define QNAP_PIC_POWER_LED_BLINK		0x4C
+#define QNAP_PIC_POWER_LED_ON		   	0x4D
+#define QNAP_PIC_ENABLE_POWER_RECOVERY		0x48
+#define QNAP_PIC_DISABLE_POWER_RECOVERY		0x49
+#define QNAP_PIC_BUZZER_SHORT			0x50
+#define QNAP_PIC_BUZZER_LONG			0x51
+#define QNAP_PIC_STATUS_RED_BLINK		0x54
+#define QNAP_PIC_STATUS_GREEN_BLINK		0x55
+#define QNAP_PIC_STATUS_GREEN_ON		0x56
+#define QNAP_PIC_STATUS_RED_ON			0x57
+#define QNAP_PIC_STATUS_BOTH_BLINK		0x58
+#define QNAP_PIC_STATUS_OFF			0x59
+#define QNAP_PIC_USB_LED_ON			0x60
+#define QNAP_PIC_USB_LED_BLINK			0x61
+#define QNAP_PIC_USB_LED_OFF			0x62
+#define QNAP_PIC_SOFTWARE_REBOOT     		0x66
+
+#endif /* __PPC_PLATFORMS_QNAP_PIC_USER_H */
