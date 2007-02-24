--- linux-2.6.12.3/drivers/scsi/sg.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/sg.c	2007-02-17 14:10:19.000000000 -0500
@@ -61,6 +61,10 @@
 
 #ifdef CONFIG_SCSI_PROC_FS
 #include <linux/proc_fs.h>
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap.h>
+#endif
+
 static char *sg_version_date = "20050328";
 
 static int sg_proc_init(void);
@@ -1439,7 +1443,16 @@
 	Sg_device *sdp;
 	unsigned long iflags;
 	void *old_sg_dev_arr = NULL;
-	int k, error;
+#ifdef CONFIG_SANDPOINT
+	int k=0, error, hub_port=0, speed=0;
+
+	int usb_port = qnap_get_ts101_usb_port_ex(scsidp->host->shost_gendev.parent, &hub_port, &speed);
+//        int usb_back_upper_port = ((qnap_get_ts101_usb_port(scsidp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT) ? 1 : 0);
+//        int usb_back_lower_port = ((qnap_get_ts101_usb_port(scsidp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT) ? 1 : 0);
+        int sata_port = qnap_get_ts101_sata_port(scsidp);
+#else
+	int k,error;
+#endif
 
 	sdp = kmalloc(sizeof(Sg_device), GFP_KERNEL);
 	if (!sdp) {
@@ -1465,11 +1478,31 @@
 		sg_dev_max = tmp_dev_max;
 	}
 
-	for (k = 0; k < sg_dev_max; k++)
-		if (!sg_dev_arr[k])
-			break;
-	if (unlikely(k >= SG_MAX_DEVS))
-		goto overflow;
+#ifdef CONFIG_SANDPOINT
+        if(sata_port==SATA1)
+                k=0;
+        else if(sata_port==SATA2)
+                k=1;
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_FRONT_PORT)
+                hub_port==0?(k=3):(k=3+(hub_port-1));
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT)
+                hub_port==0?(k=10):(k=10+(hub_port-1));
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT)
+                hub_port==0?(k=17):(k=17+(hub_port-1));
+        else {
+#else
+		for (k = 0; k < sg_dev_max; k++)
+			if (!sg_dev_arr[k])
+				break;
+#endif
+#ifdef CONFIG_SANDPOINT
+        }
+        for(;k < sg_dev_max;k++)
+                if (!sg_dev_arr[k])
+                        break;
+#endif
+		if (unlikely(k >= SG_MAX_DEVS))
+			goto overflow;
 
 	memset(sdp, 0, sizeof(*sdp));
 	SCSI_LOG_TIMEOUT(3, printk("sg_alloc: dev=%d \n", k));
