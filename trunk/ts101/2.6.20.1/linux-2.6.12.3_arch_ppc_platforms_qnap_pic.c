--- linux-2.6.12.3/arch/ppc/platforms/qnap_pic.c	1969-12-31 19:00:00.000000000 -0500
+++ linux-2.6.12.3-qnap/arch/ppc/platforms/qnap_pic.c	2007-02-17 14:10:23.000000000 -0500
@@ -0,0 +1,287 @@
+#include <linux/config.h>
+#include <linux/module.h>
+#include <linux/poll.h>
+#include <linux/miscdevice.h>
+#include <linux/serial_core.h>
+#include <linux/serial_reg.h>
+#include <asm/serial.h>
+#include <asm/io.h>
+
+#include "qnap_pic.h"
+#include "qnap_pic_user.h"
+
+static wait_queue_head_t pic_wait;
+static wait_queue_head_t queue_empty_wait;
+static int use_int = 0;
+static int tx_begin = 0, tx_end = 0, rx_begin = 0, rx_end = 0;
+static unsigned char *tx_buf = NULL;
+static unsigned char rx_buf[QUEUE_BUFSIZE];
+static int usage = 0;
+int usb_button_enabled = 1;
+
+static struct qnap_pic_event pic_event[QNAP_PIC_TOTAL_EVENT];
+
+static unsigned char qnap_pic_inb(unsigned long addr)
+{
+	return readb((void __iomem *)addr);
+}
+
+static void qnap_pic_outb(unsigned long addr, unsigned char val)
+{
+	writeb(val, (void __iomem *)addr);
+}
+
+static irqreturn_t qnap_pic_isr(int irq, void *dev_id, struct pt_regs *regs)
+{
+	unsigned char int_id;
+	unsigned char data;
+	int	i, need_wakeup;
+
+	need_wakeup = 0;
+	for (i = 0 ; i < 10 ; i++) {
+		int_id = (qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_IIR) & 0x0f);
+		if (int_id & 0x01)
+			break;
+		int_id >>= 1;
+		switch (int_id) {
+		case 3:
+			qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_LSR);
+			break;
+
+		case 2:
+		case 6:
+			while (qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_LSR) & UART_LSR_DR) {
+				data = qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_RX);
+				if((data == 0x72) ||
+				   (data == QNAP_PIC_USB_COPY_BUTTON && !usb_button_enabled))
+					continue;	// Skip it
+
+				if (((rx_end + 1) % QUEUE_BUFSIZE) != rx_begin) {
+					rx_buf[rx_end] = data;
+					//printk("qnap pic: get byte 0x%x\n", rx_buf[rx_end]);
+					rx_end = ((rx_end + 1) % QUEUE_BUFSIZE);
+				}
+				need_wakeup = 1;
+			}
+			//need_wakeup = 1;
+			break;
+		}
+	}
+	if (i == 10)
+		printk("qnap pic: Reach maximal count of pending interrupts.\n");
+	if (need_wakeup)
+		wake_up_interruptible(&pic_wait);
+	return IRQ_HANDLED;
+}
+
+int send_message_to_app(unsigned char message)
+{
+	if (((rx_end + 1) % QUEUE_BUFSIZE) != rx_begin) {
+		rx_buf[rx_end] = message;
+		//printk("qnap pic: get byte 0x%x\n", rx_buf[rx_end]);
+		rx_end = ((rx_end + 1) % QUEUE_BUFSIZE);
+	}
+	
+	wake_up_interruptible(&pic_wait);
+
+	return 0;
+}
+
+void __init qnap_pic_init(void)
+{
+	unsigned char dcr;
+
+	/* enable UART2 */
+	dcr = qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_DCR);
+	dcr |= 0x01;
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_DCR, dcr);
+	/* Access baud rate */
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_LCR, UART_LCR_DLAB);
+	/* Input clock. */
+#ifdef CONFIG_QNAP_UART_CLK_100
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_DLL, (325/*(BASE_BAUD / SERIAL_BAUD)*/ & 0xFF));
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_DLM, (325/*(BASE_BAUD / SERIAL_BAUD)*/ >> 8));
+#else
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_DLL, (435/*(BASE_BAUD / SERIAL_BAUD)*/ & 0xFF));
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_DLM, (435/*(BASE_BAUD / SERIAL_BAUD)*/ >> 8));
+#endif
+	/* 8 data, 1 stop, no parity */
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_LCR, 0x03);
+}
+
+static int pic_open(struct inode *inode, struct file *fp)
+{
+	int result;
+
+	if (usage == 0) {
+		if ((result = request_irq(SANDPOINT_SERIAL_1_INT, qnap_pic_isr, SA_INTERRUPT, "qnap-pic", NULL)) < 0) {
+			printk("qnap pic - failed to attach interrupt\n");
+			return result;
+		}
+		tx_buf = (unsigned char*)kmalloc(QUEUE_BUFSIZE, GFP_KERNEL);
+		if (tx_buf == NULL) {
+			free_irq(SANDPOINT_SERIAL_1_INT, NULL);
+			return -ENOMEM;
+		}
+		use_int = 1;
+		qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_FCR, UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_XMIT | UART_FCR_CLEAR_RCVR | UART_FCR_TRIGGER_14);
+		qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_IER, UART_IER_RDI);
+	}
+	usage++;
+	return 0;
+}
+
+static int pic_release(struct inode *inode, struct file *fp)
+{
+	usage--;
+	if (usage == 0) {
+		while (tx_begin != tx_end)
+			interruptible_sleep_on(&queue_empty_wait);
+		qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_IER, 0);
+		qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_FCR, 0);
+		free_irq(SANDPOINT_SERIAL_1_INT, NULL);
+		use_int = 0;
+		if (tx_buf)
+			kfree(tx_buf);
+	}
+	return 0;
+}
+
+static int pic_ioctl(struct inode * inode, struct file *filp, u_int cmd, u_long arg)
+{
+	int i, bytes;
+	spinlock_t lock = SPIN_LOCK_UNLOCKED;
+	unsigned long flags;
+	struct qnap_pic_ioctl qpi;
+
+	switch (cmd) {
+	case IOCTL_MSG_GET_MESSAGE:
+		memset(&qpi, 0, sizeof(struct qnap_pic_ioctl));
+		while (rx_begin == rx_end) {
+			//printk("PIC without any event\n");
+			interruptible_sleep_on(&pic_wait);
+			if (signal_pending(current))
+			{
+				printk("pic_ioctl: signal_pending current failed\n");
+				return -ERESTARTSYS;
+			}
+		}
+		spin_lock_irqsave(lock, flags);
+		// calculate how many bytes available
+		bytes = ((rx_end + QUEUE_BUFSIZE) - rx_begin) % QUEUE_BUFSIZE;
+		// read data as many as possible
+		for (i = 0 ; i < bytes ; i++) {
+			qpi.pic_data[i] = rx_buf[rx_begin];
+			qpi.count++;
+			rx_begin = (rx_begin + 1) % QUEUE_BUFSIZE;
+		}
+		spin_unlock_irqrestore(lock, flags);
+		return copy_to_user((void *)arg, &qpi, sizeof(struct qnap_pic_ioctl));
+
+	case IOCTL_MSG_SEND_MESSAGE:
+		memset(&qpi, 0, sizeof(struct qnap_pic_ioctl));
+		if (copy_from_user(&qpi, (struct qnap_pic_ioctl *)arg, sizeof(struct qnap_pic_ioctl)))
+			break;
+		for (i = 0; i < QUEUE_BUFSIZE && i < qpi.count; i += 2) {
+			if (qpi.pic_data[i] >= QNAP_PIC_TOTAL_EVENT || qpi.pic_data[i + 1] >= PIC_EVENT_COMMAND_TYPE)
+				continue;
+			switch(qpi.pic_data[i]) {
+			case QNAP_PIC_USB_COPY:
+				if(qpi.pic_data[i+1] == QNAP_PIC_EVENT_ON)
+					usb_button_enabled = 0;
+				else
+					usb_button_enabled = 1;
+				break;
+			}
+			qnap_pic_send_command(pic_event[qpi.pic_data[i]].command[qpi.pic_data[i + 1]], pic_event[qpi.pic_data[i]].count[qpi.pic_data[i + 1]]);
+		}
+		return 0;
+	case IOCTL_MSG_SEND_RAW_COMMAND:
+		memset(&qpi, 0, sizeof(struct qnap_pic_ioctl));
+		if (copy_from_user(&qpi, (struct qnap_pic_ioctl *)arg, sizeof(struct qnap_pic_ioctl)))
+			break;
+		qnap_pic_send_command(qpi.pic_data, qpi.count);
+		return 0;
+	}
+	return -EINVAL;
+}
+
+static struct file_operations pic_fops = {
+	.owner = THIS_MODULE,
+	.ioctl = pic_ioctl,
+	.open = pic_open,
+	.release = pic_release,
+};
+
+static struct miscdevice pic_device = {
+	PIC_MINOR, "pic", &pic_fops
+};
+
+static void __init qnap_pic_event_init(void)
+{
+	memset(pic_event, 0, sizeof(pic_event));
+	pic_event[QNAP_PIC_BOOT_COMPLETE].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_STATUS_GREEN_ON;
+	pic_event[QNAP_PIC_BOOT_COMPLETE].command[QNAP_PIC_EVENT_ON][1] = QNAP_PIC_BUZZER_LONG;
+	pic_event[QNAP_PIC_BOOT_COMPLETE].count[QNAP_PIC_EVENT_ON] = 2;
+	pic_event[QNAP_PIC_WRONG_HD_FORMAT].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_STATUS_RED_ON;
+	pic_event[QNAP_PIC_WRONG_HD_FORMAT].count[QNAP_PIC_EVENT_ON] = 1;
+	pic_event[QNAP_PIC_WRONG_HD_FORMAT].command[QNAP_PIC_EVENT_OFF][0] = QNAP_PIC_STATUS_GREEN_ON;
+	pic_event[QNAP_PIC_WRONG_HD_FORMAT].count[QNAP_PIC_EVENT_OFF] = 1;
+	pic_event[QNAP_PIC_POWER_OFF].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_STATUS_OFF;
+	pic_event[QNAP_PIC_POWER_OFF].command[QNAP_PIC_EVENT_ON][1] = QNAP_PIC_POWER_LED_BLINK;
+	pic_event[QNAP_PIC_POWER_OFF].count[QNAP_PIC_EVENT_ON] = 2;
+	pic_event[QNAP_PIC_HD_STANDBY].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_STATUS_GREEN_ON;
+	pic_event[QNAP_PIC_HD_STANDBY].command[QNAP_PIC_EVENT_ON][1] = QNAP_PIC_POWER_LED_BLINK;
+	pic_event[QNAP_PIC_HD_STANDBY].count[QNAP_PIC_EVENT_ON] = 2;
+	pic_event[QNAP_PIC_HD_STANDBY].command[QNAP_PIC_EVENT_OFF][0] = QNAP_PIC_STATUS_GREEN_ON;
+	pic_event[QNAP_PIC_HD_STANDBY].command[QNAP_PIC_EVENT_OFF][1] = QNAP_PIC_POWER_LED_ON;
+	pic_event[QNAP_PIC_HD_STANDBY].count[QNAP_PIC_EVENT_OFF] = 2;
+	pic_event[QNAP_PIC_USB_COPY].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_USB_LED_BLINK;
+	pic_event[QNAP_PIC_USB_COPY].count[QNAP_PIC_EVENT_ON] = 1;
+	pic_event[QNAP_PIC_USB_COPY].command[QNAP_PIC_EVENT_OFF][0] = QNAP_PIC_USB_LED_OFF;
+	pic_event[QNAP_PIC_USB_COPY].count[QNAP_PIC_EVENT_OFF] = 1;
+	pic_event[QNAP_PIC_SET_DEFAULT].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_BUZZER_SHORT;
+	pic_event[QNAP_PIC_SET_DEFAULT].count[QNAP_PIC_EVENT_ON] = 1;
+	pic_event[QNAP_PIC_POWER_RECOVERY].command[QNAP_PIC_EVENT_ON][0] = QNAP_PIC_ENABLE_POWER_RECOVERY;
+	pic_event[QNAP_PIC_POWER_RECOVERY].count[QNAP_PIC_EVENT_ON] = 1;
+	pic_event[QNAP_PIC_POWER_RECOVERY].command[QNAP_PIC_EVENT_OFF][0] = QNAP_PIC_DISABLE_POWER_RECOVERY;
+	pic_event[QNAP_PIC_POWER_RECOVERY].count[QNAP_PIC_EVENT_OFF] = 1;
+}
+
+static __init int qnap_pic_init2(void)
+{
+	int result;
+
+	result = misc_register(&pic_device);
+	if (result < 0)
+		printk("qnap pic: fail to register misc device\n");
+	else {
+		//printk("qnap pic: suceed to register misc device\n");
+		init_waitqueue_head(&pic_wait);
+		init_waitqueue_head(&queue_empty_wait);
+		qnap_pic_event_init();
+	}
+	return result;
+}
+
+__initcall(qnap_pic_init2);
+
+static void qnap_pic_putc(unsigned char c)
+{
+	while ((qnap_pic_inb(SANDPOINT_SERIAL_1 + UART_LSR) & UART_LSR_THRE) == 0)
+		;
+	qnap_pic_outb(SANDPOINT_SERIAL_1 + UART_TX, c);
+}
+
+int qnap_pic_send_command(char *data, int count)
+{
+	int i;
+
+	if (data == NULL || count <= 0)
+		return -EINVAL;
+	for (i = 0; i < count; i++)
+		qnap_pic_putc(data[i]);
+	return count;
+}
+EXPORT_SYMBOL(qnap_pic_send_command);
+EXPORT_SYMBOL(send_message_to_app);
