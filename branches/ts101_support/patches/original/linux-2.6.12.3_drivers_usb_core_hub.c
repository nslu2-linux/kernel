--- linux-2.6.12.3/drivers/usb/core/hub.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/usb/core/hub.c	2007-02-17 14:10:14.000000000 -0500
@@ -26,6 +26,10 @@
 #include <linux/ioctl.h>
 #include <linux/usb.h>
 #include <linux/usbdevice_fs.h>
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap_pic.h>
+#include <platforms/qnap_pic_user.h>
+#endif
 
 #include <asm/semaphore.h>
 #include <asm/uaccess.h>
@@ -2421,8 +2425,15 @@
 	}
  
 	/* Disconnect any existing devices under this port */
-	if (hdev->children[port1-1])
+	if (hdev->children[port1-1]) {
+#ifdef CONFIG_SANDPOINT
+		char pic_command[4] = {QNAP_PIC_USB_LED_ON, QNAP_PIC_USB_LED_OFF, QNAP_PIC_USB_LED_ON, QNAP_PIC_USB_LED_OFF};
+
+		// kevin: we may add usb disconnection signal here...
+		qnap_pic_send_command(pic_command, 4);
+#endif
 		usb_disconnect(&hdev->children[port1-1]);
+	}
 	clear_bit(port1, hub->change_bits);
 
 #ifdef	CONFIG_USB_OTG
@@ -2471,6 +2482,9 @@
 
 	for (i = 0; i < SET_CONFIG_TRIES; i++) {
 		struct usb_device *udev;
+#ifdef CONFIG_SANDPOINT
+		char pic_command[4] = {QNAP_PIC_USB_LED_ON, QNAP_PIC_USB_LED_OFF, QNAP_PIC_USB_LED_ON, QNAP_PIC_USB_LED_OFF};
+#endif
 
 		/* reallocate for each attempt, since references
 		 * to the previous one can escape in various ways
@@ -2497,7 +2511,10 @@
 		status = hub_port_init(hub, udev, port1, i);
 		if (status < 0)
 			goto loop;
-
+#ifdef CONFIG_SANDPOINT
+		// kevin: we may add usb connection signal here...
+		qnap_pic_send_command(pic_command, 4);
+#endif
 		/* consecutive bus-powered hubs aren't reliable; they can
 		 * violate the voltage drop budget.  if the new child has
 		 * a "powered" LED, users should notice we didn't enable it
