--- linux-2.6.12.3/include/linux/root_dev.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/linux/root_dev.h	2007-02-17 14:10:08.000000000 -0500
@@ -12,6 +12,8 @@
 	Root_HDA2 = MKDEV(IDE0_MAJOR, 2),
 	Root_SDA1 = MKDEV(SCSI_DISK0_MAJOR, 1),
 	Root_SDA2 = MKDEV(SCSI_DISK0_MAJOR, 2),
+	Root_SDA3 = MKDEV(SCSI_DISK0_MAJOR, 3),
+	Root_SDA4 = MKDEV(SCSI_DISK0_MAJOR, 4),
 	Root_HDC1 = MKDEV(IDE1_MAJOR, 1),
 	Root_SR0 = MKDEV(SCSI_CDROM_MAJOR, 0),
 };
