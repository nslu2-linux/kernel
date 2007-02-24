--- linux-2.6.12.3/drivers/scsi/libata-core.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/libata-core.c	2007-02-17 14:10:18.000000000 -0500
@@ -49,6 +49,14 @@
 
 #include "libata.h"
 
+/* Ricky added */
+int process=0;
+int dev_exist[MAX_ENTRY];
+int status_changed;
+struct proc_dir_entry *sata_proc;
+/* End */
+
+
 static unsigned int ata_busy_sleep (struct ata_port *ap,
 				    unsigned long tmout_pat,
 			    	    unsigned long tmout);
@@ -64,6 +72,7 @@
 
 static unsigned int ata_unique_id = 1;
 static struct workqueue_struct *ata_wq;
+static struct workqueue_struct *ata_hotplug_wq;
 
 MODULE_AUTHOR("Jeff Garzik");
 MODULE_DESCRIPTION("Library module for ATA devices");
@@ -443,6 +452,7 @@
 	return inb(ap->ioaddr.status_addr);
 }
 
+
 /**
  *	ata_check_status_mmio - Read device status reg & clear interrupt
  *	@ap: port where the device is
@@ -459,6 +469,7 @@
        	return readb((void __iomem *) ap->ioaddr.status_addr);
 }
 
+static void ata_hotplug_task(void *_data);
 
 /**
  *	ata_check_status - Read device status reg & clear interrupt
@@ -1133,6 +1144,9 @@
 		return;
 	}
 
+	/* wipe all flags; this might be a different drive on hotswap. */
+	dev->flags = 0;
+
 	if (ap->flags & (ATA_FLAG_SRST | ATA_FLAG_SATA_RESET))
 		using_edd = 0;
 	else
@@ -1310,7 +1324,8 @@
  *	Zero on success, non-zero on error.
  */
 
-static int ata_bus_probe(struct ata_port *ap)
+//static int ata_bus_probe(struct ata_port *ap)
+int ata_bus_probe(struct ata_port *ap)
 {
 	unsigned int i, found = 0;
 
@@ -1322,8 +1337,9 @@
 		ata_dev_identify(ap, i);
 		if (ata_dev_present(&ap->device[i])) {
 			found = 1;
-			if (ap->ops->dev_config)
+			if (ap->ops->dev_config){
 				ap->ops->dev_config(ap, &ap->device[i]);
+			}
 		}
 	}
 
