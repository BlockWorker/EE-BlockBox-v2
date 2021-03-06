/* ************************************************************************** */
#include "ui.h"

#ifndef UI_TOUCH

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */





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

void UI_IO_Init() {
    SPI2CON = 0;
    SPI2CONbits.DISSDI = 1;
    SPI2CONbits.DISSDO = 1; //disable SPI module and pins
    SYS_INT_SourceDisable(INT_SOURCE_EXTERNAL_4); //disable ext interrupt
    
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 0; //unlock PPS
    RPC2R = 0; //disable SS2 output
    RPG8R = 0; //disable SDO2 output
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 1; //lock PPS
    
    CNPDAbits.CNPDA5 = 0; //disable A5 pulldown
    TRISAbits.TRISA5 = 0; //make A5 an output
    CNPUCbits.CNPUC4 = 0; //disable C4 pullup
    TRISCCLR = 0b11110; //make C1-4 outputs
    TRISECLR = 0b11100000; //make E5-7 outputs
    TRISGCLR = 0b111000000; //make G6-8 outputs
    LATGbits.LATG8 = 1; //set G8 high (LCD enable)
}

void UI_Main_Init(SUCCESS_CALLBACK cb) {
    
}

void UI_Tasks() {
    
}

void UI_InterruptHandler() {
    
}


#endif
/* *****************************************************************************
 End of File
 */
