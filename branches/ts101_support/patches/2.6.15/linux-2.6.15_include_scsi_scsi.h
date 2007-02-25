--- linux-2.6.15.vanilla/include/scsi/scsi.h	2006-01-03 04:21:10.000000000 +0100
+++ linux-2.6.15/include/scsi/scsi.h	2007-02-25 02:52:27.000000000 +0100
@@ -198,6 +198,13 @@
 #define VOLUME_OVERFLOW     0x0d
 #define MISCOMPARE          0x0e
 
+/* Values for T10/04-262r7 */
+#define	ATA_16		      0x85	/* 16-byte pass-thru */
+#define	ATA_12		      0xa1	/* 12-byte pass-thru */
+
+/* Values for T10/04-262r7 */
+#define	ATA_16		      0x85	/* 16-byte pass-thru */
+#define	ATA_12		      0xa1	/* 12-byte pass-thru */
 
 /*
  *  DEVICE TYPES
