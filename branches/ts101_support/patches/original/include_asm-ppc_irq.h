--- linux-2.6.12.3/include/asm-ppc/irq.h	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/include/asm-ppc/irq.h	2007-02-17 14:10:02.000000000 -0500
@@ -300,6 +300,8 @@
 
 #ifndef CONFIG_8260
 
+// 2005.09.06, JohnsonCheng Fixed USB device Oops bug
+//#define NUM_8259_INTERRUPTS	0
 #define NUM_8259_INTERRUPTS	16
 
 #else /* CONFIG_8260 */
