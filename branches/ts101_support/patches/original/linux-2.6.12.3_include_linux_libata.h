--- linux-2.6.12.3/include/linux/libata.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/linux/libata.h	2007-02-17 14:10:09.000000000 -0500
@@ -29,6 +29,13 @@
 #include <asm/io.h>
 #include <linux/ata.h>
 #include <linux/workqueue.h>
+//Ricky
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap.h>
+#include <platforms/qnap_pic_user.h>
+#include <linux/proc_fs.h>
+#endif
+//End
 
 /*
  * compile-time options
@@ -68,6 +75,11 @@
 
 /* defines only for the constants which don't work well as enums */
 #define ATA_TAG_POISON		0xfafbfcfdU
+//Ricky
+#define MAX_ENTRY       2
+#define SATA_PROC            "scsi/SiI3512"
+extern int process;
+//End
 
 /* move to PCI layer? */
 static inline struct device *pci_dev_to_dev(struct pci_dev *pdev)
@@ -126,6 +138,10 @@
 	ATA_TMOUT_BOOT_QUICK	= 7 * HZ,	/* hueristic */
 	ATA_TMOUT_CDB		= 30 * HZ,
 	ATA_TMOUT_CDB_QUICK	= 5 * HZ,
+        /* Ricky added 2005/08/07 --- QNAP */
+        ATA_TMOUT_HOTSWAP       = HZ/2,
+	ATA_TMOUT_HOTPLUG	= HZ/2,
+        /* End */
 
 	/* ATA bus states */
 	BUS_UNKNOWN		= 0,
@@ -150,6 +166,18 @@
 	ATA_SHIFT_UDMA		= 0,
 	ATA_SHIFT_MWDMA		= 8,
 	ATA_SHIFT_PIO		= 11,
+
+        /* Ricky added 2005/08/07 --- QNAP */
+        PLUG                    = 1,
+        UNPLUG                  = 0,
+        /* End */
+
+};
+
+enum hotplug_states {
+       HOT_NOOP,
+       HOT_PLUG,
+       HOT_UNPLUG,
 };
 
 enum pio_task_states {
@@ -194,7 +222,7 @@
 	struct list_head	node;
 	struct device 		*dev;
 	struct ata_port_operations	*port_ops;
-	Scsi_Host_Template	*sht;
+	Scsi_Host_Template 	*sht;
 	struct ata_ioports	port[ATA_MAX_PORTS];
 	unsigned int		n_ports;
 	unsigned int		hard_port_no;
@@ -278,6 +306,8 @@
 	u8			xfer_protocol;	/* taskfile xfer protocol */
 	u8			read_cmd;	/* opcode to use on read */
 	u8			write_cmd;	/* opcode to use on write */
+	
+	enum hotplug_states     plug;
 };
 
 struct ata_port {
@@ -309,8 +339,13 @@
 	unsigned long		qactive;
 	unsigned int		active_tag;
 
+        /* Ricky added 2005/08/07 --- QNAP */
+        struct timer_list       hotplug_timer;
+        /* End */
+
 	struct ata_host_stats	stats;
 	struct ata_host_set	*host_set;
+	struct work_struct      hotplug_task;
 
 	struct work_struct	packet_task;
 
@@ -319,6 +354,12 @@
 	unsigned long		pio_task_timeout;
 
 	void			*private_data;
+
+        /* Ricky added 2005/08/07 --- QNAP */
+        unsigned int            orig_udma_mask;
+        unsigned int            status;
+        /* End */
+
 };
 
 struct ata_port_operations {
@@ -365,10 +406,15 @@
 
 	void (*bmdma_stop) (struct ata_port *ap);
 	u8   (*bmdma_status) (struct ata_port *ap);
+        int (*proc_gen_write)(struct file * file, const char * buf,
+                                unsigned long length, void *data);
+
+	void (*hotplug_plug_janitor) (struct ata_port *ap);
+	void (*hotplug_unplug_janitor) (struct ata_port *ap);
 };
 
 struct ata_port_info {
-	Scsi_Host_Template	*sht;
+	struct scsi_host_template	*sht;
 	unsigned long		host_flags;
 	unsigned long		pio_mask;
 	unsigned long		mwdma_mask;
@@ -389,12 +435,22 @@
 extern void ata_pci_remove_one (struct pci_dev *pdev);
 #endif /* CONFIG_PCI */
 extern int ata_device_add(struct ata_probe_ent *ent);
-extern int ata_scsi_detect(Scsi_Host_Template *sht);
+extern int ata_scsi_detect(struct scsi_host_template *sht);
 extern int ata_scsi_ioctl(struct scsi_device *dev, int cmd, void __user *arg);
 extern int ata_scsi_queuecmd(struct scsi_cmnd *cmd, void (*done)(struct scsi_cmnd *));
 extern int ata_scsi_error(struct Scsi_Host *host);
 extern int ata_scsi_release(struct Scsi_Host *host);
 extern unsigned int ata_host_intr(struct ata_port *ap, struct ata_queued_cmd *qc);
+
+//Ricky added 2005/08/09
+//extern void ata_hotplug_timer_func(unsigned long data);
+//extern void ata_scsi_handle_hotplug_func(struct ata_port *ap);
+extern int ata_bus_probe(struct ata_port *ap);
+extern void sata_hot_plug(struct ata_port *ap);
+extern void sata_hot_unplug(struct ata_port *ap);
+
+//End
+
 /*
  * Default driver ops implementations
  */
@@ -467,6 +523,7 @@
 	return ap->ops->check_status(ap);
 }
 
+extern void ata_pci_host_stop (struct ata_host_set *host_set);
 
 /**
  *	ata_pause - Flush writes and pause 400 nanoseconds.
