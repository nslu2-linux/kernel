--- linux-2.6.12.3/drivers/scsi/sata_sil.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/sata_sil.c	2007-02-17 14:10:19.000000000 -0500
@@ -5,24 +5,101 @@
  *  		    Please ALWAYS copy linux-ide@vger.kernel.org
  *		    on emails.
  *
- *  Copyright 2003 Red Hat, Inc.
+ *  Copyright 2003-2005 Red Hat, Inc.
  *  Copyright 2003 Benjamin Herrenschmidt
  *
- *  The contents of this file are subject to the Open
- *  Software License version 1.1 that can be found at
- *  http://www.opensource.org/licenses/osl-1.1.txt and is included herein
- *  by reference.
- *
- *  Alternatively, the contents of this file may be used under the terms
- *  of the GNU General Public License version 2 (the "GPL") as distributed
- *  in the kernel source COPYING file, in which case the provisions of
- *  the GPL are applicable instead of the above.  If you wish to allow
- *  the use of your version of this file only under the terms of the
- *  GPL and not to allow others to use your version of this file under
- *  the OSL, indicate your decision by deleting the provisions above and
- *  replace them with the notice and other provisions required by the GPL.
- *  If you do not delete the provisions above, a recipient may use your
- *  version of this file under either the OSL or the GPL.
+ *
+ *  This program is free software; you can redistribute it and/or modify
+ *  it under the terms of the GNU General Public License as published by
+ *  the Free Software Foundation; either version 2, or (at your option)
+ *  any later version.
+ *
+ *  This program is distributed in the hope that it will be useful,
+ *  but WITHOUT ANY WARRANTY; without even the implied warranty of
+ *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
+ *  GNU General Public License for more details.
+ *
+ *  You should have received a copy of the GNU General Public License
+ *  along with this program; see the file COPYING.  If not, write to
+ *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
+ *
+ *
+ *  libata documentation is available via 'make {ps|pdf}docs',
+ *  as Documentation/DocBook/libata.*
+ *
+ *  Documentation for SiI 3112:
+ *  http://gkernel.sourceforge.net/specs/sii/3112A_SiI-DS-0095-B2.pdf.bz2
+ *  Documentation for SiI 3114:
+ *  http://gkernel.sourceforge.net/specs/sii/sii-0680a-v1.31.pdf.bz2
+ *
+ *  Other errata and documentation available under NDA.
+ *  Currently implemented workarounds are...
+ *
+ *  1. Mod15Write errata workaround
+ *
+ *  Errata:
+ *    The Mod15Write bug is caused by the SiI3112 controller splitting
+ *    data FIS in the middle of a 512 byte block when transferring
+ *    data equal to or larger than 8k (not prohibited by the standard
+ *    but still a bit peculiar) and some Seagate drives locking up on
+ *    receiving such data FIS.
+ *
+ *  Workaround:
+ *    This is worked around by limiting max_sectors to 15 such that
+ *    write data is always smaller than 8k.  This slows down IO
+ *    severely.
+ *
+ *    This workaround is applied during sil_dev_config() and the
+ *    affected drives are marked with SIL_QUIRK_MOD15WRITE in
+ *    sil_blacklist.
+ *
+ *    The workaround slows down everything considerably and tends to
+ *    hide other problems when applied.  Due to lack of information,
+ *    this workaround had been applied when some drives show similar
+ *    symptoms (e.g. errata #3).  It seems that the blacklist
+ *    currently contains some number of unaffected drives.
+ *
+ *    As we don't know exactly which drives are affected, slow_down
+ *    module parameter is implemented which turns on this workaround
+ *    manually.
+ *
+ *  2. Maxtor udma5 max errata
+ *
+ *  Errata:
+ *    For certain Maxtor devices, we must not program the drive be
+ *    beyond udma5.
+ *
+ *  Workaround:
+ *    Limit max transfer mode to udma5.
+ *
+ *    This workaround is applied during sli_dev_config() and the
+ *    affected drives are marked with SIL_QUIRK_UDMA5MAX in
+ *    sil_blacklist.
+ *
+ *    Note that udma5 is 100Mbytes/s which is more than sufficient, so
+ *    this workaround doesn't really affect performance.
+ *
+ *  3. SiI 3512/3114 R_ERR on DMA activate FIS errta workaround
+ *
+ *  Errata:
+ *    During DMA write operations with a data length greater than 8
+ *    Kbytes, a PRD entry fetch that occurs at the same time that a
+ *    DMA Activate FIS is received may cause the controller to falsely
+ *    indicate that the DMA Activate FIS has an illegal FIS Type.
+ *    This may cause the controller to send an R_EER in response to
+ *    the DMA Active FIS.
+ *
+ *  Workaround:
+ *    By configuring bit[1:0] of the SFISCfg register to accept FIS
+ *    types other than the standard SATA defined FIS types, the
+ *    controller is prevented from falsely setting the illegal FIS
+ *    Type indicator, thus preventing the improper RERR response.  The
+ *    default value of the SFISCfg register is 0x1040_1555.  To
+ *    implement this workaround, the SFISCfg register should be set to
+ *    a value of 0x1040_1554.
+ *
+ *    This workaround is applied during controller initialization in
+ *    sil_init_one().
  *
  */
 
@@ -36,13 +113,41 @@
 #include "scsi.h"
 #include <scsi/scsi_host.h>
 #include <linux/libata.h>
+#include "libata.h"
+
+//#define DRV_NAME	"sata_sil"
+//#define DRV_VERSION	"0.9"
+
+/*  Ricky added 2005/08/07 ---QNAP */
+#include <asm/uaccess.h>
+#define PCI_BUS_REG_MASTER2     0x10
+#define SATA_INT_MASK_0         1<<4
+#define SATA_INT_MASK_1         1<<6
+#define SATA_PLUG               0x11
+#define SATA_CLEAR              0xFFFF0000
+#define SATA_ENABLE_INT         1<<16
+#define SATA_PHY_CHANGE         1<<16
+extern int dev_exist[MAX_ENTRY];
+extern int status_changed;
+extern struct proc_dir_entry *sata_proc;
+extern int read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data);
+
+struct qnap_ap {
+        struct ata_port *ap[MAX_ENTRY];
+        int dirty[MAX_ENTRY];
+};
+
+struct qnap_ap Qnap_ap;
 
