--- linux-2.6.12.3/drivers/scsi/libata.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/scsi/libata.h	2007-02-17 14:10:19.000000000 -0500
@@ -44,10 +44,12 @@
                            unsigned int wait, unsigned int can_sleep);
 extern void ata_tf_to_host_nolock(struct ata_port *ap, struct ata_taskfile *tf);
 extern void swap_buf_le16(u16 *buf, unsigned int buf_words);
+extern int ata_task_ioctl(struct scsi_device *scsidev, void __user *arg);
+extern int ata_cmd_ioctl(struct scsi_device *scsidev, void __user *arg);
 
 
 /* libata-scsi.c */
-extern void ata_to_sense_error(struct ata_queued_cmd *qc, u8 drv_stat);
+//extern void ata_to_sense_error(struct ata_queued_cmd *qc, u8 drv_stat);
 extern int ata_scsi_error(struct Scsi_Host *host);
 extern unsigned int ata_scsiop_inq_std(struct ata_scsi_args *args, u8 *rbuf,
 			       unsigned int buflen);
@@ -86,4 +88,10 @@
 	ata_scsi_badcmd(cmd, done, 0x24, 0x00);
 }
 
+extern struct proc_dir_entry *sata_proc;
+extern int read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data);
+extern void ata_check_kill_qc(struct ata_port *ap, unsigned int device);
+extern void ata_scsi_hot_plug(struct ata_port *ap, unsigned int device);
+extern void ata_scsi_hot_unplug(struct ata_port *ap, unsigned int device);
+
 #endif /* __LIBATA_H__ */
