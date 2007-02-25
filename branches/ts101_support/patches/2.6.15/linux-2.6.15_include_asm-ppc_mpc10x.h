--- linux-2.6.15.vanilla/include/asm-ppc/mpc10x.h	2006-01-03 04:21:10.000000000 +0100
+++ linux-2.6.15/include/asm-ppc/mpc10x.h	2007-02-25 02:52:22.000000000 +0100
@@ -157,7 +157,8 @@
  */
 extern unsigned long			ioremap_base;
 #define	MPC10X_MAPA_EUMB_BASE		(ioremap_base - MPC10X_EUMB_SIZE)
-#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+//#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+#define	MPC10X_MAPB_EUMB_BASE		0xfdf00000	// 2005.08.02, JohnsonCheng
 
 enum ppc_sys_devices {
 	MPC10X_IIC1,
