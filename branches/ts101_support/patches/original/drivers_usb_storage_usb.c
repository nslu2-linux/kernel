--- linux-2.6.12.3/drivers/usb/storage/usb.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/usb/storage/usb.c	2007-02-17 14:10:14.000000000 -0500
@@ -90,7 +90,17 @@
 #ifdef CONFIG_USB_STORAGE_JUMPSHOT
 #include "jumpshot.h"
 #endif
-
+#ifdef CONFIG_SANDPOINT
+#include <linux/proc_fs.h>
+#include <asm/uaccess.h>
+#include <platforms/qnap.h>
+#include <platforms/qnap_pic_user.h>
+#include <linux/usb.h>
+#define QNAP_USB_PROC	"scsi/qnap_usbdev"
+#define QNAP_USB_PROC_HOTSWAP	"scsi/qnap_usbdev_hotswap"
+#define SET_MODULE_OWNER(dev) ((dev)->owner = THIS_MODULE)
+struct proc_dir_entry *qnap_usb_proc;
+#endif
 
 /* Some informational data */
 MODULE_AUTHOR("Matthew Dharm <mdharm-usb@one-eyed-alien.net>");
@@ -182,6 +192,10 @@
 	.flags = Flags, \
 }
 
+#ifdef CONFIG_SANEPOINT
+
+#endif
+
 static struct us_unusual_dev us_unusual_dev_list[] = {
 #	include "unusual_devs.h" 
 #	undef UNUSUAL_DEV
@@ -238,7 +252,51 @@
 	.disconnect =	storage_disconnect,
 	.id_table =	storage_usb_ids,
 };
+#ifdef CONFIG_SANDPOINT
+
+int usbdev_exist[MAX_USB_ENTRY];
+int usbdev_status_changed[MAX_USB_ENTRY];
+int usbdev_speedtype[MAX_USB_ENTRY];
+
+int qnap_usb_read_proc_hotswap(char *page, char **start, off_t offset, int count, int *eof, void *data){
+	char *s_buf = page;
+	int len=0;
+	int idx=0;
+	for(idx=0;idx<MAX_USB_ENTRY;idx++){
+                len += sprintf(s_buf+len, "%x ",usbdev_status_changed[idx]);
+        }
+        for(idx=0;idx<MAX_USB_ENTRY;idx++){
+                len += sprintf(s_buf+len, "%x ",usbdev_exist[idx]);
+        }
+        for(idx=0;idx<MAX_USB_ENTRY;idx++){
+                len += sprintf(s_buf+len, "%x ",usbdev_speedtype[idx]);
+        }
+        *eof=1;
+        return len;
+}
+
+int qnap_usb_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data){
+        char *s_buf = page;
+        int len=0;
+        int idx=0;
+	for(idx=0;idx<MAX_USB_ENTRY;idx++){
+		len += sprintf(s_buf+len, "%x ",usbdev_status_changed[idx]);
+	}
+        for(idx=0;idx<MAX_USB_ENTRY;idx++){
+                len += sprintf(s_buf+len, "%x ",usbdev_exist[idx]);
+        }
+	for(idx=0;idx<MAX_USB_ENTRY;idx++){
+		len += sprintf(s_buf+len, "%x ",usbdev_speedtype[idx]);
+	}
+	for(idx=0;idx<MAX_USB_ENTRY;idx++)
+		usbdev_status_changed[idx]=0;
+        *eof=1;
+        return len;
+}
 
