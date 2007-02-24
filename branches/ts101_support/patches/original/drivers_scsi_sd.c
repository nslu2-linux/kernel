--- linux-2.6.12.3/drivers/scsi/sd.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/sd.c	2007-02-17 14:10:18.000000000 -0500
@@ -50,6 +50,11 @@
 #include <linux/kref.h>
 #include <linux/delay.h>
 #include <asm/uaccess.h>
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap.h>
+#include <platforms/qnap_pic.h>
+#include <platforms/qnap_pic_user.h>
+#endif
 
 #include <scsi/scsi.h>
 #include <scsi/scsi_cmnd.h>
@@ -89,6 +94,11 @@
 #define SD_MAX_RETRIES		5
 #define SD_PASSTHROUGH_RETRIES	1
 
+#ifdef CONFIG_SANDPOINT
+static int sd_dev_arr[26];
+extern int usbdev_exist[MAX_USB_ENTRY];
+extern int usbdev_status_changed[MAX_USB_ENTRY];
+#endif
 static void scsi_disk_release(struct kref *kref);
 
 struct scsi_disk {
@@ -99,6 +109,11 @@
 	unsigned int	openers;	/* protected by BKL for now, yuck */
 	sector_t	capacity;	/* size in 512-byte sectors */
 	u32		index;
+/*
+#ifdef CONFIG_SANDPOINT
+	u32		idr_index;
+#endif
+*/
 	u8		media_present;
 	u8		write_prot;
 	unsigned	WCE : 1;	/* state of disk WCE bit */
@@ -203,7 +218,6 @@
 static void scsi_disk_put(struct scsi_disk *sdkp)
 {
 	struct scsi_device *sdev = sdkp->device;
-
 	down(&sd_ref_sem);
 	kref_put(&sdkp->kref, scsi_disk_release);
 	scsi_device_put(sdev);
@@ -1115,6 +1129,9 @@
 	int longrc = 0;
 	struct scsi_sense_hdr sshdr;
 	int sense_valid = 0;
+#ifdef CONFIG_SANDPOINT
+	int usb_front_port = ((qnap_get_ts101_usb_port(sdp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_FRONT_PORT) ? 1 : 0);
+#endif
 
 repeat:
 	retries = 3;
@@ -1282,6 +1299,10 @@
 		       "%llu %d-byte hdwr sectors (%llu MB)\n",
 		       diskname, (unsigned long long)sdkp->capacity,
 		       hard_sector, (unsigned long long)mb);
+#ifdef CONFIG_SANDPOINT
+		if (usb_front_port == 1)
+			qnap_usb_front_port.size = mb;
+#endif
 	}
 
 	/* Rescale capacity to 512-byte units */
@@ -1529,8 +1550,20 @@
 	struct scsi_device *sdp = to_scsi_device(dev);
 	struct scsi_disk *sdkp;
 	struct gendisk *gd;
+#ifdef CONFIG_SANDPOINT
+	u32 index=0;
+	int hub_port=0, speed=0;
+//idr_index;
+#else
 	u32 index;
+#endif
 	int error;
+#ifdef CONFIG_SANDPOINT
+	int usb_port = qnap_get_ts101_usb_port_ex(sdp->host->shost_gendev.parent, &hub_port, &speed);
+//	int usb_back_upper_port = ((qnap_get_ts101_usb_port(sdp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT) ? 1 : 0);
+//	int usb_back_lower_port = ((qnap_get_ts101_usb_port(sdp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT) ? 1 : 0);
+	int sata_port = qnap_get_ts101_sata_port(sdp);
+#endif
 
 	error = -ENODEV;
 	if ((sdp->type != TYPE_DISK) && (sdp->type != TYPE_MOD))
@@ -1551,21 +1584,55 @@
 	if (!gd)
 		goto out_free;
 
-	if (!idr_pre_get(&sd_index_idr, GFP_KERNEL))
-		goto out_put;
+//	if (!idr_pre_get(&sd_index_idr, GFP_KERNEL))
+//		goto out_put;
 
 	spin_lock(&sd_index_lock);
+/*
+#ifdef CONFIG_SANDPOINT
+	error = idr_get_new(&sd_index_idr, NULL, &idr_index);
+#else
 	error = idr_get_new(&sd_index_idr, NULL, &index);
+#endif
+*/
 	spin_unlock(&sd_index_lock);
 
 	if (index >= SD_MAX_DISKS)
 		error = -EBUSY;
-	if (error)
-		goto out_put;
+//	if (error)
+//		goto out_put;
+#ifdef CONFIG_SANDPOINT
+
+//printk("usb_port : %d, hub_port : %d--------------------Ricky\n", usb_port, hub_port);
+
+        if(sata_port==SATA1)
+                index=0;
+        else if(sata_port==SATA2)
+                index=1;
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_FRONT_PORT){
+                hub_port==0?(index=3):(index=3+(hub_port-1));
+	}
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT){
+                hub_port==0?(index=10):(index=10+(hub_port-1));
+	}
+        else if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT){
+                hub_port==0?(index=17):(index=17+(hub_port-1));
+	}
+	for(;index<26;index++)
+		if(!sd_dev_arr[index]){
+			sd_dev_arr[index]=1;
+			break;
+		}
+#endif
 
 	sdkp->device = sdp;
 	sdkp->driver = &sd_template;
 	sdkp->disk = gd;
