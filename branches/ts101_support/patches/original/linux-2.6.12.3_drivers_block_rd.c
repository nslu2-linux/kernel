--- linux-2.6.12.3/drivers/block/rd.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/block/rd.c	2007-02-17 14:10:17.000000000 -0500
@@ -461,7 +461,11 @@
 		disk->flags |= GENHD_FL_SUPPRESS_PARTITION_INFO;
 		sprintf(disk->disk_name, "ram%d", i);
 		sprintf(disk->devfs_name, "rd/%d", i);
-		set_capacity(disk, rd_size * 2);
+//Modified by Paul Chen
+		//set_capacity(disk, rd_size * 2);
+		if (i == 0) set_capacity(disk, 7168 * 2);
+		else set_capacity(disk, rd_size * 2);
+//Modified ends here
 		add_disk(rd_disks[i]);
 	}
 
