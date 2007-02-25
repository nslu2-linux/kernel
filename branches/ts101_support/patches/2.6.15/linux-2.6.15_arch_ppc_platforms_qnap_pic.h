--- linux-2.6.15.vanilla/arch/ppc/platforms/qnap_pic.h	1970-01-01 01:00:00.000000000 +0100
+++ linux-2.6.15/arch/ppc/platforms/qnap_pic.h	2007-02-25 02:47:05.000000000 +0100
@@ -0,0 +1,19 @@
+#ifndef __PPC_PLATFORMS_QNAP_PIC_H
+#define __PPC_PLATFORMS_QNAP_PIC_H
+
+extern void qnap_pic_init(void);
+extern int qnap_pic_send_command(char *data, int count);
+
+#define UART_DSR				0x10
+#define UART_DCR				0x11
+#define SERIAL_BAUD				19200
+
+#define PIC_EVENT_COMMAND_SIZE	8
+#define PIC_EVENT_COMMAND_TYPE	2
+
+struct qnap_pic_event {
+	unsigned char command[PIC_EVENT_COMMAND_TYPE][PIC_EVENT_COMMAND_SIZE];
+	int count[PIC_EVENT_COMMAND_TYPE];
+};
+
+#endif /* __PPC_PLATFORMS_QNAP_PIC_H */
