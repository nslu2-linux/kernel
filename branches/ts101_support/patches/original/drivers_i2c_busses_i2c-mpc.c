--- linux-2.6.12.3/drivers/i2c/busses/i2c-mpc.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/i2c/busses/i2c-mpc.c	2007-02-17 14:10:16.000000000 -0500
@@ -128,6 +128,7 @@
 	}
 
 	if (writing && (x & CSR_RXAK)) {
+		printk("I2C: No RXAK\n");
 		pr_debug("I2C: No RXAK\n");
 		/* generate stop */
 		writeccr(i2c, CCR_MEN);
