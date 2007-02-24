--- linux-2.6.12.3/arch/ppc/syslib/mpc10x_common.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/arch/ppc/syslib/mpc10x_common.c	2007-02-17 14:10:23.000000000 -0500
@@ -503,8 +503,11 @@
 	/* Skip reserved space and map i2c and DMA Ch[01] */
 	openpic_set_sources(EPIC_IRQ_BASE, 3, OpenPIC_Addr + 0x11020);
 	/* Skip reserved space and map Message Unit Interrupt (I2O) */
-	openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x110C0);
-
+	//openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x110C0);
+#ifdef CONFIG_SANDPOINT
+	openpic_set_sources(EPIC_IRQ_BASE + 3, 1, OpenPIC_Addr + 0x11140);
+	openpic_set_sources(EPIC_IRQ_BASE + 4, 1, OpenPIC_Addr + 0x11120);
+#endif
 	openpic_init(NUM_8259_INTERRUPTS);
 }
 #endif
