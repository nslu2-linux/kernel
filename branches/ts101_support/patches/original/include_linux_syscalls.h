--- linux-2.6.12.3/include/linux/syscalls.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/linux/syscalls.h	2007-02-17 14:10:09.000000000 -0500
@@ -505,4 +505,6 @@
 asmlinkage long sys_keyctl(int cmd, unsigned long arg2, unsigned long arg3,
 			   unsigned long arg4, unsigned long arg5);
 
+//add by Jimmy to add a syscall
+asmlinkage long sys_qnap_rmdir(const char __user *pathname);
 #endif
