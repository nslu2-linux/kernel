Index: linux-2.6.13/include/scsi/scsi.h
===================================================================
--- linux-2.6.13.orig/include/scsi/scsi.h	2005-08-28 19:41:01.000000000 -0400
+++ linux-2.6.13/include/scsi/scsi.h	2007-02-24 19:18:08.000000000 -0500
@@ -116,6 +116,13 @@
 /* values for service action in */
 #define	SAI_READ_CAPACITY_16  0x10
 
+/* Values for T10/04-262r7 */
+#define	ATA_16		      0x85	/* 16-byte pass-thru */
+#define	ATA_12		      0xa1	/* 12-byte pass-thru */
+
+/* Values for T10/04-262r7 */
+#define	ATA_16		      0x85	/* 16-byte pass-thru */
+#define	ATA_12		      0xa1	/* 12-byte pass-thru */
 
 /*
  *  SCSI Architecture Model (SAM) Status codes. Taken from SAM-3 draft
