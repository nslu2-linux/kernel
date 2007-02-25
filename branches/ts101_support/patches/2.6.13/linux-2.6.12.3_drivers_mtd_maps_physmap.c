--- linux-2.6.12.3/drivers/mtd/maps/physmap.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/mtd/maps/physmap.c	2007-02-17 14:10:16.000000000 -0500
@@ -29,6 +29,46 @@
 	.bankwidth = CONFIG_MTD_PHYSMAP_BANKWIDTH,
 };
 
+static struct mtd_partition TS101_partitions[] = {
+	{
+	  .name = "U-Boot",
+	  .offset = 0x00F00000,
+	  .size = 0x00040000,
+	  .mask_flags = MTD_WRITEABLE,  /* force read-only */
+	},
+	{
+	  .name = "Kernel",             /* default kernel image */
+	  .offset = 0x00000000,
+	  .size = 0x00200000,
+	},
+	{
+	  .name = "RootFS1",
+	  .offset = 0x00200000,
+	  .size = 0x00900000,
+	},
+	{
+	  .name = "RootFS2",
+	  .offset = 0x00b00000,
+	  .size = 0x00300000,
+	},
+	{
+	  .name = "Vendor",
+	  .offset = 0x00E00000,
+	  .size = 0x00100000,
+	},
+	{
+	  .name = "U-Boot Config",
+	  .offset = 0x00F40000,
+	  .size = 0x00020000,
+	},
+	{
+	  .name = "NAS Config",
+	  .offset = 0x00F60000,
+	  .size = 0x000A0000,
+	}
+};
+
+
 #ifdef CONFIG_MTD_PARTITIONS
 static struct mtd_partition *mtd_parts;
 static int                   mtd_parts_nb;
@@ -78,6 +118,7 @@
 			return 0;
 		}
 
+		physmap_set_partitions(TS101_partitions, 7);
 		if (num_physmap_partitions != 0) 
 		{
 			printk(KERN_NOTICE 