+/*
+#ifdef CONFIG_SANDPOINT
+	sdkp->idr_index = idr_index;
+#endif
+*/
 	sdkp->index = index;
 	sdkp->openers = 0;
 
@@ -1580,7 +1647,6 @@
 	gd->first_minor = ((index & 0xf) << 4) | (index & 0xfff00);
 	gd->minors = 16;
 	gd->fops = &sd_fops;
-
 	if (index < 26) {
 		sprintf(gd->disk_name, "sd%c", 'a' + index % 26);
 	} else if (index < (26 + 1) * 26) {
@@ -1613,7 +1679,156 @@
 	       "id %d, lun %d\n", sdp->removable ? "removable " : "",
 	       gd->disk_name, sdp->host->host_no, sdp->channel,
 	       sdp->id, sdp->lun);
+#ifdef CONFIG_SANDPOINT
+	if (usb_port == USB_MASS_STORAGE_DEVICE_FRONT_PORT) {
+		if (qnap_usb_front_port.status == USB_MASS_STORAGE_DEVICE_CONNECTED || qnap_usb_front_port.cnt_of_device > 0) {
+			int i;
+			struct hd_struct *part;
 
+			
+			for (i = gd->first_minor; i < gd->minors && (part = gd->part[i]) != NULL; i++)
+				qnap_usb_front_port.partno++;
+			qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_READY;
+			strncpy(qnap_usb_front_port.name, gd->disk_name, sizeof(qnap_usb_front_port.name) - 1);
+//			printk("The USB Mass Storage device in front port is ready to use!\n");
+			//printk("%s with total %d mb and %d partition%s\n", qnap_usb_front_port.name, qnap_usb_front_port.size, qnap_usb_front_port.partno, (qnap_usb_front_port.partno > 1 ? "s" : ""));
+
+//printk("Cnt of device = %d--------------------------------------Ricky\n", qnap_usb_front_port.cnt_of_device);
+//printk("Size of latest disk = %d--------------------------Ricky\n", qnap_usb_front_port.size);
+//printk("Speed = %d\n", speed);
+
+			// 2005.12.06, Johnson Cheng
+			// Shouldn't do it here. Move it to picd.
+			/*if (qnap_usb_front_port.size < 10240) {
+				char pic_command = QNAP_PIC_USB_LED_ON;
+				qnap_pic_send_command(&pic_command, 1);
+				//usb_button_enabled = 1;
+			}
+			else
+				usb_button_enabled = 0;*/
+				
+			if(hub_port==0){
+				usbdev_exist[usb_port]=USB_PORT;
+				usbdev_status_changed[usb_port]=USB_PORT;
+				if(speed==USB_HIGH_SPEED)
+					usbdev_speedtype[usb_port]=USB_PORT;
+			}
+			else if(hub_port==1){
+				usbdev_exist[usb_port]|=USB_HUB_PORT1;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+				if(speed==USB_HIGH_SPEED)
+					usbdev_speedtype[usb_port] |= USB_HUB_PORT1;
+			}
+			else if(hub_port==2){
+				usbdev_exist[usb_port]|=USB_HUB_PORT2;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+				if(speed==USB_HIGH_SPEED)
+					usbdev_speedtype[usb_port] |= USB_HUB_PORT2;
+			}
+			else if(hub_port==3){
+				usbdev_exist[usb_port]|=USB_HUB_PORT3;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+				if(speed==USB_HIGH_SPEED)
+					usbdev_speedtype[usb_port] |= USB_HUB_PORT3;
+			}
+			else if(hub_port==4){
+				usbdev_exist[usb_port]|=USB_HUB_PORT4;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+				if(speed==USB_HIGH_SPEED)
+					usbdev_speedtype[usb_port] |= USB_HUB_PORT4;
+			}
+			qnap_usb_front_port.cnt_of_avail_device++;
+			if(hub_port==0){
+				if(index==3||index==10||index==17)
+					send_message_to_app(QNAP_USB_FRONT_UP);
+			}
+			else
+				send_message_to_app(QNAP_USB_FRONT_UP);
+			
+//printk("Cnt of available devices = %d--------------------------------------Ricky\n", qnap_usb_front_port.cnt_of_avail_device);
+		}
+		else {
+			//??? something must be wrong...
+		}
+	}
+	if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT){
+		if(hub_port==0){
+                	usbdev_exist[usb_port]=USB_PORT;
+			usbdev_status_changed[usb_port] =USB_PORT;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port]=USB_PORT;
+                }
+                else if(hub_port==1){
+                	usbdev_exist[usb_port]|=USB_HUB_PORT1;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT1;
+                }
+                else if(hub_port==2){
+                	usbdev_exist[usb_port]|=USB_HUB_PORT2;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT2;
+                }
+                else if(hub_port==3){
+                	usbdev_exist[usb_port]|=USB_HUB_PORT3;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT3;
+               	}
+                else if(hub_port==4){
+                	usbdev_exist[usb_port]|=USB_HUB_PORT4;
+                	usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT4;
+                }
+		if(hub_port==0){
+             		if(index==3||index==10||index==17)
+                		send_message_to_app(QNAP_USB_B_UPPER_UP);
+		}
+		else
+			send_message_to_app(QNAP_USB_B_UPPER_UP);
+        }
+        if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT){
+		if(hub_port==0){
+                        usbdev_exist[usb_port]=USB_PORT;
+			usbdev_status_changed[usb_port] =USB_PORT;
+
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port]=USB_PORT;
+                }
+                else if(hub_port==1){
+                        usbdev_exist[usb_port]|=USB_HUB_PORT1;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT1;
+                }
+                else if(hub_port==2){
+                        usbdev_exist[usb_port]|=USB_HUB_PORT2;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT2;
+                }
+                else if(hub_port==3){
+                        usbdev_exist[usb_port]|=USB_HUB_PORT3;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT3;
+                }
+                else if(hub_port==4){
+                        usbdev_exist[usb_port]|=USB_HUB_PORT4;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+			if(speed==USB_HIGH_SPEED)
+				usbdev_speedtype[usb_port] |= USB_HUB_PORT4;
+                }
+		if(hub_port==0){
+                        if(index==3||index==10||index==17)
+                		send_message_to_app(QNAP_USB_B_LOWER_UP);
+		}
+		else
+			send_message_to_app(QNAP_USB_B_LOWER_UP);
+        }
+#endif
 	return 0;
 
 out_put:
