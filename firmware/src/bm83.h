/* ************************************************************************** */


#ifndef _BM83_H    /* Guard against multiple inclusion */
#define _BM83_H

#include "config/default/driver/usart/drv_usart.h"



/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */



/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */





    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************


    typedef enum {
        BM83_NOT_INITIALIZED = 0,
        BM83_OFF,
        BM83_IDLE,
        BM83_CONNECTED,
        BM83_PLAYING
    } BM83_STATE;
    
    typedef enum {
        BM83_CMD_MMI_Action = 0x02,
        BM83_CMD_EventAck = 0x14,
        BM83_CMD_Rx_Buffer_Size = 0x1f,
        BM83_CMD_NONE = 0xff
    } BM83_COMMAND;
    
    typedef enum {
        BM83_EVENT_CommandAck = 0x00,
        BM83_EVENT_NONE = 0xff
    } BM83_EVENT;
    
    typedef enum {
        BM83_RESULT_SUCCESS = 0,
        BM83_RESULT_TIMEOUT,
        BM83_RESULT_CHECKSUM_MISMATCH,
        BM83_RESULT_NACK,
        BM83_RESULT_OTHER_ERROR
    } BM83_COMMAND_RESULT;
    
    typedef void (*BM83_COMMAND_CALLBACK)(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context);


    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    
    BM83_STATE bm83_state;
    
    void BM83_IO_Init();
    void BM83_Module_Init();
    bool BM83_Queue_Command(BM83_COMMAND command, uint8_t* params, uint16_t paramLength);
    bool BM83_Queue_Command_Callback(BM83_COMMAND command, uint8_t* params, uint16_t paramLength, BM83_COMMAND_CALLBACK callback, uintptr_t context);
    
    void BM83_Tasks();
    

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
