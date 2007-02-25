Index: linux-2.6.14/arch/ppc/syslib/open_pic.c
===================================================================
--- linux-2.6.14.orig/arch/ppc/syslib/open_pic.c	2005-10-27 20:02:08.000000000 -0400
+++ linux-2.6.14/arch/ppc/syslib/open_pic.c	2007-02-24 21:04:45.000000000 -0500
@@ -320,7 +320,8 @@
 #ifdef CONFIG_EPIC_SERIAL_MODE
 	/* Have to start from ground zero.
 	*/
-	openpic_reset();
+	// 2005.09.06, JohnsonCheng Fixed USB device Oops bug
+	//openpic_reset();
 #endif
 
 	if (ppc_md.progress) ppc_md.progress("openpic: enter", 0x122);