+struct qnap_usb_mass_storage_device_info qnap_usb_front_port;
+EXPORT_SYMBOL(qnap_usb_front_port);
+#endif
 /*
  * fill_inquiry_response takes an unsigned char array (which must
  * be at least 36 characters) and populates the vendor name,
@@ -864,8 +922,108 @@
 	scsi_host_put(us_to_host(us));
 	complete_and_exit(&threads_gone, 0);
 }
+#ifdef CONFIG_SANDPOINT
 
+int qnap_get_ts101_usb_port_ex(struct device *dev, int *hub_port, int *speed){
+	int ret=-1;
+	if (dev && dev->bus && dev->bus->name && !strncmp(dev->bus->name, "usb", 3)) {
+		struct usb_interface *intf = to_usb_interface(dev);
+
+                if (intf) {
+                        struct usb_device *usbdev = interface_to_usbdev(intf);
+                        char buf[256];
+
+                        if (usbdev && usbdev->bus && usbdev->bus->bus_name) {
+                                memset(buf, 0, sizeof(buf));
+                                sprintf(buf, "%s-%s", usbdev->bus->bus_name, usbdev->devpath);
+                                if (!strncmp(buf, "0000:00:0e.1-1", 14) || !strncmp(buf, "0000:00:0e.2-2", 14))
+                                        ret = USB_MASS_STORAGE_DEVICE_FRONT_PORT;
+                                else if (!strncmp(buf, "0000:00:0e.1-2", 14) || !strncmp(buf, "0000:00:0e.2-4", 14))
+                                        ret = USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT;
+                                else if (!strncmp(buf, "0000:00:0e.0-2", 14) || !strncmp(buf, "0000:00:0e.2-3", 14))
+                                        ret = USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT;
+                        }
+                        if(*(buf+14) == '.')
+				*hub_port = (*(buf+15))-48;
+			else
+				*hub_port = 0;
+			switch(usbdev->speed){
+                        case USB_SPEED_HIGH:
+                        	*speed=USB_HIGH_SPEED;
+                        	break;
+                        case USB_SPEED_LOW:
+                                *speed=USB_LOW_SPEED;
+                                break;
+                        default:
+                                *speed=USB_LOW_SPEED;
+                                break;
+                        }
+                }	
+	}
+	return ret;
+}
+// return 0: front port, 1: backend upper port, 2: backend lower port
+int qnap_get_ts101_usb_port(struct device *dev)
+{
+    	int ret = -1;
 
+	if (dev && dev->bus && dev->bus->name && !strncmp(dev->bus->name, "usb", 3)) {
+#if 0
+		if (!strncmp(dev->bus_id, "3-1", 3) || !strncmp(dev->bus_id, "1-2", 3)) 
+            ret = USB_MASS_STORAGE_DEVICE_FRONT_PORT;
+        else if (!strncmp(dev->bus_id, "3-2", 3) || !strncmp(dev->bus_id, "1-4", 3)) 
+            ret = USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT;
+        else if (!strncmp(dev->bus_id, "2-2", 3) || !strncmp(dev->bus_id, "1-3", 3)) 
+            ret = UBS_MASS_STORAGE_DEVICE_BACK_LOWER_PORT;
+#else
+		struct usb_interface *intf = to_usb_interface(dev);
+	
+		if (intf) {
+	    		struct usb_device *usbdev = interface_to_usbdev(intf);
+	    		char buf[256];
+	
+		    	if (usbdev && usbdev->bus && usbdev->bus->bus_name) {
+		        	memset(buf, 0, sizeof(buf));
+		        	sprintf(buf, "%s-%s", usbdev->bus->bus_name, usbdev->devpath);
+		        	if (!strncmp(buf, "0000:00:0e.1-1", 14) || !strncmp(buf, "0000:00:0e.2-2", 14)) 
+		        		ret = USB_MASS_STORAGE_DEVICE_FRONT_PORT;
+		        	else if (!strncmp(buf, "0000:00:0e.1-2", 14) || !strncmp(buf, "0000:00:0e.2-4", 14)) 
+		            		ret = USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT;
+		        	else if (!strncmp(buf, "0000:00:0e.0-2", 14) || !strncmp(buf, "0000:00:0e.2-3", 14)) 
+		            		ret = USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT;
+	     		}
+		}
+    	}
+    	return ret;
+}
+int qnap_get_ts101_usb_speed(struct device *dev){
+	int ret = -1;
+	if (dev && dev->bus && dev->bus->name && !strncmp(dev->bus->name, "usb", 3)) {
+		struct usb_interface *intf = to_usb_interface(dev);
+		if (intf) {
+			struct usb_device *usbdev = interface_to_usbdev(intf);
+			
+			switch(usbdev->speed){
+			case USB_SPEED_HIGH:
+				ret=USB_HIGH_SPEED;
+				break;
+			case USB_SPEED_LOW:
+				ret=USB_LOW_SPEED;
+				break;
+			default:	
+				ret=USB_LOW_SPEED;
+				break;
+			}
+				
+		}
+	}
+	return ret;
+}
+EXPORT_SYMBOL(qnap_get_ts101_usb_speed);
+EXPORT_SYMBOL(qnap_get_ts101_usb_port);
+EXPORT_SYMBOL(qnap_get_ts101_usb_port_ex);
+#endif
+#endif
 /* Probe to see if we can drive a newly-connected USB device */
 static int storage_probe(struct usb_interface *intf,
 			 const struct usb_device_id *id)
