--- linux-2.6.12.3/include/asm-ppc/unistd.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/asm-ppc/unistd.h	2007-02-17 14:10:02.000000000 -0500
@@ -278,7 +278,9 @@
 #define __NR_keyctl		271
 #define __NR_waitid		272
 
-#define __NR_syscalls		273
+#define __NR_qnap_rmdir		273
+
+#define __NR_syscalls		274
 
 #define __NR(n)	#n
 