@@ -1356,6 +1372,22 @@
 void ata_port_probe(struct ata_port *ap)
 {
 	ap->flags &= ~ATA_FLAG_PORT_DISABLED;
+//ricky added
+//        dev_exist[ap->host->host_no]=1;
+//        status_changed=1;
+/*        sata_proc = create_proc_read_entry( SATA_PROC,0,NULL,read_proc,NULL);
+        if(sata_proc==NULL){
+                printk("create proc entry failed!\n");
+                goto error_register;
+        }
+        else{
+                sata_proc->write_proc=ap->ops->proc_gen_write;
+        }
+*/        return;
+//error_register:
+//        remove_proc_entry(SATA_PROC,NULL);
+        return;
+//End
 }
 
 /**
@@ -2109,9 +2141,7 @@
 
 static void ata_sg_clean(struct ata_queued_cmd *qc)
 {
-	struct ata_port *ap = qc->ap;
 	struct scatterlist *sg = qc->sg;
-	int dir = qc->dma_dir;
 
 	assert(qc->flags & ATA_QCFLAG_DMAMAP);
 	assert(sg != NULL);
@@ -3738,11 +3768,20 @@
 	ap->active_tag = ATA_TAG_POISON;
 	ap->last_ctl = 0xFF;
 
+        ap->orig_udma_mask = ent->udma_mask;
+
+//        ap->hotplug_timer.function = ata_hotplug_timer_func;
+//        ap->hotplug_timer.data = (unsigned int)ap;
+//        init_timer(&ap->hotplug_timer);
+
+	INIT_WORK(&ap->hotplug_task, ata_hotplug_task, ap);
 	INIT_WORK(&ap->packet_task, atapi_packet_task, ap);
 	INIT_WORK(&ap->pio_task, ata_pio_task, ap);
 
-	for (i = 0; i < ATA_MAX_DEVICES; i++)
+	for (i = 0; i < ATA_MAX_DEVICES; i++){
 		ap->device[i].devno = i;
+		ap->device[i].plug = HOT_NOOP;
+	}
 
 #ifdef ATA_IRQ_TRAP
 	ap->stats.unhandled_irq = 1;
@@ -3752,6 +3791,43 @@
 	memcpy(&ap->ioaddr, &ent->port[port_no], sizeof(struct ata_ioports));
 }
 
+void ata_scsi_hot_plug(struct ata_port *ap, unsigned int device)
+{
+       /* libata uses the 'id' or 'target' value */
+       scsi_add_device(ap->host, 0, device, 0);
+printk("SCSI add %d %d %d %d\n", ap->host->host_no, 0, device, 0);
+/*	if(ap->host->host_no==0)
+		send_message_to_app(QNAP_SATA_UP);
+	else if(ap->host->host_no==1)
+		send_message_to_app(QNAP_ESATA_UP);
+*/}
+
+void ata_scsi_hot_unplug(struct ata_port *ap, unsigned int device)
+{
+       /* libata uses the 'id' or 'target' value */
+       struct scsi_device *scd = scsi_device_lookup(ap->host, 0, device, 0);
+
+printk("SCSI remove %d %d %d %d\n", ap->host->host_no, 0, device, 0);
+       /* Make sure that we set this here, in case we aren't called as a
+        * result of sata_hot_unplug */
+       ap->device[device].class = ATA_DEV_NONE;
+
+       if (scd)  /* Set to cancel state to block further I/O */
+               scsi_device_set_state(scd, SDEV_CANCEL);
+
+       /* We might have a pending qc on I/O to a removed device. */
+       ata_check_kill_qc(ap, device);
+
+       if (scd) {
+               scsi_remove_device(scd);
+               scsi_device_put(scd);
+       }
+/*	if(ap->host->host_no==0)
+                send_message_to_app(QNAP_SATA_DOWN);
+        else if(ap->host->host_no==1)
+                send_message_to_app(QNAP_ESATA_DOWN);
+*/}
+
 /**
  *	ata_host_add - Attach low-level ATA driver to system
  *	@ent: Information provided by low-level driver
@@ -3898,6 +3974,11 @@
 			 * or the h/w is unplugged.
 			 */
 		}
+		else{
+			dev_exist[ap->host->host_no]=1;
+                        status_changed=1;
+		}
+			
 
 		rc = scsi_add_host(ap->host, dev);
 		if (rc) {
@@ -4338,7 +4419,114 @@
 	return (tmp == bits->val) ? 1 : 0;
 }
 #endif /* CONFIG_PCI */
