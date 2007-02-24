--- linux-2.6.12.3/drivers/scsi/libata-scsi.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/libata-scsi.c	2007-02-17 14:10:18.000000000 -0500
@@ -29,14 +29,24 @@
 #include "scsi.h"
 #include <scsi/scsi_host.h>
 #include <linux/libata.h>
+#include <linux/hdreg.h>
 #include <asm/uaccess.h>
-
 #include "libata.h"
+#ifdef CONFIG_SANDPOINT
+#include <platforms/qnap.h>
+#endif
+
+#define SECTOR_SIZE	512
 
 typedef unsigned int (*ata_xlat_func_t)(struct ata_queued_cmd *qc, u8 *scsicmd);
 static struct ata_device *
 ata_scsi_find_dev(struct ata_port *ap, struct scsi_device *scsidev);
 
+/* Ricky added */
+extern int dev_exist[MAX_ENTRY];
+extern int status_changed;
+//extern struct proc_dir_entry *sata_proc;
+/* End */
 
 /**
  *	ata_std_bios_param - generic bios head/sector/cylinder calculator used by sd.
@@ -67,6 +77,164 @@
 	return 0;
 }
 
+/**
+ *	ata_cmd_ioctl - Handler for HDIO_DRIVE_CMD ioctl
+ *	@dev: Device to whom we are issuing command
+ *	@arg: User provided data for issuing command
+ *
+ *	LOCKING:
+ *	Defined by the SCSI layer.  We don't really care.
+ *
+ *	RETURNS:
+ *	Zero on success, negative errno on error.
+ */
+
+int ata_cmd_ioctl(struct scsi_device *scsidev, void __user *arg)
+{
+	int rc = 0;
+	u8 scsi_cmd[MAX_COMMAND_SIZE];
+	u8 args[4], *argbuf = NULL;
+	int argsize = 0;
+	struct scsi_request *sreq;
+
+	if (NULL == (void *)arg)
+		return -EINVAL;
+
+	if (copy_from_user(args, arg, sizeof(args)))
+		return -EFAULT;
+
+	sreq = scsi_allocate_request(scsidev, GFP_KERNEL);
+	if (!sreq)
+		return -EINTR;
+
+	memset(scsi_cmd, 0, sizeof(scsi_cmd));
+
+	if (args[3]) {
+		argsize = SECTOR_SIZE * args[3];
+		argbuf = kmalloc(argsize, GFP_KERNEL);
+		if (argbuf == NULL)
+			return -ENOMEM;
+
+		scsi_cmd[1]  = (4 << 1); /* PIO Data-in */
+		scsi_cmd[2]  = 0x0e;     /* no off.line or cc, read from dev,
+		                            block count in sector count field */
+		sreq->sr_data_direction = DMA_FROM_DEVICE;
+	} else {
+//Ricky
+//		argsize = 4 + (SECTOR_SIZE * args[3]);
+		argsize = 4;
+		argbuf = kmalloc(argsize, GFP_KERNEL);
+//End
+		scsi_cmd[1]  = (3 << 1); /* Non-data */
+		/* scsi_cmd[2] is already 0 -- no off.line, cc, or data xfer */
+		sreq->sr_data_direction = DMA_NONE;
+	}
+
+	scsi_cmd[0] = ATA_16;
+
+	scsi_cmd[4] = args[2];
+	if (args[0] == WIN_SMART) { /* hack -- ide driver does this too... */
+		scsi_cmd[6]  = args[3];
+		scsi_cmd[8]  = args[1];
+		scsi_cmd[10] = 0x4f;
+		scsi_cmd[12] = 0xc2;
+	} else {
+		scsi_cmd[6]  = args[1];
+	}
+	scsi_cmd[14] = args[0];
+
+	/* Good values for timeout and retries?  Values below
+	   from scsi_ioctl_send_command() for default case... */
+	scsi_wait_req(sreq, scsi_cmd, argbuf, argsize, (10*HZ), 5);
+
+	if (sreq->sr_result) {
+		rc = -EIO;
+		goto error;
+	}
+
+	/* Need code to retrieve data from check condition? */
+
+//Ricky
+//printk("7. argbuf[2](Standy status: 0XFF-> active, 0x0 -> standby): %p, 0x%X\n", argbuf, argbuf[2]);
+	if(scsi_cmd[14] == WIN_CHECKPOWERMODE1 || scsi_cmd[14] == WIN_CHECKPOWERMODE2){
+		if ((argbuf)
+		 && copy_to_user((void __user *)(arg), argbuf, argsize))
+			rc = -EFAULT;
+//End
+	}
+	else{
+		if ((argbuf)
+		 && copy_to_user((void *)(arg + sizeof(args)), argbuf, argsize))
+			rc = -EFAULT;
+	}
+error:
+	scsi_release_request(sreq);
+
+	if (argbuf)
+		kfree(argbuf);
+
+	return rc;
+}
+
+/**
+ *	ata_task_ioctl - Handler for HDIO_DRIVE_TASK ioctl
+ *	@dev: Device to whom we are issuing command
+ *	@arg: User provided data for issuing command
+ *
+ *	LOCKING:
+ *	Defined by the SCSI layer.  We don't really care.
+ *
+ *	RETURNS:
+ *	Zero on success, negative errno on error.
+ */
+int ata_task_ioctl(struct scsi_device *scsidev, void __user *arg)
+{
+	int rc = 0;
+	u8 scsi_cmd[MAX_COMMAND_SIZE];
+	u8 args[7];
+	struct scsi_request *sreq;
+
+	if (NULL == (void *)arg)
+		return -EINVAL;
+
+	if (copy_from_user(args, arg, sizeof(args)))
+		return -EFAULT;
+
+	memset(scsi_cmd, 0, sizeof(scsi_cmd));
+	scsi_cmd[0]  = ATA_16;
+	scsi_cmd[1]  = (3 << 1); /* Non-data */
+	/* scsi_cmd[2] is already 0 -- no off.line, cc, or data xfer */
+	scsi_cmd[4]  = args[1];
+	scsi_cmd[6]  = args[2];
+	scsi_cmd[8]  = args[3];
+	scsi_cmd[10] = args[4];
+	scsi_cmd[12] = args[5];
+	scsi_cmd[14] = args[0];
+
+	sreq = scsi_allocate_request(scsidev, GFP_KERNEL);
+	if (!sreq) {
+		rc = -EINTR;
+		goto error;
+	}
+
+	sreq->sr_data_direction = DMA_NONE;
+	/* Good values for timeout and retries?  Values below
+	   from scsi_ioctl_send_command() for default case... */
+	scsi_wait_req(sreq, scsi_cmd, NULL, 0, (10*HZ), 5);
+
+	if (sreq->sr_result) {
+		rc = -EIO;
+		goto error;
+	}
+
+	/* Need code to retrieve data from check condition? */
+
+error:
+	scsi_release_request(sreq);
+	return rc;
+}
+
+
 int ata_scsi_ioctl(struct scsi_device *scsidev, int cmd, void __user *arg)
 {
 	struct ata_port *ap;
@@ -96,6 +264,16 @@
 			return -EINVAL;
 		return 0;
 
+	case HDIO_DRIVE_CMD:
+		if (!capable(CAP_SYS_ADMIN) || !capable(CAP_SYS_RAWIO))
+			return -EACCES;
+		return ata_cmd_ioctl(scsidev, arg);
+
+	case HDIO_DRIVE_TASK:
+		if (!capable(CAP_SYS_ADMIN) || !capable(CAP_SYS_RAWIO))
+			return -EACCES;
+		return ata_task_ioctl(scsidev, arg);
+
 	default:
 		rc = -ENOTTY;
 		break;
@@ -154,6 +332,51 @@
 }
 
 /**
+ *	ata_dump_status - user friendly display of error info
+ *	@id: id of the port in question
+ *	@tf: ptr to filled out taskfile
+ *
+ *	Decode and dump the ATA error/status registers for the user so
+ *	that they have some idea what really happened at the non
+ *	make-believe layer.
+ *
+ *	LOCKING:
+ *	inherited from caller
+ */
+void ata_dump_status(unsigned id, struct ata_taskfile *tf)
+{
+	u8 stat = tf->command, err = tf->feature;
+
+	printk(KERN_WARNING "ata%u: status=0x%02x { ", id, stat);
+	if (stat & ATA_BUSY) {
+		printk("Busy }\n");	/* Data is not valid in this case */
+	} else {
+		if (stat & 0x40)	printk("DriveReady ");
+		if (stat & 0x20)	printk("DeviceFault ");
+		if (stat & 0x10)	printk("SeekComplete ");
+		if (stat & 0x08)	printk("DataRequest ");
+		if (stat & 0x04)	printk("CorrectedError ");
+		if (stat & 0x02)	printk("Index ");
+		if (stat & 0x01)	printk("Error ");
+		printk("}\n");
+
+		if (err) {
+			printk(KERN_WARNING "ata%u: error=0x%02x { ", id, err);
+			if (err & 0x04)		printk("DriveStatusError ");
+			if (err & 0x80) {
+				if (err & 0x04)	printk("BadCRC ");
+				else		printk("Sector ");
+			}
+			if (err & 0x40)		printk("UncorrectableError ");
+			if (err & 0x10)		printk("SectorIdNotFound ");
+			if (err & 0x02)		printk("TrackZeroNotFound ");
+			if (err & 0x01)		printk("AddrMarkNotFound ");
+			printk("}\n");
+		}
+	}
+}
+
+/**
  *	ata_to_sense_error - convert ATA error to SCSI error
  *	@qc: Command that we are erroring out
  *	@drv_stat: value contained in ATA status register
@@ -167,11 +390,10 @@
  *	spin_lock_irqsave(host_set lock)
  */
 
