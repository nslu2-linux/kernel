--- linux-2.6.15.vanilla/drivers/ide/pci/siimage.c	2006-01-03 04:21:10.000000000 +0100
+++ linux-2.6.15/drivers/ide/pci/siimage.c	2007-02-25 02:50:34.000000000 +0100
@@ -53,6 +53,7 @@
 	switch(pdev->device)
 	{
 		case PCI_DEVICE_ID_SII_3112:
+		case PCI_DEVICE_ID_SII_3512:
 		case PCI_DEVICE_ID_SII_1210SA:
 			return 1;
 		case PCI_DEVICE_ID_SII_680:
@@ -1103,7 +1104,8 @@
 static ide_pci_device_t siimage_chipsets[] __devinitdata = {
 	/* 0 */ DECLARE_SII_DEV("SiI680"),
 	/* 1 */ DECLARE_SII_DEV("SiI3112 Serial ATA"),
-	/* 2 */ DECLARE_SII_DEV("Adaptec AAR-1210SA")
+	/* 2 */ DECLARE_SII_DEV("SiI3512 Serial ATA"),
+	/* 3 */ DECLARE_SII_DEV("Adaptec AAR-1210SA")
 };
 
 /**
@@ -1124,7 +1126,8 @@
 	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_680,  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
 #ifdef CONFIG_BLK_DEV_IDE_SATA
 	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_3112, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 1},
-	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_1210SA, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2},
+	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_3512, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2},
+	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_1210SA, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 3},
 #endif
 	{ 0, },
 };
