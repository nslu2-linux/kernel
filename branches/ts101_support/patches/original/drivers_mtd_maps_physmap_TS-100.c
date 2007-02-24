--- linux-2.6.12.3/drivers/mtd/maps/physmap_TS-100.c	1969-12-31 19:00:00.000000000 -0500
+++ linux-2.6.12.3-qnap/drivers/mtd/maps/physmap_TS-100.c	2007-02-17 14:10:16.000000000 -0500
@@ -0,0 +1,202 @@
+/*
+ * $Id: physmap_TS-100.c,v 1.2 2006/10/26 10:09:18 ken Exp $
+ *
+ * Normal mappings of chips in physical memory
+ *
+ * Copyright (C) 2003 MontaVista Software Inc.
+ * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
+ *
+ * 031022 - [jsun] add run-time configure and partition setup
+ */
+
+#include <linux/module.h>
+#include <linux/types.h>
+#include <linux/kernel.h>
+#include <linux/init.h>
+#include <linux/slab.h>
+#include <asm/io.h>
+#include <linux/mtd/mtd.h>
+#include <linux/mtd/map.h>
+#include <linux/config.h>
+#include <linux/mtd/partitions.h>
+
+static struct mtd_info *mymtd;
+
+struct map_info physmap_map = {
+	.name = "phys_mapped_flash",
+	.phys = CONFIG_MTD_PHYSMAP_START,
+	.size = CONFIG_MTD_PHYSMAP_LEN,
+	.bankwidth = CONFIG_MTD_PHYSMAP_BANKWIDTH,
+};
+/*
+static struct mtd_partition TS101_partitions[] = {
+	{
+	  .name = "U-Boot",
+	  .offset = 0x00F00000,
+	  .size = 0x00040000,
+	  .mask_flags = MTD_WRITEABLE,  // force read-only //
+	},
+	{
+	  .name = "Kernel",             // default kernel image //
+	  .offset = 0x00000000,
+	  .size = 0x00200000,
+	},
+	{
+	  .name = "RootFS1",
+	  .offset = 0x00200000,
+	  .size = 0x00300000,
+	},
+	{
+	  .name = "RootFS2",
+	  .offset = 0x00500000,
+	  .size = 0x00900000,
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
+*/
+//MTD partition for TS-100, Ken Chen 20060920 
+static struct mtd_partition TS100_partitions[] = {
+	{
+	  .name = "U-Boot",
+	  .offset = 0x00300000,
+	  .size = 0x00040000,
+	  .mask_flags = MTD_WRITEABLE,  // force read-only //
+	},
+	{
+	  .name = "Kernel",             // default kernel image //
+	  .offset = 0x00000000,
+	  .size = 0x00200000,
+	},
+	{
+	  .name = "RootFS1",
+	  .offset = 0x00200000,
+	  .size = 0x00100000,
+	},
+	{
+	  .name = "U-Boot Config",
+	  .offset = 0x00340000,
+	  .size = 0x00010000,
+	},
+	{
+	  .name = "RootFS2",
+	  .offset = 0x00350000,
+	  .size = 0x000A0000,
+	},
+	{
+	  .name = "NAS Config",
+	  .offset = 0x003F0000,
+	  .size = 0x00010000,
+	}
+};
+
+
+#ifdef CONFIG_MTD_PARTITIONS
+static struct mtd_partition *mtd_parts;
+static int                   mtd_parts_nb;
+
+static int num_physmap_partitions;
+static struct mtd_partition *physmap_partitions;
+
+static const char *part_probes[] __initdata = {"cmdlinepart", "RedBoot", NULL};
+
+void physmap_set_partitions(struct mtd_partition *parts, int num_parts)
+{
+	physmap_partitions=parts;
+	num_physmap_partitions=num_parts;
+}
+#endif /* CONFIG_MTD_PARTITIONS */
+
+static int __init init_physmap(void)
+{
+	static const char *rom_probe_types[] = { "cfi_probe", "jedec_probe", "map_rom", NULL };
+	const char **type;
+
+       	printk(KERN_NOTICE "physmap flash device: %lx at %lx\n", physmap_map.size, physmap_map.phys);
+	physmap_map.virt = ioremap(physmap_map.phys, physmap_map.size);
+
+	if (!physmap_map.virt) {
+		printk("Failed to ioremap\n");
+		return -EIO;
+	}
+
+	simple_map_init(&physmap_map);
+
+	mymtd = NULL;
+	type = rom_probe_types;
+	for(; !mymtd && *type; type++) {
+		mymtd = do_map_probe(*type, &physmap_map);
+	}
+	if (mymtd) {
+		mymtd->owner = THIS_MODULE;
+
+#ifdef CONFIG_MTD_PARTITIONS
+		mtd_parts_nb = parse_mtd_partitions(mymtd, part_probes, 
+						    &mtd_parts, 0);
+
+		if (mtd_parts_nb > 0)
+		{
+			add_mtd_partitions (mymtd, mtd_parts, mtd_parts_nb);
+			return 0;
+		}
+
+		//physmap_set_partitions(TS101_partitions, 7);
+		physmap_set_partitions(TS100_partitions, 6);
+		if (num_physmap_partitions != 0) 
+		{
+			printk(KERN_NOTICE 
+			       "Using physmap partition definition\n");
+			add_mtd_partitions (mymtd, physmap_partitions, num_physmap_partitions);
+			return 0;
+		}
+
+#endif
+		add_mtd_device(mymtd);
+
+		return 0;
+	}
+
+	iounmap(physmap_map.virt);
+	return -ENXIO;
+}
+
+static void __exit cleanup_physmap(void)
+{
+#ifdef CONFIG_MTD_PARTITIONS
+	if (mtd_parts_nb) {
+		del_mtd_partitions(mymtd);
+		kfree(mtd_parts);
+	} else if (num_physmap_partitions) {
+		del_mtd_partitions(mymtd);
+	} else {
+		del_mtd_device(mymtd);
+	}
+#else
+	del_mtd_device(mymtd);
+#endif
+	map_destroy(mymtd);
+
+	iounmap(physmap_map.virt);
+	physmap_map.virt = NULL;
+}
+
+module_init(init_physmap);
+module_exit(cleanup_physmap);
+
+
+MODULE_LICENSE("GPL");
+MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org>");
+MODULE_DESCRIPTION("Generic configurable MTD map driver");
