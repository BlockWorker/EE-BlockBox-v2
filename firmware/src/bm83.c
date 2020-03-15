/* ************************************************************************** */

#include "bm83.h"
#include "app.h"

/*****************************/
/* Definitions and variables */
/*****************************/

#define BM83_RECEIVE_BUFFER_LEN 0x2000
#define BM83_RECEIVE_BUFFER_PHYS_LOC 0x4000
#define BM83_RECEIVE_BUFFER_VIRT_LOC BM83_RECEIVE_BUFFER_PHYS_LOC | 0x80000000

typedef struct {
    BM83_COMMAND command;
    uint8_t* commandData;
    uint16_t commandDataLength;
    BM83_EVENT expectedResponseEvent;
    uint8_t* responseData;
    uint16_t responseDataLength;
    uint32_t ackTimeoutTick;
    bool previouslyFailed;
    DRV_USART_BUFFER_HANDLE sendHandle;
    BM83_COMMAND_CALLBACK callback;
    uintptr_t callbackContext;
} BM83_COMMAND_TASK;

typedef struct bm_queueitem {
    BM83_COMMAND_TASK* task;
    struct bm_queueitem* next;
} BM83_QUEUE_ITEM;

typedef struct {
    BM83_QUEUE_ITEM* head;
    BM83_QUEUE_ITEM* tail;
    uint16_t length;
} BM83_COMMAND_QUEUE;

BM83_STATE bm83_state = BM83_NOT_INITIALIZED;

DRV_HANDLE drv;
volatile uint8_t uart_receiveBuffer[BM83_RECEIVE_BUFFER_LEN] __attribute__((address(BM83_RECEIVE_BUFFER_VIRT_LOC), keep));

BM83_COMMAND_QUEUE unsentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE sentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE ackQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE responseQueue = { NULL, NULL, 0 };

uint32_t ackTimeoutTicks;

/******************/
/* List functions */
/******************/

void bm83_addToQueue(BM83_COMMAND_QUEUE* queue, BM83_QUEUE_ITEM* item) {
    item->next = NULL;
    if (queue->length == 0) queue->head = item;
    else queue->tail->next = item;
    queue->tail = item;
    queue->length++;
}

BM83_QUEUE_ITEM* bm83_takeFromQueue(BM83_COMMAND_QUEUE* queue) {
    if (queue->length == 0) return NULL;
    BM83_QUEUE_ITEM* item = queue->head;
    queue->head = item->next;
    if (--queue->length == 0) queue->tail = NULL;
    item->next = NULL;
    return item;
}

void bm83_clearQueue(BM83_COMMAND_QUEUE* queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
}

/********************/
/* Helper functions */
/********************/

BM83_EVENT bm83_getResponseEvent(BM83_COMMAND command) {
    switch (command) {
        
        default: return BM83_EVENT_NONE;
    }
}

/******************************/
/* Command handling functions */
/******************************/

void bm83_cmd_time_callback(uintptr_t context) {
    BM_MFB_Clear();
    
    if (unsentQueue.length == 0) return;
    uint16_t itemsLeft;
    
    for (itemsLeft = unsentQueue.length; itemsLeft > 0; itemsLeft--) {
        BM83_QUEUE_ITEM* item = bm83_takeFromQueue(&unsentQueue);
        if (item == NULL) break;
        BM83_COMMAND_TASK* task = item->task;
        DRV_USART_WriteBufferAdd(drv, task->commandData, task->commandDataLength, &task->sendHandle);
        if (task->sendHandle == DRV_USART_BUFFER_HANDLE_INVALID) {
            if (task->previouslyFailed) {
                if (task->callback != NULL) task->callback(BM83_RESULT_OTHER_ERROR, NULL, 0, task->callbackContext);
            } else {
                task->previouslyFailed = true;
                bm83_addToQueue(&unsentQueue, item);
            }
            return;
        }
        bm83_addToQueue(&sentQueue, item);
    }
}

void bm83_cmd_send_callback(DRV_USART_BUFFER_EVENT event, DRV_USART_BUFFER_HANDLE bufferHandle, uintptr_t context) {
    
}

