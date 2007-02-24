--- linux-2.6.12.3/arch/ppc/syslib/todc_time.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/arch/ppc/syslib/todc_time.c	2007-02-17 14:10:23.000000000 -0500
@@ -461,7 +461,7 @@
 	ulong	tbl, tbu;
         long	i, loop_count;
         u_char	sec;
-
+#if 0	// JohnsonCheng
 	todc_time_init();
 
 	/*
@@ -502,7 +502,8 @@
 
 		freq -= tbl;
 	} while ((get_tbu() != tbu) && (++loop_count < 2));
-
+#endif
+freq = 33000000;
 	printk("time_init: decrementer frequency = %lu.%.6lu MHz\n",
 	       freq/1000000, freq%1000000);
 
