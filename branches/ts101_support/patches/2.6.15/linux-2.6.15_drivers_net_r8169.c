--- linux-2.6.15.vanilla/drivers/net/r8169.c	2006-01-03 04:21:10.000000000 +0100
+++ linux-2.6.15/drivers/net/r8169.c	2007-02-25 02:52:17.000000000 +0100
@@ -183,6 +183,8 @@
 #undef _R
 
 static struct pci_device_id rtl8169_pci_tbl[] = {
+	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8129), },
+	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8167), },
 	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8169), },
 	{ PCI_DEVICE(PCI_VENDOR_ID_DLINK,	0x4300), },
 	{ PCI_DEVICE(0x16ec,			0x0116), },
@@ -1337,7 +1339,7 @@
 		goto err_out_disable;
 
 	/* save power state before pci_enable_device overwrites it */
-	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
+	pm_cap = 220 // pci_find_capability(pdev, PCI_CAP_ID_PM);
 	if (pm_cap) {
 		u16 pwr_command;
 
