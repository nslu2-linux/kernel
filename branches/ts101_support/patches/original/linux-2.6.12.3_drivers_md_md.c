--- linux-2.6.12.3/drivers/md/md.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/md/md.c	2007-02-17 14:10:12.000000000 -0500
@@ -46,6 +46,12 @@
 
 #include <asm/unaligned.h>
 
+#ifdef CONFIG_SANDPOINT
+#include <linux/string.h>
+#include <platforms/qnap_pic.h>
+#include <platforms/qnap_pic_user.h>
+#endif
+
 #define MAJOR_NR MD_MAJOR
 #define MD_DRIVER
 
@@ -3356,6 +3362,19 @@
 			mdname(mddev));
 		mddev->curr_resync = j;
 	}
+	else{
+#ifdef CONFIG_SANDPOINT
+		if( !test_bit(MD_RECOVERY_SYNC, &mddev->recovery) && !strncmp(mdname(mddev), "md0", sizeof("md0"))){
+			printk("Rebuilding start: Should send command\n");
+			printk("md name: %s\n", mdname(mddev));
+			send_message_to_app(MD_REBUILDING);
+		}
+		else{
+			printk("else md name: %s\n", mdname(mddev));
+//		send_message_to_app(MD_REBUILDING);
+		}
+	}
+#endif
 
 	while (j < max_sectors) {
 		int sectors;
@@ -3424,6 +3443,14 @@
 		}
 	}
 	printk(KERN_INFO "md: %s: sync done.\n",mdname(mddev));
+#ifdef CONFIG_SANDPOINT
+        if( !test_bit(MD_RECOVERY_SYNC, &mddev->recovery) && !strncmp(mdname(mddev), "md0", sizeof("md0"))){
+
+                printk("Rebuilding done: Should send command\n");
+                send_message_to_app(MD_REBUILDING_DONE);
+        }
+#endif
+
 	/*
 	 * this also signals 'finished resyncing' to md_stop
 	 */
@@ -3453,6 +3480,13 @@
 	wake_up(&resync_wait);
 	set_bit(MD_RECOVERY_DONE, &mddev->recovery);
 	md_wakeup_thread(mddev->thread);
+#ifdef CONFIG_SANDPOINT
+	if(!strncmp(mdname(mddev), "md0", sizeof("md0"))){
+
+		printk("Rebuilding skip: Should send command\n");
+		send_message_to_app(MD_REBUILDING_SKIP);
+	}
+#endif
 }
 
 
