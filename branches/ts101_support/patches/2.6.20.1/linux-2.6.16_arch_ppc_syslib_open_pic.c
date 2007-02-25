--- linux-2.6.16.vanilla/arch/ppc/syslib/open_pic.c	2006-01-03 04:21:10.000000000 +0100
+++ linux-2.6.16/arch/ppc/syslib/open_pic.c	2007-02-25 05:09:57.000000000 +0100
@@ -321,7 +321,8 @@
 #ifdef CONFIG_EPIC_SERIAL_MODE
 	/* Have to start from ground zero.
 	*/
-	openpic_reset();
+	// 2005.09.06, JohnsonCheng Fixed USB device Oops bug
+	//openpic_reset();
 #endif
 
 	if (ppc_md.progress) ppc_md.progress("openpic: enter", 0x122);
