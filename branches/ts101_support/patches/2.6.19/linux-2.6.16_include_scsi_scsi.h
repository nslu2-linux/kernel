--- linux-2.6.16.vanilla/include/scsi/scsi.h	2007-02-25 05:35:08.000000000 +0100
+++ linux-2.6.16/include/scsi/scsi.h	2007-02-25 05:12:58.000000000 +0100
@@ -204,6 +204,13 @@
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
Binary files linux-2.6.16.vanilla/.tmp_gas_check and linux-2.6.16/.tmp_gas_check differ
