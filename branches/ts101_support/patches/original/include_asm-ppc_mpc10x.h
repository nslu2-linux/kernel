--- linux-2.6.12.3/include/asm-ppc/mpc10x.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/asm-ppc/mpc10x.h	2007-02-17 14:10:02.000000000 -0500
@@ -157,7 +157,8 @@
  */
 extern unsigned long			ioremap_base;
 #define	MPC10X_MAPA_EUMB_BASE		(ioremap_base - MPC10X_EUMB_SIZE)
-#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+//#define	MPC10X_MAPB_EUMB_BASE		MPC10X_MAPA_EUMB_BASE
+#define	MPC10X_MAPB_EUMB_BASE		0xfdf00000	// 2005.08.02, JohnsonCheng
 
 
 int mpc10x_bridge_init(struct pci_controller *hose,