@@ -1621,6 +1836,10 @@
 out_free:
 	kfree(sdkp);
 out:
+#ifdef CONFIG_SANDPOINT
+	if (usb_port == USB_MASS_STORAGE_DEVICE_FRONT_PORT)
+		qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_FAILED;
+#endif
 	return error;
 }
 
@@ -1638,7 +1857,10 @@
 static int sd_remove(struct device *dev)
 {
 	struct scsi_disk *sdkp = dev_get_drvdata(dev);
-
+//#ifdef CONFIG_SANDPOINT
+//	struct scsi_device *sdp = sdkp->device;
+//	int usb_front_port = ((qnap_get_ts101_usb_port(sdp->host->shost_gendev.parent) == USB_MASS_STORAGE_DEVICE_FRONT_PORT) ? 1 : 0);
+//#endif
 	del_gendisk(sdkp->disk);
 	sd_shutdown(dev);
 	down(&sd_ref_sem);
@@ -1663,7 +1885,12 @@
 	struct gendisk *disk = sdkp->disk;
 	
 	spin_lock(&sd_index_lock);
+#ifdef CONFIG_SANDPOINT
+//	idr_remove(&sd_index_idr, sdkp->idr_index);
+	sd_dev_arr[sdkp->index]=0;
+#else
 	idr_remove(&sd_index_idr, sdkp->index);
+#endif
 	spin_unlock(&sd_index_lock);
 
 	disk->private_data = NULL;
