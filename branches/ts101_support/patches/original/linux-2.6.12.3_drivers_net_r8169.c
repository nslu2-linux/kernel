--- linux-2.6.12.3/drivers/net/r8169.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/net/r8169.c	2007-02-17 14:10:09.000000000 -0500
@@ -69,6 +69,10 @@
 #include <asm/io.h>
 #include <asm/irq.h>
 
+#include <platforms/qnap.h>
+#include <platforms/qnap_pic.h>
+#include <platforms/qnap_pic_user.h>
+
 #define RTL8169_VERSION "2.2LK"
 #define MODULENAME "r8169"
 #define PFX MODULENAME ": "
@@ -119,6 +123,7 @@
 #define EarlyTxThld 	0x3F	/* 0x3F means NO early transmit */
 #define RxPacketMaxSize	0x3FE8	/* 16K - 1 - ETH_HLEN - VLAN - CRC... */
 #define SafeMtu		0x1c20	/* ... actually life sucks beyond ~7k */
+#define MaxMtu		0x2710	/* Max mtu size*/
 #define InterFrameGap	0x03	/* 3 means InterFrameGap = the shortest one */
 
 #define R8169_REGS_SIZE		256
@@ -174,6 +179,8 @@
 #undef _R
 
 static struct pci_device_id rtl8169_pci_tbl[] = {
+	{0x10ec, 0x8129, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
+	{0x10ec, 0x8167, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
 	{0x10ec, 0x8169, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
 	{0x1186, 0x4300, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
 	{0,},
@@ -543,9 +550,13 @@
 	spin_lock_irqsave(&tp->lock, flags);
 	if (tp->link_ok(ioaddr)) {
 		netif_carrier_on(dev);
+		send_message_to_app(QNAP_NET_NIC_UP);
 		printk(KERN_INFO PFX "%s: link up\n", dev->name);
-	} else
+	} else{
 		netif_carrier_off(dev);
+		send_message_to_app(QNAP_NET_NIC_DOWN);
+		printk(KERN_INFO PFX "%s: link down\n", dev->name);
+	}
 	spin_unlock_irqrestore(&tp->lock, flags);
 }
 
@@ -1189,7 +1200,8 @@
 		goto err_out_disable;
 
 	/* save power state before pci_enable_device overwrites it */
-	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
+//	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
+	pm_cap = 220;
 	if (pm_cap) {
 		u16 pwr_command;
 
@@ -1301,6 +1313,37 @@
 	goto out;
 }
 
+//Ricky added for set mac address ioctl operation
+static int
+rtl8169_set_mac(struct net_device *netdev, void *p)
+{
+	struct rtl8169_private *priv = netdev->priv;
+	struct sockaddr *addr = p;
+	void __iomem *ioaddr = priv->mmio_addr;
+
+	if(!is_valid_ether_addr(addr->sa_data)){
+		dprintk("Invalid mac address\n");
+                return -EADDRNOTAVAIL;
+	}
+
+	dprintk("Now mac address\n");
+	dprintk("%2.2X:%2.2X:%2.2X:%2.2x:%2.2X:%2.2X\n", netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2], netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5]);
+
+	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
+
+	dprintk("Set mac address\n");
+	dprintk("%2.2X:%2.2X:%2.2X:%2.2x:%2.2X:%2.2X\n", netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2], netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5]);
+
+	RTL_W8(Cfg9346, Cfg9346_Unlock);
+	RTL_W32 (MAC0 + 0, cpu_to_le32 (*(u32 *) (netdev->dev_addr + 0)));
+        RTL_W32 (MAC0 + 4, cpu_to_le32 (*(u32 *) (netdev->dev_addr + 4)));
+	RTL_W8(Cfg9346, Cfg9346_Lock);
+
+	return 0;
+}
+
+//End
+
 static int __devinit
 rtl8169_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
 {
@@ -1311,7 +1354,8 @@
 	static int printed_version = 0;
 	u8 autoneg, duplex;
 	u16 speed;
-	int i, rc;
+//	int i, rc;
+	int rc;
 
 	assert(pdev != NULL);
 	assert(ent != NULL);
@@ -1348,8 +1392,23 @@
 	}
 
 	/* Get MAC address.  FIXME: read EEPROM */
-	for (i = 0; i < MAC_ADDR_LEN; i++)
-		dev->dev_addr[i] = RTL_R8(MAC0 + i);
+//	for (i = 0; i < MAC_ADDR_LEN; i++)
+//		dev->dev_addr[i] = RTL_R8(MAC0 + i);
+//Ricky added - for QNAP default mac address
+	
+	dev->dev_addr[0]=0;
+	dev->dev_addr[1]=0x33;
+	dev->dev_addr[2]=0x44;
+	dev->dev_addr[3]=0x55;
+	dev->dev_addr[4]=0x66;
+	dev->dev_addr[5]=0x77;
+
+	RTL_W8(Cfg9346, Cfg9346_Unlock);
+        RTL_W32(MAC0 + 0, cpu_to_le32 (*(u32 *) (dev->dev_addr + 0)));
+        RTL_W32(MAC0 + 4, cpu_to_le32 (*(u32 *) (dev->dev_addr + 4)));
+	RTL_W8(Config1, 0x40);  //Open LED for 100/1000
+	RTL_W8(Cfg9346, Cfg9346_Lock);
+//End
 
 	dev->open = rtl8169_open;
 	dev->hard_start_xmit = rtl8169_start_xmit;
@@ -1358,6 +1417,7 @@
 	dev->stop = rtl8169_close;
 	dev->tx_timeout = rtl8169_tx_timeout;
 	dev->set_multicast_list = rtl8169_set_rx_mode;
+	dev->set_mac_address = rtl8169_set_mac;
 	dev->watchdog_timeo = RTL8169_TX_TIMEOUT;
 	dev->irq = pdev->irq;
 	dev->base_addr = (unsigned long) ioaddr;
@@ -1640,6 +1700,8 @@
 	int ret = 0;
 
 	if (new_mtu < ETH_ZLEN || new_mtu > SafeMtu)
+//	if (new_mtu < ETH_ZLEN || new_mtu > MaxMtu)
+
 		return -EINVAL;
 
 	dev->mtu = new_mtu;
@@ -2313,8 +2375,9 @@
 		RTL_W16(IntrMask, rtl8169_intr_mask & ~rtl8169_napi_event);
 		tp->intr_mask = ~rtl8169_napi_event;
 
-		if (likely(netif_rx_schedule_prep(dev)))
+		if (likely(netif_rx_schedule_prep(dev))){
 			__netif_rx_schedule(dev);
+}
 		else {
 			printk(KERN_INFO "%s: interrupt %04x taken in poll\n",
 			       dev->name, status);	
@@ -2329,7 +2392,6 @@
 		if (status & (TxOK | TxErr))
 			rtl8169_tx_interrupt(dev, tp, ioaddr);
 #endif
-
 		boguscnt--;
 	} while (boguscnt > 0);
 