-void ata_to_sense_error(struct ata_queued_cmd *qc, u8 drv_stat)
-{
-	struct scsi_cmnd *cmd = qc->scsicmd;
-	u8 err = 0;
-	unsigned char *sb = cmd->sense_buffer;
+//void ata_to_sense_error(struct ata_queued_cmd *qc, u8 drv_stat)
+void ata_to_sense_error(unsigned id, u8 drv_stat, u8 drv_err, u8 *sk, u8 *asc, 
+			u8 *ascq){
+	int i;
 	/* Based on the 3ware driver translation table */
 	static unsigned char sense_table[][4] = {
 		/* BBD|ECC|ID|MAR */
@@ -212,106 +434,153 @@
 		{0x04, 		RECOVERED_ERROR, 0x11, 0x00},	// Recovered ECC error	  Medium error, recovered
 		{0xFF, 0xFF, 0xFF, 0xFF}, // END mark
 	};
-	int i = 0;
-
-	cmd->result = SAM_STAT_CHECK_CONDITION;
 
 	/*
 	 *	Is this an error we can process/parse
 	 */
-
-	if(drv_stat & ATA_ERR)
-		/* Read the err bits */
-		err = ata_chk_err(qc->ap);
-
-	/* Display the ATA level error info */
-
-	printk(KERN_WARNING "ata%u: status=0x%02x { ", qc->ap->id, drv_stat);
-	if(drv_stat & 0x80)
-	{
-		printk("Busy ");
-		err = 0;	/* Data is not valid in this case */
-	}
-	else {
-		if(drv_stat & 0x40)	printk("DriveReady ");
-		if(drv_stat & 0x20)	printk("DeviceFault ");
-		if(drv_stat & 0x10)	printk("SeekComplete ");
-		if(drv_stat & 0x08)	printk("DataRequest ");
-		if(drv_stat & 0x04)	printk("CorrectedError ");
-		if(drv_stat & 0x02)	printk("Index ");
-		if(drv_stat & 0x01)	printk("Error ");
+	if (drv_stat & ATA_BUSY) {
+		drv_err = 0;	/* Ignore the err bits, they're invalid */
 	}
-	printk("}\n");
 
-	if(err)
-	{
-		printk(KERN_WARNING "ata%u: error=0x%02x { ", qc->ap->id, err);
-		if(err & 0x04)		printk("DriveStatusError ");
-		if(err & 0x80)
-		{
-			if(err & 0x04)
-				printk("BadCRC ");
-			else
-				printk("Sector ");
+	if (drv_err) {
+		/* Look for drv_err */
+		for (i = 0; sense_table[i][0] != 0xFF; i++) {
+			/* Look for best matches first */
+			if ((sense_table[i][0] & drv_err) == 
+			    sense_table[i][0]) {
+				*sk = sense_table[i][1];
+				*asc = sense_table[i][2];
+				*ascq = sense_table[i][3];
+				goto translate_done;
+			}
 		}
-		if(err & 0x40)		printk("UncorrectableError ");
-		if(err & 0x10)		printk("SectorIdNotFound ");
-		if(err & 0x02)		printk("TrackZeroNotFound ");
-		if(err & 0x01)		printk("AddrMarkNotFound ");
-		printk("}\n");
-
-		/* Should we dump sector info here too ?? */
+		/* No immediate match */
+		printk(KERN_WARNING "ata%u: no sense translation for "
+		       "error 0x%02x\n", id, drv_err);	
 	}
 
 
-	/* Look for err */
-	while(sense_table[i][0] != 0xFF)
-	{
-		/* Look for best matches first */
-		if((sense_table[i][0] & err) == sense_table[i][0])
-		{
-			sb[0] = 0x70;
-			sb[2] = sense_table[i][1];
-			sb[7] = 0x0a;
-			sb[12] = sense_table[i][2];
-			sb[13] = sense_table[i][3];
-			return;
+	/* Fall back to interpreting status bits */
+	for (i = 0; stat_table[i][0] != 0xFF; i++) {
+		if (stat_table[i][0] & drv_stat) {
+			*sk = stat_table[i][1];
+			*asc = stat_table[i][2];
+			*ascq = stat_table[i][3];
+			goto translate_done;
 		}
-		i++;
 	}
-	/* No immediate match */
-	if(err)
-		printk(KERN_DEBUG "ata%u: no sense translation for 0x%02x\n", qc->ap->id, err);
+	/* No error?  Undecoded? */
+	printk(KERN_WARNING "ata%u: no sense translation for status: 0x%02x\n", 
+	       id, drv_stat);
+
+	/* For our last chance pick, use medium read error because
+	 * it's much more common than an ATA drive telling you a write
+	 * has failed.
+	 */
+	*sk = MEDIUM_ERROR;
+	*asc = 0x11; /* "unrecovered read error" */
+	*ascq = 0x04; /*  "auto-reallocation failed" */
+
+ translate_done:
+	printk(KERN_ERR "ata%u: translated ATA stat/err 0x%02x/%02x to "
+	       "SCSI SK/ASC/ASCQ 0x%x/%02x/%02x\n", id, drv_stat, drv_err,
+	       *sk, *asc, *ascq);
+	return;
+}
 
-	i = 0;
-	/* Fall back to interpreting status bits */
-	while(stat_table[i][0] != 0xFF)
-	{
-		if(stat_table[i][0] & drv_stat)
-		{
-			sb[0] = 0x70;
-			sb[2] = stat_table[i][1];
-			sb[7] = 0x0a;
-			sb[12] = stat_table[i][2];
-			sb[13] = stat_table[i][3];
-			return;
-		}
-		i++;
+void ata_gen_ata_desc_sense(struct ata_queued_cmd *qc)
+{
+	struct scsi_cmnd *cmd = qc->scsicmd;
+	struct ata_taskfile *tf = &qc->tf;
+	unsigned char *sb = cmd->sense_buffer;
+	unsigned char *desc = sb + 8;
+
+	memset(sb, 0, SCSI_SENSE_BUFFERSIZE);
+
+	cmd->result = SAM_STAT_CHECK_CONDITION;
+
+	/*
+	 * Read the controller registers.
+	 */
+	assert(NULL != qc->ap->ops->tf_read);
+	qc->ap->ops->tf_read(qc->ap, tf);
+ 
+	/*
+	 * Use ata_to_sense_error() to map status register bits
+	 * onto sense key, asc & ascq.
+	 */
+	if (unlikely(tf->command & (ATA_BUSY | ATA_DF | ATA_ERR | ATA_DRQ))) {
+		ata_to_sense_error(qc->ap->id, tf->command, tf->feature,
+				   &sb[1], &sb[2], &sb[3]);
+		sb[1] &= 0x0f;
+ 	}
+ 
+	/*
+	 * Sense data is current and format is descriptor.
+	 */
+	sb[0] = 0x72;
+	desc[0] = 0x09;
+
+	/*
+	 * Set length of additional sense data.
+	 * Since we only populate descriptor 0, the total
+	 * length is the same (fixed) length as descriptor 0.
+	 */
+	desc[1] = sb[7] = 14;
+
+	/*
+	 * Copy registers into sense buffer.
+	 */
+	desc[2] = 0x00;
+	desc[3] = tf->feature;	/* == error reg */
+	desc[5] = tf->nsect;
+	desc[7] = tf->lbal;
+	desc[9] = tf->lbam;
+	desc[11] = tf->lbah;
+	desc[12] = tf->device;
+	desc[13] = tf->command; /* == status reg */
+
+	/*
+	 * Fill in Extend bit, and the high order bytes
+	 * if applicable.
+	 */
+	if (tf->flags & ATA_TFLAG_LBA48) {
+		desc[2] |= 0x01;
+		desc[4] = tf->hob_nsect;
+		desc[6] = tf->hob_lbal;
+		desc[8] = tf->hob_lbam;
+		desc[10] = tf->hob_lbah;
 	}
-	/* No error ?? */
-	printk(KERN_ERR "ata%u: called with no error (%02X)!\n", qc->ap->id, drv_stat);
-	/* additional-sense-code[-qualifier] */
+}
+
+void ata_gen_fixed_sense(struct ata_queued_cmd *qc)
+{
+	struct scsi_cmnd *cmd = qc->scsicmd;
+	struct ata_taskfile *tf = &qc->tf;
+	unsigned char *sb = cmd->sense_buffer;
 
+	memset(sb, 0, SCSI_SENSE_BUFFERSIZE);
+
+	cmd->result = SAM_STAT_CHECK_CONDITION;
+
+	assert(NULL != qc->ap->ops->tf_read);
+	qc->ap->ops->tf_read(qc->ap, tf);
+
+	if (unlikely(tf->command & (ATA_BUSY | ATA_DF | ATA_ERR | ATA_DRQ))) {
+		ata_to_sense_error(qc->ap->id, tf->command, tf->feature,
+				   &sb[2], &sb[12], &sb[13]);
+		sb[2] &= 0x0f;
+ 	}
 	sb[0] = 0x70;
-	sb[2] = MEDIUM_ERROR;
-	sb[7] = 0x0A;
-	if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
-		sb[12] = 0x11; /* "unrecovered read error" */
-		sb[13] = 0x04;
-	} else {
-		sb[12] = 0x0C; /* "write error -             */
-		sb[13] = 0x02; /*  auto-reallocation failed" */
-	}
+	sb[7] = 0x0a;
+
+	if (!(tf->flags & ATA_TFLAG_LBA48)) {
+		sb[0] |= 0x80;	
+		sb[3] = tf->device & 0x0f;
+		sb[4] = tf->lbah;
+		sb[5] = tf->lbam;
+		sb[6] = tf->lbal;
+ 	} 
 }
 
 /**
@@ -629,11 +898,42 @@
 static int ata_scsi_qc_complete(struct ata_queued_cmd *qc, u8 drv_stat)
 {
 	struct scsi_cmnd *cmd = qc->scsicmd;
+	int need_sense = drv_stat & (ATA_ERR | ATA_BUSY | ATA_DRQ);
 
-	if (unlikely(drv_stat & (ATA_ERR | ATA_BUSY | ATA_DRQ)))
-		ata_to_sense_error(qc, drv_stat);
-	else
-		cmd->result = SAM_STAT_GOOD;
+	if (unlikely(!ata_scsi_find_dev(qc->ap, cmd->device))) {
+                cmd->result = (DID_BAD_TARGET << 16);
+                goto out_no_dev;
+        }
+
+	/* For ATA pass thru (SAT) commands, generate a sense block if
+	 * user mandated it or if there's an error.  Note that if we
+	 * generate because the user forced us to, a check condition
+	 * is generated and the ATA register values are returned
+	 * whether the command completed successfully or not. If there
+	 * was no error, SK, ASC and ASCQ will all be zero.
+	 */
+	if (((cmd->cmnd[0] == ATA_16) || (cmd->cmnd[0] == ATA_12)) &&
+ 	    ((cmd->cmnd[2] & 0x20) || need_sense)) {
+ 		ata_gen_ata_desc_sense(qc);
+	} else {
+		if (!need_sense) {
+			cmd->result = SAM_STAT_GOOD;
+		} else {
+			/* TODO: decide which descriptor format to use
+			 * for 48b LBA devices and call that here
+			 * instead of the fixed desc, which is only
+			 * good for smaller LBA (and maybe CHS?)
+			 * devices.
+			 */
+			ata_gen_fixed_sense(qc);
+		}
+	}
+
+	if (need_sense) {
+		/* The ata_gen_..._sense routines fill in tf */
+		ata_dump_status(qc->ap->id, &qc->tf);
+	}
+out_no_dev:
 
 	qc->scsidone(cmd);
 
@@ -699,7 +999,23 @@
 	/* select device, send command to hardware */
 	if (ata_qc_issue(qc))
 		goto err_out;
-
+//Ricky
+	{
+		if(qc->tf.command == WIN_CHECKPOWERMODE1 || qc->tf.command == WIN_CHECKPOWERMODE2){
+			int index=0;
+			u8 *argbuf, nsector;
+//Fixme: There should be delay for ? naroseconds to get the correct value of standby status. Here is delay for 400*12 narosecond = 0.0000048 seconds by experiment.
+			for(index=0;index<200;index++)
+				ata_pause(ap);
+//End of fixme.
+			nsector = readb((void __iomem *) ap->ioaddr.nsect_addr);
+			if(cmd->request_buffer != NULL){
+				argbuf = (u8*)cmd->request_buffer;
+				argbuf[2] = nsector;
+			}
+		}
+        }
+//End
 	VPRINTK("EXIT\n");
 	return;
 
@@ -1396,6 +1712,144 @@
 	return dev;
 }
 
+/*
+ *	ata_scsi_map_proto - Map pass-thru protocol value to taskfile value.
+ *	@byte1: Byte 1 from pass-thru CDB.
+ *
+ *	RETURNS:
+ *	ATA_PROT_UNKNOWN if mapping failed/unimplemented, protocol otherwise.
+ */
+static u8
+ata_scsi_map_proto(u8 byte1)
+{
+	switch((byte1 & 0x1e) >> 1) {
+		case 3:		/* Non-data */
+			return ATA_PROT_NODATA;
+
+		case 6:		/* DMA */
+			return ATA_PROT_DMA;
+
+		case 4:		/* PIO Data-in */
+		case 5:		/* PIO Data-out */
+			if (byte1 & 0xe0) {
+				return ATA_PROT_PIO_MULT;
+			}
+			return ATA_PROT_PIO;
+
+		case 10:	/* Device Reset */
+		case 0:		/* Hard Reset */
+		case 1:		/* SRST */
+		case 2:		/* Bus Idle */
+		case 7:		/* Packet */
+		case 8:		/* DMA Queued */
+		case 9:		/* Device Diagnostic */
+		case 11:	/* UDMA Data-in */
+		case 12:	/* UDMA Data-Out */
+		case 13:	/* FPDMA */
+		default:	/* Reserved */
+			break;
+	}
+
+	return ATA_PROT_UNKNOWN;
+}
+
+/**
+ *	ata_scsi_pass_thru - convert ATA pass-thru CDB to taskfile
+ *	@qc: command structure to be initialized
+ *	@cmd: SCSI command to convert
+ *
+ *	Handles either 12 or 16-byte versions of the CDB.
+ *
+ *	RETURNS:
+ *	Zero on success, non-zero on failure.
+ */
+static unsigned int
+ata_scsi_pass_thru(struct ata_queued_cmd *qc, u8 *scsicmd)
+{
+	struct ata_taskfile *tf = &(qc->tf);
+	struct scsi_cmnd *cmd = qc->scsicmd;
+
+	if ((tf->protocol = ata_scsi_map_proto(scsicmd[1])) == ATA_PROT_UNKNOWN)
+		return 1;
+
+	/*
+	 * 12 and 16 byte CDBs use different offsets to
+	 * provide the various register values.
+	 */
+	if (scsicmd[0] == ATA_16) {
+		/*
+		 * 16-byte CDB - may contain extended commands.
+		 *
+		 * If that is the case, copy the upper byte register values.
+		 */
+		if (scsicmd[1] & 0x01) {
+			tf->hob_feature = scsicmd[3];
+			tf->hob_nsect = scsicmd[5];
+			tf->hob_lbal = scsicmd[7];
+			tf->hob_lbam = scsicmd[9];
+			tf->hob_lbah = scsicmd[11];
+			tf->flags |= ATA_TFLAG_LBA48;
+		} else
+			tf->flags &= ~ATA_TFLAG_LBA48;
+
+		/*
+		 * Always copy low byte, device and command registers.
+		 */
+		tf->feature = scsicmd[4];
+		tf->nsect = scsicmd[6];
+		tf->lbal = scsicmd[8];
+		tf->lbam = scsicmd[10];
+		tf->lbah = scsicmd[12];
+		tf->device = scsicmd[13];
+		tf->command = scsicmd[14];
+	} else {
+		/*
+		 * 12-byte CDB - incapable of extended commands.
+		 */
+		tf->flags &= ~ATA_TFLAG_LBA48;
+
+		tf->feature = scsicmd[3];
+		tf->nsect = scsicmd[4];
+		tf->lbal = scsicmd[5];
+		tf->lbam = scsicmd[6];
+		tf->lbah = scsicmd[7];
+		tf->device = scsicmd[8];
+		tf->command = scsicmd[9];
+	}
+
+	/*
+	 * Filter SET_FEATURES - XFER MODE command -- otherwise,
+	 * SET_FEATURES - XFER MODE must be preceded/succeeded
+	 * by an update to hardware-specific registers for each
+	 * controller (i.e. the reason for ->set_piomode(),
+	 * ->set_dmamode(), and ->post_set_mode() hooks).
+	 */
+	if ((tf->command == ATA_CMD_SET_FEATURES)
+	 && (tf->feature == SETFEATURES_XFER))
+		return 1;
+
+	/*
+	 * Set flags so that all registers will be written,
+	 * and pass on write indication (used for PIO/DMA
+	 * setup.)
+	 */
+	tf->flags |= (ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE);
+
+	if (cmd->sc_data_direction == DMA_TO_DEVICE)
+		tf->flags |= ATA_TFLAG_WRITE;
+
+	/*
+	 * Set transfer length.
+	 *
+	 * TODO: find out if we need to do more here to
+	 *       cover scatter/gather case.
+	 */
+	qc->nsect = cmd->bufflen / ATA_SECT_SIZE;
+
+	return 0;
+}
+
+
 /**
  *	ata_get_xlat_func - check if SCSI to ATA translation is possible
  *	@dev: ATA device
@@ -1428,6 +1882,10 @@
 	case VERIFY:
 	case VERIFY_16:
 		return ata_scsi_verify_xlat;
+
+	case ATA_12:
+	case ATA_16:
+		return ata_scsi_pass_thru;
 	}
 
 	return NULL;
@@ -1503,7 +1961,7 @@
 			ata_scsi_simulate(dev->id, cmd, done);
 	} else
 		ata_scsi_translate(ap, dev, cmd, done, atapi_xlat);
-
+	
 out_unlock:
 	return 0;
 }
@@ -1594,3 +2052,55 @@
 	}
 }
 
+/************** Ricky added 2005/08/07 --- QNAP **************/
+
+int read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data){
+        char *s_buf = page;
+        int len=0;
+        int idx=0;
+        len += sprintf(s_buf+len, "%d ",status_changed);
+        for(idx=0;idx<MAX_ENTRY;idx++){
+                len += sprintf(s_buf+len, "%d ",dev_exist[idx]);
+        }
+        status_changed=0;
+        *eof=1;
+        return len;
+}
+
+static int ata_scsi_qc_abort(struct ata_queued_cmd *qc, u8 drv_stat){
+        struct scsi_cmnd *cmd = qc->scsicmd;
+        cmd->result = SAM_STAT_TASK_ABORTED; //FIXME:  Is this what we want?
+        qc->scsidone(cmd);
+        return 0;
+}
+
+void ata_scsi_prepare_qc_abort(struct ata_queued_cmd *qc){
+        // For some reason or another, we can't allow a normal scsi_qc_complete
+        if (qc->complete_fn == ata_scsi_qc_complete);
+        qc->complete_fn = ata_scsi_qc_abort;
+}
+void ata_check_kill_qc(struct ata_port *ap, unsigned int device)
+{
+       struct ata_queued_cmd *qc = ata_qc_from_tag(ap, ap->active_tag);
+
+       if (unlikely(qc) && device == qc->dev->devno) {
+               /* Kill outstanding qc to device if one exists */
+               ata_qc_complete(qc, ATA_ERR);
+       }
+}
+#ifdef CONFIG_SANDPOINT
+int qnap_get_ts101_sata_port(struct scsi_device *scsidev){
+	struct ata_port *ap;
+	int rc = -EINVAL;
+	ap = (struct ata_port *) &scsidev->host->hostdata[0];
+	if (!ap)
+                goto out;
+	return ap->id;
+
+out:
+	return rc;
+}
+EXPORT_SYMBOL(qnap_get_ts101_sata_port);
+#endif
+/*************************** End *****************************/
+
