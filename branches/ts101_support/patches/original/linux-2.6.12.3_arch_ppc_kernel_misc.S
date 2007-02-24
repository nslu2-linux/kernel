--- linux-2.6.12.3/arch/ppc/kernel/misc.S	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/arch/ppc/kernel/misc.S	2007-02-17 14:10:23.000000000 -0500
@@ -1441,3 +1441,4 @@
 	.long sys_request_key		/* 270 */
 	.long sys_keyctl
 	.long sys_waitid
+	.long sys_qnap_rmdir
