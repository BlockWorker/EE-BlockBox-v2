/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "i2s.h"
#include "bm83.h"
#include "dap.h"
#include "ui.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void sendStuff(uintptr_t context) {
    //uint8_t type = 0x00;
    //BM83_Queue_Command(BM83_CMD_Read_BTM_Version, &type, 1);
    SYS_DEBUG_Message("test");
}

void init_bm83_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context) {
    appData.state = APP_STATE_SERVICE_TASKS;
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize() {
    UI_IO_Init();
    I2S_Init();
    DAP_IO_Init();
    BM83_IO_Init();
    
    appData.state = APP_STATE_INIT_BM83;
    BM83_Module_Init(init_bm83_callback);
    
    //SYS_TIME_CallbackRegisterMS(sendStuff, 0, 500, SYS_TIME_PERIODIC);
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks() {

    /* Check the application's current state. */
    switch(appData.state) {
        
        case APP_STATE_INIT_UI:
        {
            
            break;
        }
        
        case APP_STATE_INIT_DAP:
        {
            
            break;
        }
        
        case APP_STATE_INIT_BM83:
        {
            BM83_Tasks();
            break;
        }        

        case APP_STATE_SERVICE_TASKS:
        {
            BM83_Tasks();
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