bool BM83_Queue_Command_Callback(BM83_COMMAND command, uint8_t* params, uint16_t paramLength, BM83_COMMAND_CALLBACK callback, uintptr_t context) {
    BM83_COMMAND_TASK* task = malloc(sizeof(BM83_COMMAND_TASK));
    if (task == NULL) return false;
    
    task->command = command;
    task->commandDataLength = paramLength + 5;
    task->commandData = malloc(task->commandDataLength);
    if (task->commandData == NULL) return false;
    task->expectedResponseEvent = bm83_getResponseEvent(command);
    task->callback = callback;
    task->callbackContext = context;
    task->previouslyFailed = false;
    
    task->commandData[0] = 0xAA;
    uint16_t sentLength = paramLength + 1;
    task->commandData[1] = sentLength >> 8;
    task->commandData[2] = sentLength & 0xff;
    task->commandData[3] = command;
    memcpy(task->commandData + 4, params, paramLength);
    
    uint8_t sum = 0;
    uint16_t i;
    for (i = 1; i < task->commandDataLength - 1; i++) sum += task->commandData[i];
    task->commandData[task->commandDataLength - 1] = -sum;
    
    BM83_QUEUE_ITEM* item = malloc(sizeof(BM83_QUEUE_ITEM));
    if (item == NULL) return false;
    item->task = task;
    bm83_addToQueue(&unsentQueue, item);
    
    return true;
}

bool BM83_Queue_Command(BM83_COMMAND command, uint8_t* params, uint16_t paramLength) {
    return BM83_Queue_Command_Callback(command, params, paramLength, NULL, 0);
}

void BM83_Tasks() {
    if (unsentQueue.length > 0) {
        BM_MFB_Set();
        SYS_TIME_CallbackRegisterMS(bm83_cmd_time_callback, 0, 3, SYS_TIME_SINGLE);
    }
}

/****************************/
/* Initialization functions */
/****************************/

void bm83_init_time_callback(uintptr_t context) {
    switch (context) {
        case 0:
        {
            BM_MFB_Set();
            SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 1, 20, SYS_TIME_SINGLE);
            break;
        }
        case 1:
        {
            BM_RST_N_Set();
            SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 2, 500, SYS_TIME_SINGLE);
            break;
        }
        case 2:
        {
            BM_MFB_Clear();
            
        }
    }
}

void BM83_IO_Init() {
    IEC4bits.DMA3IE = 0; //disable DMA interrupt
    
    DCH3CON = 0; //reset DMA config and disable channel
    DCH3ECON = 0; //reset DMA event config
    DCH3INT = 0; //disable and clear all DMA channel interrupts
    DCH3ECONbits.CHAIRQ = 0xFF; //no IRQ for DMA abort
    DCH3ECONbits.CHSIRQ = _UART2_RX_VECTOR; //DMA start on UART RX interrupt
    DCH3ECONbits.SIRQEN = 1; //enable DMA start IRQ
    DCH3SSA = 0x1F822230; //DMA source: U2RXREG (physical address)
    DCH3DSA = BM83_RECEIVE_BUFFER_PHYS_LOC; //DMA dest: receive buffer (physical address)
    DCH3SSIZ = 1; //DMA source size: 1 byte
    DCH3DSIZ = BM83_RECEIVE_BUFFER_LEN; //DMA dest size: (buffer length) bytes
    DCH3CSIZ = 1; //DMA cell size: 1 byte
    DCH3CONbits.CHAEN = 1; //DMA auto-enable
    DCH3CONbits.CHEN = 1; //enable DMA channel
    
    drv = DRV_USART_Open(DRV_USART_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    DRV_USART_BufferEventHandlerSet(drv, bm83_cmd_send_callback, 0);
    
    ackTimeoutTicks = SYS_TIME_MSToCount(200);
}

void BM83_Module_Init() {
    bm83_state = BM83_NOT_INITIALIZED;
    BM_MFB_Clear();
    BM_RST_N_Clear();
    SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 0, 200, SYS_TIME_SINGLE);
}


/* *****************************************************************************
 End of File
 */
