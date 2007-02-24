--- linux-2.6.12.3/drivers/net/e1000/e1000_main.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/drivers/net/e1000/e1000_main.c	2007-02-17 14:10:12.000000000 -0500
@@ -25,7 +25,8 @@
   Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 
 *******************************************************************************/
-
+#include <platforms/qnap.h>
+#include <platforms/qnap_pic_user.h>
 #include "e1000.h"
 
 /* Change Log
@@ -610,7 +611,7 @@
 	e1000_reset_hw(&adapter->hw);
 
 	/* make sure the EEPROM is good */
-
+#if 0	// 2005.08.02, JohnsonCheng
 	if(e1000_validate_eeprom_checksum(&adapter->hw) < 0) {
 		DPRINTK(PROBE, ERR, "The EEPROM Checksum Is Not Valid\n");
 		err = -EIO;
@@ -621,6 +622,13 @@
 
 	if(e1000_read_mac_addr(&adapter->hw))
 		DPRINTK(PROBE, ERR, "EEPROM Read Error\n");
+#endif
+	adapter->hw.mac_addr[0] = 0;
+	adapter->hw.mac_addr[1] = 0x33;
+	adapter->hw.mac_addr[2] = 0x44;
+	adapter->hw.mac_addr[3] = 0x55;
+	adapter->hw.mac_addr[4] = 0x66;
+	adapter->hw.mac_addr[5] = 0x77;
 	memcpy(netdev->dev_addr, adapter->hw.mac_addr, netdev->addr_len);
 
 	if(!is_valid_ether_addr(netdev->dev_addr)) {
@@ -1769,6 +1777,7 @@
 	struct net_device *netdev = adapter->netdev;
 	struct e1000_desc_ring *txdr = &adapter->tx_ring;
 	uint32_t link;
+	static uint32_t first = 1;
 
 	e1000_check_for_link(&adapter->hw);
 	if (adapter->hw.mac_type == e1000_82573) {
@@ -1793,6 +1802,14 @@
 			       adapter->link_speed,
 			       adapter->link_duplex == FULL_DUPLEX ?
 			       "Full Duplex" : "Half Duplex");
+			/*--add by KenChen for GIGA Lan up notification--
+			if(adapter->link_speed == GIGA)	
+				send_message_to_app(QNAP_GIGA_LAN_UP);
+			else
+				send_message_to_app(QNAP_NOT_GIGA_LAN_UP);
+			--end here--*/
+			if(!first)
+				send_message_to_app(QNAP_NET_NIC_UP);
 
 			netif_carrier_on(netdev);
 			netif_wake_queue(netdev);
@@ -1800,10 +1817,16 @@
 			adapter->smartspeed = 0;
 		}
 	} else {
-		if(netif_carrier_ok(netdev)) {
+		if(first)
+		{
+			DPRINTK(LINK, INFO, "NIC Link is Down\n");
+			send_message_to_app(QNAP_NET_NIC_DOWN);
+		}
+		else if(netif_carrier_ok(netdev)) {
 			adapter->link_speed = 0;
 			adapter->link_duplex = 0;
 			DPRINTK(LINK, INFO, "NIC Link is Down\n");
+			send_message_to_app(QNAP_NET_NIC_DOWN);
 			netif_carrier_off(netdev);
 			netif_stop_queue(netdev);
 			mod_timer(&adapter->phy_info_timer, jiffies + 2 * HZ);
@@ -1811,6 +1834,7 @@
 
 		e1000_smartspeed(adapter);
 	}
+	first = 0;
 
 	e1000_update_stats(adapter);
 
@@ -3373,6 +3397,7 @@
 {
 	struct e1000_adapter *adapter = netdev->priv;
 	struct mii_ioctl_data *data = if_mii(ifr);
+	unsigned long flags;
 	int retval;
 	uint16_t mii_reg;
 	uint16_t spddplx;
@@ -3387,9 +3412,13 @@
 	case SIOCGMIIREG:
 		if (!capable(CAP_NET_ADMIN))
 			return -EPERM;
+		spin_lock_irqsave(&adapter->stats_lock, flags);
 		if (e1000_read_phy_reg(&adapter->hw, data->reg_num & 0x1F,
-				   &data->val_out))
+				   &data->val_out)) {
+			spin_unlock_irqrestore(&adapter->stats_lock, flags);
 			return -EIO;
+		}
+		spin_unlock_irqrestore(&adapter->stats_lock, flags);
 		break;
 	case SIOCSMIIREG:
 		if (!capable(CAP_NET_ADMIN))
