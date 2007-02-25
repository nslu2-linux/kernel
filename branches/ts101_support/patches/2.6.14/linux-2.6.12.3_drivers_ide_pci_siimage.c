--- linux-2.6.12.3/drivers/ide/pci/siimage.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/ide/pci/siimage.c	2007-02-17 14:10:15.000000000 -0500
@@ -47,6 +47,7 @@
 	switch(pdev->device)
 	{
 		case PCI_DEVICE_ID_SII_3112:
+		case PCI_DEVICE_ID_SII_3512:
 		case PCI_DEVICE_ID_SII_1210SA:
 			return 1;
 		case PCI_DEVICE_ID_SII_680:
@@ -1088,7 +1089,8 @@
 static ide_pci_device_t siimage_chipsets[] __devinitdata = {
 	/* 0 */ DECLARE_SII_DEV("SiI680"),
 	/* 1 */ DECLARE_SII_DEV("SiI3112 Serial ATA"),
-	/* 2 */ DECLARE_SII_DEV("Adaptec AAR-1210SA")
+	/* 2 */ DECLARE_SII_DEV("SiI3512 Serial ATA"),
+	/* 3 */ DECLARE_SII_DEV("Adaptec AAR-1210SA")
 };
 
 /**
@@ -1109,7 +1111,8 @@
 	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_680,  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
 #ifdef CONFIG_BLK_DEV_IDE_SATA
 	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_3112, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 1},
-	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_1210SA, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2},
+	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_3512, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2},
+	{ PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_SII_1210SA, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 3},
 #endif
 	{ 0, },
 };
