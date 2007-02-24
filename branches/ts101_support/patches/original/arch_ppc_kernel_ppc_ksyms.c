--- linux-2.6.12.3/arch/ppc/kernel/ppc_ksyms.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/arch/ppc/kernel/ppc_ksyms.c	2007-02-17 14:10:23.000000000 -0500
@@ -70,6 +70,11 @@
 long long __lshrdi3(long long, int);
 
 extern unsigned long mm_ptov (unsigned long paddr);
+// 2005.10.27, Johnson Cheng, Export for qraid1 module
+extern void* sys_call_table;
+extern char* full_d_path(struct dentry* d, struct vfsmount* mnt, char* buf, int len);
+extern struct file fastcall *fget_light(unsigned int fd, int *fput_needed);
+// 2005.10.27, Johnson Cheng End
 
 EXPORT_SYMBOL(clear_pages);
 EXPORT_SYMBOL(clear_user_page);
@@ -348,3 +353,8 @@
 EXPORT_SYMBOL(__mtdcr);
 EXPORT_SYMBOL(__mfdcr);
 #endif
+// 2005.10.27, Johnson Cheng, Export for qraid1 module
+EXPORT_SYMBOL(sys_call_table);
+EXPORT_SYMBOL(full_d_path);
+EXPORT_SYMBOL(fget_light);
+// 2005.10.27, Johnson Cheng End
