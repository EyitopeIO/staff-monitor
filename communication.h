/*
 * communication.h
 *
 *  Created on: 7 Nov 2018
 *      Author: Eyitope
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#define UART0_MODEM_DATA_SIZE   50
#define UART1_BLE_DATA_SIZE   50

void config_IO(void);

void config_USCI_A0_modem(void);
void USCI_A0_ISR_modem(void);

void USCI_A1_ISR_ble(void);
void config_USCI_A1_ble(void);

void timer0_A3_setup(void);

void config_CLOCK(void);
void send_STRING(unsigned char *data, unsigned char dir);
void send_BYTE(unsigned char data, unsigned char dir);



#endif /* COMMUNICATION_H_ */

