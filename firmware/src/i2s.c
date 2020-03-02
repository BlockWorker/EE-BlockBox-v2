/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "i2s.h"
#include "app.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

#define I2S_RECEIVE_BUFFER_LEN 2048
#define I2S_RECEIVE_BUFFER_PHYS_LOC 0x2000
#define I2S_RECEIVE_BUFFER_VIRT_LOC I2S_RECEIVE_BUFFER_PHYS_LOC | 0x80000000

volatile int32_t receiveBuffer[I2S_RECEIVE_BUFFER_LEN] __attribute__((address(I2S_RECEIVE_BUFFER_VIRT_LOC), keep));


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */





/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void I2S_Init() {
    IEC3bits.SPI1EIE = 0;
    IEC3bits.SPI1RXIE = 0;
    IEC3bits.SPI1TXIE = 0; //disable SPI interrupts
    IEC4bits.DMA4IE = 0; //disable DMA interrupt
    
    IFS3bits.SPI1RXIF = 0; //reset SPI receive interrupt flag
    
    SPI1CON = 0; //reset SPI config and disable SPI module
    SPI1CON2 = 0; //reset SPI config 2
    SPI1BUF = 0; //clear SPI RX buffer
    SPI1CONbits.DISSDO = 1; //disable SDO
    SPI1CONbits.ENHBUF = 1; //enable enhanced SPI buffer
    SPI1STATbits.SPIROV = 0; //clear SPI RX overflow flag
    SPI1CON2bits.AUDMOD = 0b00; //SPI I2S mode
    SPI1CON2bits.IGNROV = 1; //ignore receive overflow (non-critical application)
    SPI1CON2bits.AUDEN = 1; //enable SPI audio mode
    SPI1CONbits.MSTEN = 0; //SPI slave mode
    SPI1CONbits.CKP = 1; //inverted SPI clock polarity (required for I2S)
    SPI1CONbits.MODE32 = 1;
    SPI1CONbits.MODE16 = 1; //I2S 24-bit data, 64-bit frame mode
    
    DCH4CON = 0; //reset DMA config and disable channel
    DCH4ECON = 0; //reset DMA event config
    DCH4INT = 0; //disable and clear all DMA channel interrupts
    DCH4ECONbits.CHAIRQ = 0xFF; //no IRQ for DMA abort
    DCH4ECONbits.CHSIRQ = _SPI1_RX_VECTOR; //DMA start on SPI RX interrupt
    DCH4ECONbits.SIRQEN = 1; //enable DMA start IRQ
    DCH4SSA = 0x1F821020; //DMA source: SPI1BUF (physical address)
    DCH4DSA = I2S_RECEIVE_BUFFER_PHYS_LOC; //DMA dest: receive buffer (physical address)
    DCH4SSIZ = 4; //DMA source size: 4 bytes
    DCH4DSIZ = 4 * I2S_RECEIVE_BUFFER_LEN; //DMA dest size: (4 * buffer length) bytes
    DCH4CSIZ = 8; //DMA cell size: 8 bytes (stereo sample)
    DCH4CONbits.CHAEN = 1; //DMA auto-enable
    DCH4CONbits.CHEN = 1; //enable DMA channel
    
    SPI1CONbits.ON = 1; //enable SPI/I2S
}



/* *****************************************************************************
 End of File
 */
