Index: linux-2.6.13/arch/ppc/syslib/open_pic.c
===================================================================
--- linux-2.6.13.orig/arch/ppc/syslib/open_pic.c	2005-08-28 19:41:01.000000000 -0400
+++ linux-2.6.13/arch/ppc/syslib/open_pic.c	2007-02-24 19:12:08.000000000 -0500
@@ -321,7 +321,8 @@
 #ifdef CONFIG_EPIC_SERIAL_MODE
 	/* Have to start from ground zero.
 	*/
-	openpic_reset();
+	// 2005.09.06, JohnsonCheng Fixed USB device Oops bug
+	//openpic_reset();
 #endif
 
 	if (ppc_md.progress) ppc_md.progress("openpic: enter", 0x122);