+static void ata_hotplug_task(void *_data)
+{
+       struct ata_port *ap = (struct ata_port *)_data;
+       enum hotplug_states hotplug_todo[ATA_MAX_DEVICES];
+       unsigned long flags;
+       int i;
+
+       /* This function could have just one loop, but then we'd have to acquire
+        * the spin_lock multiple times.  Better to just have two loops.
+        */
+       spin_lock_irqsave(&ap->host_set->lock, flags);
+       for (i = 0; i < ATA_MAX_DEVICES; ++i) {
+               /* Make sure we don't modify while reading! */
+               hotplug_todo[i] = ap->device[i].plug;
+               ap->device[i].plug = HOT_NOOP;
+       }
+       spin_unlock_irqrestore(&ap->host_set->lock, flags);
+
+       for (i = 0; i < ATA_MAX_DEVICES; ++i) {
+               switch (hotplug_todo[i]) {
+                       case HOT_PLUG:
+                               printk("Got a plug request on port %d\n", ap->id);
+
+                               /* This might be necessary if we unplug and plug
+                                * in a drive within ATA_TMOUT_HOTPLUG / HZ
+                                * seconds... due to the debounce timer, one
+                                * event is generated.  Since the last event was
+                                * a plug, the unplug routine never gets called,
+                                * so we need to clean up the mess first.  If
+                                * there was never a drive here in the first
+                                * place, this will just do nothing.  Otherwise,
+                                * it basically prevents 'plug' from being
+                                * called twice with no unplug in between.
+                                */
+                               ata_scsi_hot_unplug(ap, i);
+
+                               /* This is necessary for some Seagate drives. */
+                               if (ap->flags & ATA_FLAG_SATA)
+                                       ap->flags |= ATA_FLAG_SATA_RESET;
+                               ap->udma_mask = ap->orig_udma_mask;
+
+                               if (!ata_bus_probe(ap)) /* Success */
+                                       ata_scsi_hot_plug(ap, i);
+				dev_exist[ap->host->host_no]=1;
+        			status_changed=1;
+				if(ap->host->host_no==0)
+					send_message_to_app(QNAP_SATA_UP);
+        			else if(ap->host->host_no==1)
+                			send_message_to_app(QNAP_ESATA_UP);
+                               break;
+                       case HOT_UNPLUG:
+                               printk("Got an unplug request on port %d\n", ap->id);
+
+                               ata_scsi_hot_unplug(ap, i);
+				dev_exist[ap->host->host_no]=0;
+        			status_changed=1;
+				if(ap->host->host_no==0)
+		                	send_message_to_app(QNAP_SATA_DOWN);
+        			else if(ap->host->host_no==1)
+          			      	send_message_to_app(QNAP_ESATA_DOWN);
+
+//				writel(0x7E, (void __iomem *) ap->ioaddr.status_addr);
+
+                               /* Fall through */
+                       case HOT_NOOP:
+                               /* No-op; do nothing */
+                               break;
+                       default:
+                               /* Should never happen */
+                               BUG();
+               }
+       }
+}
+
+void sata_hot_plug(struct ata_port *ap)
+{
+       if (ap->ops->hotplug_plug_janitor)
+               ap->ops->hotplug_plug_janitor(ap);
+
+       /* This line should be protected by a host_set->lock.  If we follow the
+        * call chain up from this function, it's called from within an
+        * interrupt handler.  Make sure that, when called, this function is
+        * protected in said handler.
+        */
+       ap->device[0].plug = HOT_PLUG;
+
+       queue_delayed_work(ata_hotplug_wq, &ap->hotplug_task, ATA_TMOUT_HOTPLUG);
+}
+
+void sata_hot_unplug(struct ata_port *ap)
+{
+       ap->device[0].class = ATA_DEV_NONE;
+
+       if (ap->ops->hotplug_unplug_janitor)
+               ap->ops->hotplug_unplug_janitor(ap);
+
+       /* See comment near similar line in sata_hot_plug function. */
+       ap->device[0].plug = HOT_UNPLUG;
+
+       queue_delayed_work(ata_hotplug_wq, &ap->hotplug_task, ATA_TMOUT_HOTPLUG);
+}
 
+void ata_pci_host_stop (struct ata_host_set *host_set)
+{
+        struct pci_dev *pdev = to_pci_dev(host_set->dev);
+
+        pci_iounmap(pdev, host_set->mmio_base);
+}
 
 static int __init ata_init(void)
 {
@@ -4346,6 +4534,12 @@
 	if (!ata_wq)
 		return -ENOMEM;
 
+	ata_hotplug_wq = create_workqueue("ata_plug");
+	if (!ata_hotplug_wq) {
+               destroy_workqueue(ata_wq);
+               return -ENOMEM;
+ 	}
+
 	printk(KERN_DEBUG "libata version " DRV_VERSION " loaded.\n");
 	return 0;
 }
@@ -4353,6 +4547,7 @@
 static void __exit ata_exit(void)
 {
 	destroy_workqueue(ata_wq);
+	destroy_workqueue(ata_hotplug_wq);
 }
 
 module_init(ata_init);
@@ -4407,10 +4602,21 @@
 EXPORT_SYMBOL_GPL(ata_dev_classify);
 EXPORT_SYMBOL_GPL(ata_dev_id_string);
 EXPORT_SYMBOL_GPL(ata_scsi_simulate);
+/* Ricky added 2005/08/07 --- QNAP*/
+EXPORT_SYMBOL_GPL(ata_bus_probe);
+//EXPORT_SYMBOL_GPL(ata_hotplug_timer_func);
+//EXPORT_SYMBOL_GPL(ata_scsi_handle_hotplug_func);
+EXPORT_SYMBOL_GPL(sata_hot_plug);
+EXPORT_SYMBOL_GPL(sata_hot_unplug);
+EXPORT_SYMBOL_GPL(ata_pci_host_stop);
+/* End */
 
 #ifdef CONFIG_PCI
 EXPORT_SYMBOL_GPL(pci_test_config_bits);
 EXPORT_SYMBOL_GPL(ata_pci_init_native_mode);
 EXPORT_SYMBOL_GPL(ata_pci_init_one);
 EXPORT_SYMBOL_GPL(ata_pci_remove_one);
+EXPORT_SYMBOL_GPL(sata_proc);
+EXPORT_SYMBOL_GPL(read_proc);
+EXPORT_SYMBOL_GPL(process);
 #endif /* CONFIG_PCI */
