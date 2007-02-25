Index: linux-2.6.13/arch/ppc/syslib/mpc10x_common.c
===================================================================
--- linux-2.6.13.orig/arch/ppc/syslib/mpc10x_common.c	2005-08-28 19:41:01.000000000 -0400
+++ linux-2.6.13/arch/ppc/syslib/mpc10x_common.c	2007-02-24 19:12:04.000000000 -0500
@@ -647,9 +647,11 @@
 	/* Skip reserved space and map i2c and DMA Ch[01] */
 	openpic_set_sources(EPIC_IRQ_BASE, 3, OpenPIC_Addr + 0x11020);
 	/* Skip reserved space and map Message Unit Interrupt (I2O) */
-	openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x110C0);
-	/* Skip reserved space and map Serial Interupts */
-	openpic_set_sources(EPIC_IRQ_BASE + 4, 2, OpenPIC_Addr + 0x11120);
+	//openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x110C0);
+#ifdef CONFIG_SANDPOINT
+	openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x11140);
+	openpic_set_sources(EPIC_IRQ_BASE + 4, 1, OpenPIC_Addr + 0x11120);
+#endif
 
 	openpic_init(NUM_8259_INTERRUPTS);
 }