@@ -874,7 +1032,13 @@
 	struct us_data *us;
 	const int id_index = id - storage_usb_ids; 
 	int result;
-
+#ifdef CONFIG_SANDPOINT
+	int hub_port = 0, speed=0;
+	int usb_port = qnap_get_ts101_usb_port_ex(&intf->dev, &hub_port, &speed);
+//	int usb_back_upper_port = ((qnap_get_ts101_usb_port(&intf->dev) == USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT) ? 1 : 0);
+//        int usb_back_lower_port = ((qnap_get_ts101_usb_port(&intf->dev) == USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT) ? 1 : 0);
+//	int speed = qnap_get_ts101_usb_speed(&intf->dev);
+#endif
 	US_DEBUGP("USB Mass Storage device detected\n");
 
 	/*
@@ -887,7 +1051,22 @@
 			"Unable to allocate the scsi host\n");
 		return -ENOMEM;
 	}
-
+#ifdef CONFIG_SANDPOINT
+	if (usb_port == USB_MASS_STORAGE_DEVICE_FRONT_PORT) {
+		if (qnap_usb_front_port.status == USB_MASS_STORAGE_DEVICE_DISCONNECTED || qnap_usb_front_port.cnt_of_device > 0) {
+			qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_CONNECTED;
+			qnap_usb_front_port.cnt_of_device++;
+			qnap_usb_front_port.size = 0;
+			memset(qnap_usb_front_port.name, 0, sizeof(qnap_usb_front_port.name));
+			qnap_usb_front_port.partno = 0;
+			printk("USB Mass Storage device connected to front port!\n");
+		}
+		else {
+			//??? something must be wrong...
+		}
+	}
+//	status_changed=1;
+#endif
 	us = host_to_us(host);
 	memset(us, 0, sizeof(struct us_data));
 	init_MUTEX(&(us->dev_semaphore));
@@ -965,11 +1144,14 @@
 
 	/* Wait for the thread to start */
 	wait_for_completion(&(us->notify));
-
 	return 0;
 
 	/* We come here if there are any problems */
 BadDevice:
+#ifdef CONFIG_SANDPOINT
+	if (usb_port == USB_MASS_STORAGE_DEVICE_FRONT_PORT)
+		qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_FAILED;
+#endif
 	US_DEBUGP("storage_probe() failed\n");
 	set_bit(US_FLIDX_DISCONNECTING, &us->flags);
 	usb_stor_release_resources(us);
