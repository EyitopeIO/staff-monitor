/*
 *communication.c
 *
 *  Created on: 7 Nov 2018
 *      Author: Eyitope Adelowo
 */



#include <communication.h>
#include <driverlib.h>
#include <string.h>
#include <common.h>

const unsigned char *AT_OUT = "AT\r";
const unsigned char *OK_IN = "OK";
const unsigned char *MODEM_TEST_STRING_OUT = "START\r";
const unsigned char *MODEM_ECHO_OUT = "ATE0\r";
const unsigned char *MODEM_NETWORK_IN = "+CREG: 0,1";
const unsigned char *MODEM_NETWORK_OUT = "AT+CREG?\r";
const unsigned char *MODEM_STARTUP_RESP_IN = "MODEM:STARTUP";
const unsigned char *PBREADY_IN = "+PBREADY";
const unsigned char *MODEM_GSM_MODE_OUT = "AT+CSCS=\"GSM\"\r";
const unsigned char *MODEM_TEXT_MODE_OUT = "AT+CMGF=1\r";
const unsigned char *MODEM_PHONE_NUMBER_SELECT = "AT+CMGS=";
const unsigned char *BLE_COMMAND_MODE_START_OUT = "$$$";
const unsigned char *BLE_COMMAND_MODE_STOP_OUT = "---\r";
const unsigned char *BLE_START_SCAN_OUT = "F\r";
const unsigned char *BLE_STOP_SCAN_OUT = "X\r";
const unsigned char *BLE_COMMAND_READY_IN = "CMD>";

unsigned char modem_case = 1;
unsigned char modem_data_complete = 0;
unsigned char ble_case = 1;
unsigned char UART0_modem_data[UART1_MODEM_DATA_SIZE];              //Modem
unsigned char UART1_blE_data[UART2_BLE_DATA_SIZE];          //Bluetooth
unsigned char index_UART0_modem = 0;
unsigned char index_UART1_ble = 0;


void config_CLOCK(void)
{
    /*
     * DCO is digitally controlled oscillator.
     * Family user guide has a table showing what value to put in the register.
     * Then driverlib defined those somewhere. Check cs.h & cs.c for more ideas
     */
    CS_setDCOFreq(CS_DCORSEL_1,CS_DCOFSEL_4);       //Set DCO for 16MHz
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    flash();
}

void config_IO(void)
{

    // Configure UART_A0 operation,  P2.0 & P2.1
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,       //TXD
                                               GPIO_PIN0,
                                               GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,       //RXD
                                               GPIO_PIN1,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    //Configure UART_A1 operation P3.5 & P3.4
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3,       //TXD
                                               GPIO_PIN4,
                                               GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3,       //RXD
                                               GPIO_PIN5,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,       //RXD
                                                   GPIO_PIN0,
                                                   GPIO_PRIMARY_MODULE_FUNCTION);

    //Configure debug LEDs
     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);      //left
     GPIO_setAsOutputPin(GPIO_PORT_P9, GPIO_PIN0 + GPIO_PIN7);      //right

     // Disable the GPIO power-on default high-impedance mode to activate
     // previously configured port settings
     PM5CTL0 &= ~LOCKLPM5;
     flash();
}



void config_USCI_A0_modem(void)
{
    // Configure USCI_A0 for UART mode at 9600
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_ACLK;
    param.clockPrescalar = 3;           //Go to page 566 of family user guide for table
    param.secondModReg = 0x92;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    while(!EUSCI_A_UART_init(EUSCI_A0_BASE, &param))
    EUSCI_A_UART_enable(EUSCI_A0_BASE);
    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
          EUSCI_A_UART_RECEIVE_INTERRUPT);
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
          EUSCI_A_UART_RECEIVE_INTERRUPT);
    flash();
}

void config_USCI_A1_ble(void)
{
    // Configure USCI_A1 for UART mode at 9600
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_ACLK;
    param.clockPrescalar = 3;
    param.secondModReg = 0x92;          //Go to page 566 of family user guide for table
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    while(!EUSCI_A_UART_init(EUSCI_A1_BASE, &param))
    EUSCI_A_UART_enable(EUSCI_A1_BASE);
    EUSCI_A_UART_clearInterrupt(EUSCI_A1_BASE,
          EUSCI_A_UART_RECEIVE_INTERRUPT);
    EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
          EUSCI_A_UART_RECEIVE_INTERRUPT);
    flash();
}

//UART_A0 ISR for modem
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR_modem(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  unsigned char rcvd = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;

    case USCI_UART_UCRXIFG:
      while(!(UCA0IFG&UCTXIFG));

      //Your code here
      UART0_modem_data[index_UART1_modem++] = rcvd;

      __no_operation();
      break;

    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}

//UART_A1 ISR for modem
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR_modem(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  unsigned char rcvd = EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
  switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;

    case USCI_UART_UCRXIFG:
      while(!(UCA1IFG&UCTXIFG));

      //Your code here
      UART1_ble_data[index_UART1_modem++] = rcvd;
      __no_operation();

      break;

    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}

void send_STRING(unsigned char *data, unsigned char dir)
{
    unsigned char len = length(data);
    while(len--)
    {
        send_BYTE(*data, dir);
        data++;
    }
}

void send_BYTE(unsigned char data, unsigned char dir)
{
    switch(dir)
    {
    case 'm':
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE,data);
        break;
    case 'b':
        EUSCI_A_UART_transmitData(EUSCI_A1_BASE, data);
        break;
    }
}

void timer0_A3_setup(void)
{
    Timer_A_clearTimerInterrupt(TIMER_A0_BASE);

    Timer_A_initContinuousModeParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_ACLK; //Sourced from 32.768kHz clock
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    param.timerClear = TIMER_A_DO_CLEAR;
    param.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A0_BASE, &param);
    Timer_A_startCounter(TIMER_A0_BASE,
        TIMER_A_CONTINUOUS_MODE
        );
}
