/* ************************************************************************** */


#ifndef _BM83_H    /* Guard against multiple inclusion */
#define _BM83_H

#include "app.h"

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
        BM83_PLAYING,
        BM83_BUSY
    } BM83_STATE;
    
    typedef enum {
        BM83_CMD_MMI_Action = 0x02,
        BM83_CMD_Read_BTM_Version = 0x08,
        BM83_CMD_EventAck = 0x14,
        BM83_CMD_Rx_Buffer_Size = 0x1f,
        BM83_CMD_NONE = 0xff
    } BM83_COMMAND;
    
    typedef enum {
        BM83_EVENT_CommandAck = 0x00,
        BM83_EVENT_BTM_Status = 0x01,
        BM83_EVENT_Read_BTM_Version_Reply = 0x18,
        BM83_EVENT_Ringtone_Status_Indication = 0x24,
        BM83_EVENT_Report_Type_Codec = 0x2D,
        BM83_EVENT_Report_BTM_Initial_Status = 0x30,
        BM83_EVENT_NONE = 0xff
    } BM83_EVENT;
    
    typedef enum {
        BM83_RESULT_SUCCESS = 0,
        BM83_RESULT_TIMEOUT,
        BM83_RESULT_CHECKSUM_MISMATCH,
        BM83_RESULT_NACK,
        BM83_RESULT_OTHER_ERROR
    } BM83_COMMAND_RESULT;
    
    typedef enum {
        BM83_SR_8K = 0x00,
        BM83_SR_16K = 0x02,
        BM83_SR_32K = 0x04,
        BM83_SR_48K = 0x05,
        BM83_SR_44_1K = 0x06,
        BM83_SR_88K = 0x07,
        BM83_SR_96K = 0x08,
        BM83_SR_UNKNOWN = 0xff
    } BM83_SAMPLERATE;    
    
    typedef enum {
        BM83_CODEC_IDLE = 0x00,
        BM83_CODEC_PREPARE = 0x01,
        BM83_CODEC_AUX = 0x02,
        BM83_CODEC_PCM = 0x03,
        BM83_CODEC_A2DP = 0x04,
        BM83_CODEC_HF = 0x05,
        BM83_CODEC_TONE = 0x06,
        BM83_CODEC_VP = 0x07
    } BM83_CODEC_STATUS;
    
    typedef enum {
        BM83_CHANGE_STATE = 0,
        BM83_CHANGE_PLAYBACK,
        BM83_CHANGE_CODEC
    } BM83_STATE_CHANGE_TYPE;
    
    typedef void (*BM83_COMMAND_CALLBACK)(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context);
    
    typedef void (*BM83_STATE_CHANGE_CALLBACK)(BM83_STATE_CHANGE_TYPE changeType);

    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    BM83_STATE bm83_state;
    BM83_SAMPLERATE bm83_samplerate;
    uint32_t bm83_samplerate_number;
    BM83_CODEC_STATUS bm83_codec_status;
    bool bm83_pairing;
    
    void BM83_PowerOn(SUCCESS_CALLBACK callback);
    void BM83_PowerOff(SUCCESS_CALLBACK callback);
    void BM83_EnterPairing(SUCCESS_CALLBACK callback);
    void BM83_ExitPairing(SUCCESS_CALLBACK callback);
    
    void BM83_IO_Init();
    void BM83_Module_Init(SUCCESS_CALLBACK callback);
    bool BM83_Queue_Command(BM83_COMMAND command, uint8_t* params, uint16_t paramLength);
    bool BM83_Queue_Command_Callback(BM83_COMMAND command, uint8_t* params, uint16_t paramLength, BM83_COMMAND_CALLBACK callback, uintptr_t context);
    
    void BM83_SetStateChangeCallback(BM83_STATE_CHANGE_CALLBACK callback);
    
    void BM83_Tasks();
    

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