-#define DRV_NAME	"sata_sil"
-#define DRV_VERSION	"0.9"
+/* End */
 
 enum {
-	sil_3112		= 0,
-	sil_3114		= 1,
+	SIL_FLAG_RERR_ON_DMA_ACT = (1 << 29),
+	SIL_FLAG_MOD15WRITE     = (1 << 30),
+
+        sil_3112                = 0,
+        sil_3112_m15w           = 1,
+        sil_3114                = 2,
 
 	SIL_FIFO_R0		= 0x40,
 	SIL_FIFO_W0		= 0x41,
@@ -75,14 +180,26 @@
 static void sil_scr_write (struct ata_port *ap, unsigned int sc_reg, u32 val);
 static void sil_post_set_mode (struct ata_port *ap);
 
+/********* Ricky added 2005/08/07 --- QNAP *********/
+struct ata_port *global_ap;
+
+static void sil_pci_remove_one(struct pci_dev *pdev);
+static irqreturn_t sil_interrupt (int irq, void *dev_instance, struct pt_regs *regs);
+static void sil_host_init(struct ata_probe_ent *);
+static void sil_phy_reset(struct ata_port *ap);
+int sil_proc_gen_write(struct file * file, const char * buf,
+                        unsigned long length, void *data);
+/********************* End *************************/
+
+
 static struct pci_device_id sil_pci_tbl[] = {
-	{ 0x1095, 0x3112, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
-	{ 0x1095, 0x0240, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
-	{ 0x1095, 0x3512, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
+	{ 0x1095, 0x3112, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
+	{ 0x1095, 0x0240, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
+	{ 0x1095, 0x3512, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
 	{ 0x1095, 0x3114, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3114 },
-	{ 0x1002, 0x436e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
-	{ 0x1002, 0x4379, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
-	{ 0x1002, 0x437a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112 },
+	{ 0x1002, 0x436e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
+	{ 0x1002, 0x4379, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
+	{ 0x1002, 0x437a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, sil_3112_m15w },
 	{ }	/* terminate list */
 };
 
@@ -115,7 +232,9 @@
 	.name			= DRV_NAME,
 	.id_table		= sil_pci_tbl,
 	.probe			= sil_init_one,
-	.remove			= ata_pci_remove_one,
+//Ricky modified
+//      .remove                 = ata_pci_remove_one,
+	.remove			= sil_pci_remove_one,
 };
 
 static Scsi_Host_Template sil_sht = {
@@ -146,7 +265,10 @@
 	.check_status		= ata_check_status,
 	.exec_command		= ata_exec_command,
 	.dev_select		= ata_std_dev_select,
-	.phy_reset		= sata_phy_reset,
+//Ricky modidied 2005/08/09 --- QNAP
+//      .phy_reset              = sata_phy_reset,
+	.phy_reset		= sil_phy_reset,
+//end
 	.post_set_mode		= sil_post_set_mode,
 	.bmdma_setup            = ata_bmdma_setup,
 	.bmdma_start            = ata_bmdma_start,
@@ -155,13 +277,19 @@
 	.qc_prep		= ata_qc_prep,
 	.qc_issue		= ata_qc_issue_prot,
 	.eng_timeout		= ata_eng_timeout,
-	.irq_handler		= ata_interrupt,
+//Ricky modidied 2005/08/07 --- QNAP
+//      .irq_handler            = ata_interrupt,
+	.irq_handler		= sil_interrupt,
+//End
 	.irq_clear		= ata_bmdma_irq_clear,
 	.scr_read		= sil_scr_read,
 	.scr_write		= sil_scr_write,
 	.port_start		= ata_port_start,
 	.port_stop		= ata_port_stop,
 	.host_stop		= ata_host_stop,
+//Ricky
+	.proc_gen_write         = sil_proc_gen_write,
+//End
 };
 
 static struct ata_port_info sil_port_info[] = {
@@ -169,16 +297,28 @@
 	{
 		.sht		= &sil_sht,
 		.host_flags	= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
-				  ATA_FLAG_SRST | ATA_FLAG_MMIO,
+				  ATA_FLAG_SRST | ATA_FLAG_MMIO |
+				  SIL_FLAG_RERR_ON_DMA_ACT,
 		.pio_mask	= 0x1f,			/* pio0-4 */
 		.mwdma_mask	= 0x07,			/* mwdma0-2 */
 		.udma_mask	= 0x3f,			/* udma0-5 */
 		.port_ops	= &sil_ops,
+	},/* sil_3112_15w - keep it sync'd w/ sil_3112 */
+	{
+                .sht            = &sil_sht,
+                .host_flags     = ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
+                                  ATA_FLAG_SRST | ATA_FLAG_MMIO |
+                                  SIL_FLAG_MOD15WRITE | SIL_FLAG_RERR_ON_DMA_ACT,
+                .pio_mask       = 0x1f,                 /* pio0-4 */
+                .mwdma_mask     = 0x07,                 /* mwdma0-2 */
+                .udma_mask      = 0x3f,                 /* udma0-5 */
+                .port_ops       = &sil_ops,
 	}, /* sil_3114 */
 	{
 		.sht		= &sil_sht,
 		.host_flags	= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
-				  ATA_FLAG_SRST | ATA_FLAG_MMIO,
+				  ATA_FLAG_SRST | ATA_FLAG_MMIO | 
+				  SIL_FLAG_RERR_ON_DMA_ACT,
 		.pio_mask	= 0x1f,			/* pio0-4 */
 		.mwdma_mask	= 0x07,			/* mwdma0-2 */
 		.udma_mask	= 0x3f,			/* udma0-5 */
@@ -195,13 +335,20 @@
 	unsigned long scr;	/* SATA control register block */
 	unsigned long sien;	/* SATA Interrupt Enable register */
 	unsigned long xfer_mode;/* data transfer mode register */
+        unsigned long ser;      /* SATA Error register --- Ricky added --- QNAP */
+        unsigned long ssts;     /* SATA Status register --- Ricky added ---QNAP */
+	unsigned long sfis_cfg;	/* SATA FIS reception config register */
 } sil_port[] = {
-	/* port 0 ... */
-	{ 0x80, 0x8A, 0x00, 0x100, 0x148, 0xb4 },
-	{ 0xC0, 0xCA, 0x08, 0x180, 0x1c8, 0xf4 },
-	{ 0x280, 0x28A, 0x200, 0x300, 0x348, 0x2b4 },
-	{ 0x2C0, 0x2CA, 0x208, 0x380, 0x3c8, 0x2f4 },
-	/* ... port 3 */
+	/* port 0 ... */ /* QNAP */
+//	{ 0x80, 0x8A, 0x00, 0x100, 0x148, 0xb4 },
+        { 0x80, 0x8A, 0x00, 0x100, 0x148, 0xb4, 0x108, 0x104, 0x14c },
+//	{ 0xC0, 0xCA, 0x08, 0x180, 0x1c8, 0xf4 },
+        { 0xC0, 0xCA, 0x08, 0x180, 0x1c8, 0xf4, 0x188, 0x184, 0x1cc },
+//	{ 0x280, 0x28A, 0x200, 0x300, 0x348, 0x2b4 },
+        { 0x280, 0x28A, 0x200, 0x300, 0x348, 0x2b4, 0x308, 0x304, 0x34c },
+//	{ 0x2C0, 0x2CA, 0x208, 0x380, 0x3c8, 0x2f4 },
+        { 0x2C0, 0x2CA, 0x208, 0x380, 0x3c8, 0x2f4, 0x388, 0x384, 0x3cc },
+	/* ... port 3 */ /* End */
 };
 
 MODULE_AUTHOR("Jeff Garzik");
@@ -331,7 +478,7 @@
 		}
 	
 	/* limit requests to 15 sectors */
-	if (quirks & SIL_QUIRK_MOD15WRITE) {
+	if ((ap->flags & SIL_FLAG_MOD15WRITE) && (quirks & SIL_QUIRK_MOD15WRITE)) {
 		printk(KERN_INFO "ata%u(%u): applying Seagate errata fix\n",
 		       ap->id, dev->devno);
 		ap->host->max_sectors = 15;
@@ -349,6 +496,203 @@
 	}
 }
 
+/******************** Ricky added 2005/08/07 --- QNAP ************************/
+
+static irqreturn_t sil_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
+{
+        struct ata_host_set *host_set = dev_instance;
+        struct ata_port *ap;
+        unsigned int i;
+        unsigned int handled = 0;
+        void *mmio_base=NULL;
+        u32     tmp;
+        unsigned long flags;
+
+        VPRINTK("ENTER\n");
+        if (!host_set || !host_set->mmio_base) {
+                return IRQ_NONE;
+        }
+
+        spin_lock_irqsave(&host_set->lock, flags);
+
+	mmio_base = host_set->mmio_base;
+ 	for (i = 0; i < host_set->n_ports; i++) {
+		tmp = readl(mmio_base + PCI_BUS_REG_MASTER2);
+        	if(tmp&SATA_INT_MASK_0){
+			i=0;
+        	printk("S-ATA interrupt occur : SError %ld\n", sil_port[i].ser);
+	        	goto try_hotplug;
+        	}
+		else if(tmp&SATA_INT_MASK_1){
+			i=1;
+		printk("S-ATA interrupt occur : SError %ld\n", sil_port[i].ser);
+			goto try_hotplug;
+		}
+	        else
+        		goto done_irq;
+try_hotplug:
+    		ap = host_set->ports[i];
+        	tmp = readl(mmio_base + sil_port[i].ser);
+	        if(tmp & SATA_PHY_CHANGE){
+        		writel(readl(mmio_base + sil_port[i].ser), mmio_base + sil_port[i].ser);
+                	VPRINTK("port %u\n", i);
+	                tmp = readl(mmio_base + sil_port[i].ssts);
+			printk(" port %u : %2X\n", i, tmp);
+			if(!(tmp & SATA_PLUG)){
+                        	printk("QNAP: PHYRDY changed port %d ---- Unplugged.\n", i);
+                                /* Do stuff related to unplugging a device */
+//                                ap->status = UNPLUG;
+				sata_hot_unplug(ap);
+
+	                }
+        	        else if((tmp & SATA_PLUG) == SATA_PLUG){
+                		printk("QNAP: PHYRDY changed port %d ---- Plugged.\n", i);
+//                      	          ap->status = PLUG;
+				sata_hot_plug(ap);
+        	        }
+			else{
+				printk("QNAP: PHYRDY changed port %d ---- Unplugged. NOT GOOD!!!!!\n", i);
+                	/* Do stuff related to unplugging a device */
+//                      ap->status = UNPLUG;
+                		sata_hot_unplug(ap);
+			}
+//			handled += 1;
+/*
+                        Qnap_ap.ap[ap->id] = ap;
+                        Qnap_ap.dirty[ap->id]=1;
+                        printk("Set port %d dirty bit to 1\n", ap->id);
+                        handled = 1;
+                        ata_scsi_handle_hotplug_func(ap);
+*/
+        	}
+done_irq:
+                spin_unlock_irqrestore(&host_set->lock, flags);
+		ap = host_set->ports[i];
+                if (ap && (!(ap->flags & ATA_FLAG_PORT_DISABLED))) {
+                        struct ata_queued_cmd *qc;
+                        qc = ata_qc_from_tag(ap, ap->active_tag);
+                        if (qc && (!(qc->tf.ctl & ATA_NIEN)))
+                                handled += ata_host_intr(ap, qc);
+                }
+        }
+
+        VPRINTK("EXIT\n");
+        return IRQ_RETVAL(handled);
+}
+
+static void sil_host_init(struct ata_probe_ent *pe){
+        void *mmio = pe->mmio_base;
+        unsigned int i;
+        /* mask all SATA phy-related interrupts */
+        /* TODO: unmask bit 6 (SError N bit) for hotplug */
+        for (i = 0; i < pe->n_ports; i++){
+                writel(0, mmio + sil_port[i].sien);
+        /*
+         * Ricky added 2005/8/2
+         * Clear SError & Enable PHYRDY change interrupt
+         */
+
+                writel(SATA_CLEAR, mmio + sil_port[i].ser);
+                VPRINTK("Enable PHYRDY change interrupt\n");
+                writel(SATA_ENABLE_INT, mmio + sil_port[i].sien);
+        /** End **/
+        }
+}
+void sil_pci_remove_one(struct pci_dev *pdev){
+        u32 tmp;
+        unsigned int i, port;
+        void *mmio_base;
+//        unsigned long mmio;
+
+        mmio_base = ioremap(pci_resource_start(pdev, 5),
+                pci_resource_len(pdev, 5));
+//        mmio = (unsigned long)mmio_base;
+        if(pdev->device==0x3114)
+                port = 4;
+        else
+                port = 2;
+        for (i = 0; i < port; i++){
+                printk("Disable PHYRDY change interrupt\n");
+                tmp = readl(mmio_base + sil_port[i].sien);
+                tmp &= ~SATA_ENABLE_INT;
+                writel(tmp, mmio_base + sil_port[i].sien);
+        }
+        ata_pci_remove_one(pdev);
+        remove_proc_entry(SATA_PROC,NULL);
+}
+
+static inline void sii_disable_channel_hotplug_interrupts(struct ata_port *ap){
+        u32 tmp;
+        void *mmio = ap->host_set->mmio_base;
+        VPRINTK("ENTER, port=%d\n", ap->port_no);
+        writel(0, mmio + sil_port[ap->port_no].sien);
+        tmp = readl(mmio + sil_port[ap->port_no].sien);
+        tmp &= ~SATA_ENABLE_INT;
+        writel(tmp, mmio + sil_port[ap->port_no].sien);
+        VPRINTK("EXIT\n");
+}
+
+static inline void sii_enable_channel_hotplug_interrupts(struct ata_port *ap){
+        void *mmio = ap->host_set->mmio_base;
+        VPRINTK("ENTER\n");
+        writel(SATA_CLEAR, mmio + sil_port[ap->port_no].ser);
+        writel(SATA_ENABLE_INT, mmio + sil_port[ap->port_no].sien);
+        VPRINTK("EXIT\n");
+}
+
+static void sil_phy_reset(struct ata_port *ap){
+        if (ap->flags & ATA_FLAG_SATA) {
+                sii_disable_channel_hotplug_interrupts(ap);
+                sata_phy_reset(ap);
+                sii_enable_channel_hotplug_interrupts(ap);
+        }
+        else
+                sata_phy_reset(ap);
+}
+
+int sil_proc_gen_write(struct file * file, const char * buf,
+                                unsigned long length, void *data){
+
+        int err=0;
+        char * buffer;
+        if (!buf || length>PAGE_SIZE)
+                return -EINVAL;
+        if (!(buffer = (char *) __get_free_page(GFP_KERNEL)))
+                return -ENOMEM;
+        if(copy_from_user(buffer, buf, length)){
+                err =-EFAULT;
+                goto out;
+        }
+
+        err = -EINVAL;
+        if (length < PAGE_SIZE)
+                buffer[length] = '\0';
+        else if (buffer[PAGE_SIZE-1])
+                goto out;
+        if(!strncmp("probe", buffer, 1)){
+                int i=1;
+                printk("QNAP: Probe port\n");
+                for(;i<MAX_ENTRY+1;i++){
+                        if(Qnap_ap.dirty[i]==1){
+                                if(Qnap_ap.ap[i]!=NULL){
+                                        ata_bus_probe(Qnap_ap.ap[i]);
+                                        printk("QNAP: Set port %d dirty bit to 0\n",i);
+                                        Qnap_ap.dirty[i]=0;
+                                }
+                                else{
+                                        printk("QNAP: No device --- bug\n");
+                                }
+                        }
+                }
+        }
+
+out:
+        free_page((unsigned long) buffer);
+        return err;
+}
+
+/************************************** End *************************************/
+
 static int sil_init_one (struct pci_dev *pdev, const struct pci_device_id *ent)
 {
 	static int printed_version;
@@ -443,6 +787,23 @@
 		printk(KERN_WARNING DRV_NAME "(%s): cache line size not set.  Driver may not function\n",
 			pci_name(pdev));
 
+	/* Apply R_ERR on DMA activate FIS errata workaround */
+	if (probe_ent->host_flags & SIL_FLAG_RERR_ON_DMA_ACT) {
+		int cnt;
+
+		for (i = 0, cnt = 0; i < probe_ent->n_ports; i++) {
+			tmp = readl(mmio_base + sil_port[i].sfis_cfg);
+			if ((tmp & 0x3) != 0x01)
+				continue;
+			if (!cnt)
+				dev_printk(KERN_INFO, &pdev->dev,
+					   "Applying R_ERR on DMA activate "
+					   "FIS errata fix\n");
+			writel(tmp & ~0x3, mmio_base + sil_port[i].sfis_cfg);
+			cnt++;
+		}
+	}
+
 	if (ent->driver_data == sil_3114) {
 		irq_mask = SIL_MASK_4PORT;
 
@@ -463,12 +824,24 @@
 		writel(tmp, mmio_base + SIL_SYSCFG);
 		readl(mmio_base + SIL_SYSCFG);	/* flush */
 	}
+	sata_proc = create_proc_read_entry( SATA_PROC,0,NULL,read_proc,NULL);
+        if(sata_proc==NULL){
+                printk("create proc entry failed!\n");
+                goto error_register;
+        }
+        else{
+                sata_proc->write_proc=NULL;
+                printk("create proc entry succeed!\n");
+        }
 
+//Ricky modified 2005/08/07
 	/* mask all SATA phy-related interrupts */
 	/* TODO: unmask bit 6 (SError N bit) for hotplug */
-	for (i = 0; i < probe_ent->n_ports; i++)
-		writel(0, mmio_base + sil_port[i].sien);
-
+//	for (i = 0; i < probe_ent->n_ports; i++)
+//		writel(0, mmio_base + sil_port[i].sien);
+        /* initialize adapter */
+        sil_host_init(probe_ent);
+//End
 	pci_set_master(pdev);
 
 	/* FIXME: check ata_device_add return value */
@@ -477,6 +850,8 @@
 
 	return 0;
 
+error_register:
+        remove_proc_entry(SATA_PROC,NULL);
 err_out_free_ent:
 	kfree(probe_ent);
 err_out_regions:
