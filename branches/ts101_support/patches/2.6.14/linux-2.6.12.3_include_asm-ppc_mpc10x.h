Index: linux-2.6.13/include/asm-ppc/mpc10x.h
===================================================================
--- linux-2.6.13.orig/include/asm-ppc/mpc10x.h	2005-08-28 19:41:01.000000000 -0400
+++ linux-2.6.13/include/asm-ppc/mpc10x.h	2007-02-24 19:17:58.000000000 -0500
@@ -157,7 +157,8 @@
  */
 extern unsigned long			ioremap_base;
 #define	MPC10X_MAPA_EUMB_BASE		(ioremap_base - MPC10X_EUMB_SIZE)
-#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+//#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+#define	MPC10X_MAPB_EUMB_BASE		0xfdf00000	// 2005.08.02, JohnsonCheng
 
 enum ppc_sys_devices {
 	MPC10X_IIC1,
