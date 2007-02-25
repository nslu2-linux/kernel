Index: linux-2.6.13/drivers/net/r8169.c
===================================================================
--- linux-2.6.13.orig/drivers/net/r8169.c	2005-08-28 19:41:01.000000000 -0400
+++ linux-2.6.13/drivers/net/r8169.c	2007-02-24 19:17:49.000000000 -0500
@@ -184,6 +184,8 @@
 #undef _R
 
 static struct pci_device_id rtl8169_pci_tbl[] = {
+	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8129), },
+	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8167), },
 	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8169), },
 	{ PCI_DEVICE(PCI_VENDOR_ID_DLINK,	0x4300), },
 	{ PCI_DEVICE(0x16ec,			0x0116), },
@@ -1336,7 +1338,7 @@
 		goto err_out_disable;
 
 	/* save power state before pci_enable_device overwrites it */
-	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
+	pm_cap = 220 // pci_find_capability(pdev, PCI_CAP_ID_PM);
 	if (pm_cap) {
 		u16 pwr_command;
 
