/* ************************************************************************** */


#ifndef _DAP_H    /* Guard against multiple inclusion */
#define _DAP_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"


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

    typedef void (*DAP_COMMAND_CALLBACK)(bool success, uint8_t* buffer, uint16_t bufferLength, uintptr_t context);

    
    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    bool dap_shutDown;
    uint16_t dap_volume;
    bool dap_muted;
    
    bool DAP_WriteBufferCallback(uint8_t subaddress, uint8_t* buffer, uint16_t length, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs);
    bool DAP_WriteBuffer(uint8_t subaddress, uint8_t* buffer, uint16_t length);
    bool DAP_WriteWordCallback(uint8_t subaddress, uint32_t value, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs);
    bool DAP_WriteWord(uint8_t subaddress, uint32_t value);
    bool DAP_WriteByteCallback(uint8_t subaddress, uint8_t value, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs);
    bool DAP_WriteByte(uint8_t subaddress, uint8_t value);
    bool DAP_Read(uint8_t subaddress, uint16_t length, DAP_COMMAND_CALLBACK callback, uintptr_t context);
    
    bool DAP_SetVolume(uint16_t volume, SUCCESS_CALLBACK callback);
    bool DAP_ShutDown(SUCCESS_CALLBACK callback);
    bool DAP_StartUp(SUCCESS_CALLBACK callback);
    bool DAP_Mute(SUCCESS_CALLBACK callback);
    bool DAP_Unmute(SUCCESS_CALLBACK callback);
    
    void DAP_Tasks();
    void DAP_IO_Init();
    void DAP_Chip_Init(SUCCESS_CALLBACK callback);


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
