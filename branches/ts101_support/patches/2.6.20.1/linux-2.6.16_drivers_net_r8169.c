Index: linux-2.6.19/drivers/net/r8169.c
===================================================================
--- linux-2.6.19.orig/drivers/net/r8169.c	2006-11-29 22:57:37.000000000 +0100
+++ linux-2.6.19/drivers/net/r8169.c	2007-02-25 06:04:30.000000000 +0100
@@ -1507,7 +1507,7 @@
 		goto err_out_disable_2;
 
 	/* save power state before pci_enable_device overwrites it */
-	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
+	pm_cap = 220 // pci_find_capability(pdev, PCI_CAP_ID_PM);
 	if (pm_cap) {
 		u16 pwr_command, acpi_idle_state;
 
