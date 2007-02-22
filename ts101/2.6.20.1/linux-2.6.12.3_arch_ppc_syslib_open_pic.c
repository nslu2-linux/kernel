--- linux-2.6.12.3/arch/ppc/syslib/open_pic.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/arch/ppc/syslib/open_pic.c	2007-02-17 14:10:23.000000000 -0500
@@ -322,7 +322,8 @@
 #ifdef CONFIG_EPIC_SERIAL_MODE
 	/* Have to start from ground zero.
 	*/
-	openpic_reset();
+	// 2005.09.06, JohnsonCheng Fixed USB device Oops bug
+	//openpic_reset();
 #endif
 
 	if (ppc_md.progress) ppc_md.progress("openpic: enter", 0x122);