@@ -982,9 +1164,112 @@
 static void storage_disconnect(struct usb_interface *intf)
 {
 	struct us_data *us = usb_get_intfdata(intf);
-
+#ifdef CONFIG_SANDPOINT
+	int hub_port=0, speed=0;
+	int usb_port = qnap_get_ts101_usb_port_ex(&intf->dev, &hub_port, &speed);
+#endif
 	US_DEBUGP("storage_disconnect() called\n");
-
+#ifdef CONFIG_SANDPOINT
+	if (usb_port == USB_MASS_STORAGE_DEVICE_FRONT_PORT) {
+		if (qnap_usb_front_port.status > USB_MASS_STORAGE_DEVICE_DISCONNECTED) {
+			qnap_usb_front_port.cnt_of_device--;
+//printk("The size of current plugged out disk = %d------------Ricky\n", qnap_usb_front_port.size);
+			if(qnap_usb_front_port.cnt_of_device==0)
+				qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_DISCONNECTED;
+
+			if(hub_port==0){
+				usbdev_exist[usb_port]=0;
+				usbdev_status_changed[usb_port] = USB_PORT;
+				usbdev_speedtype[usb_port]=0;
+			}
+			else if(hub_port==1){
+                        	usbdev_exist[usb_port]&=~USB_HUB_PORT1;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+				usbdev_speedtype[usb_port]&=~USB_HUB_PORT1;
+			}
+                       	else if(hub_port==2){
+                        	usbdev_exist[usb_port]&=~USB_HUB_PORT2;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+				usbdev_speedtype[usb_port]&=~USB_HUB_PORT2;
+			}
+                        else if(hub_port==3){
+                        	usbdev_exist[usb_port]&=~USB_HUB_PORT3;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+				usbdev_speedtype[usb_port]&=~USB_HUB_PORT3;
+			}
+                        else if(hub_port==4){
+                        	usbdev_exist[usb_port]&=~USB_HUB_PORT4;
+				usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+				usbdev_speedtype[usb_port]&=~USB_HUB_PORT4;
+			}
+			if(qnap_usb_front_port.size<10240)
+				qnap_usb_front_port.cnt_of_avail_device--;
+//			printk("Count of device = %d\n", qnap_usb_front_port.cnt_of_device);
+			send_message_to_app(QNAP_USB_FRONT_DOWN);
+			printk("USB Mass Storage device disconnected from front port!\n");
+		}
+		else {
+			//??? something must be wrong...
+		}
+	}
+	if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_UPPER_PORT){
+		if(hub_port==0){
+                        usbdev_exist[usb_port]=0;
+			usbdev_status_changed[usb_port] = USB_PORT;
+			usbdev_speedtype[usb_port]=0;
+                }
+                else if(hub_port==1){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT1;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT1;
+                }
+                else if(hub_port==2){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT2;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT2;
+                }
+                else if(hub_port==3){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT3;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT3;
+                }
+                else if(hub_port==4){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT4;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT4;
+                }
+		send_message_to_app(QNAP_USB_B_UPPER_DOWN);
+	}
+	if(usb_port==USB_MASS_STORAGE_DEVICE_BACK_LOWER_PORT){
+		if(hub_port==0){
+                        usbdev_exist[usb_port]=0;
+			usbdev_status_changed[usb_port] = USB_PORT;
+			usbdev_speedtype[usb_port]=0;
+                }
+                else if(hub_port==1){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT1;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT1;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT1;
+                }
+                else if(hub_port==2){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT2;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT2;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT2;
+                }
+                else if(hub_port==3){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT3;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT3;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT3;
+                }
+                else if(hub_port==4){
+                        usbdev_exist[usb_port]&=~USB_HUB_PORT4;
+                        usbdev_status_changed[usb_port] |= USB_HUB_PORT4;
+			usbdev_speedtype[usb_port]&=~USB_HUB_PORT4;
+                }
+		send_message_to_app(QNAP_USB_B_LOWER_DOWN);
+	}
+//	status_changed=1;
+#endif
 	/* Prevent new USB transfers, stop the current command, and
 	 * interrupt a SCSI-scan or device-reset delay */
 	set_bit(US_FLIDX_DISCONNECTING, &us->flags);
@@ -1017,11 +1302,32 @@
 	int retval;
 	printk(KERN_INFO "Initializing USB Mass Storage driver...\n");
 
-	/* register the driver, return usb_register return code if error */
+	/* register the driver, return usb_register retmisc_register(&aaaaaaurn code if error */
 	retval = usb_register(&usb_storage_driver);
-	if (retval == 0)
+	if (retval == 0) {
+#ifdef CONFIG_SANDPOINT
+		qnap_usb_front_port.status = USB_MASS_STORAGE_DEVICE_DISCONNECTED;
+		qnap_usb_front_port.port = USB_MASS_STORAGE_DEVICE_FRONT_PORT;
+#endif
 		printk(KERN_INFO "USB Mass Storage support registered.\n");
+	}
+#ifdef CONFIG_SANDPOINT
+	qnap_usb_proc = create_proc_read_entry(QNAP_USB_PROC,0,NULL,qnap_usb_read_proc,NULL);
+	qnap_usb_proc = create_proc_read_entry(QNAP_USB_PROC_HOTSWAP,0,NULL,qnap_usb_read_proc_hotswap,NULL);
+	if(qnap_usb_proc==NULL){
+                printk("create proc entry failed!\n");
+                goto error_register;
+        }
+        else{
+                qnap_usb_proc->write_proc=NULL;
+                printk("create proc entry succeed!\n");
+        }
+#endif
+	return retval;
 
+error_register:
+	        remove_proc_entry(QNAP_USB_PROC,NULL);
+		remove_proc_entry(QNAP_USB_PROC_HOTSWAP, NULL);
 	return retval;
 }
 
@@ -1045,6 +1351,8 @@
 		wait_for_completion(&threads_gone);
 		atomic_dec(&total_threads);
 	}
+	remove_proc_entry(QNAP_USB_PROC,NULL);
+	remove_proc_entry(QNAP_USB_PROC_HOTSWAP, NULL);
 }
 
 module_init(usb_stor_init);
