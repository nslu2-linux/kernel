--- linux-2.6.12.3/drivers/usb/class/usblp.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/usb/class/usblp.c	2007-02-17 14:10:14.000000000 -0500
@@ -126,6 +126,10 @@
  */
 #define STATUS_BUF_SIZE		8
 
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap_pic_user.h>
+#endif
+
 struct usblp {
 	struct usb_device 	*dev;			/* USB device */
 	struct semaphore	sem;			/* locks this struct, especially "dev" */
@@ -403,6 +407,9 @@
 	usb_free_urb(usblp->writeurb);
 	usb_free_urb(usblp->readurb);
 	kfree (usblp);
+#ifdef CONFIG_SANDPOINT
+	send_message_to_app(QNAP_USB_PRINTER_DOWN);
+#endif
 }
 
 static void usblp_unlink_urbs(struct usblp *usblp)
@@ -952,6 +959,9 @@
 		goto abort_intfdata;
 	}
 	usblp->minor = intf->minor;
+#ifdef CONFIG_SANDPOINT
+	send_message_to_app(QNAP_USB_PRINTER_UP);
+#endif
 
 	return 0;
 
