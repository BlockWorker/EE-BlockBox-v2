/* ************************************************************************** */

#include "bm83.h"
#include "app.h"

/*****************************/
/* Definitions and variables */
/*****************************/

#define UART_RECEIVE_BUFFER_LEN 0x2000
#define UART_RECEIVE_BUFFER_PHYS_LOC 0x4000
#define UART_RECEIVE_BUFFER_VIRT_LOC UART_RECEIVE_BUFFER_PHYS_LOC | 0x80000000
#define UART_TIMEOUT_MAX_DIFF 100000000 //difference (tick - timeoutTick) is compared to this, greater than this means not timed out (very high value means negative difference)

typedef struct {
    BM83_COMMAND command;
    uint8_t* data;
    uint16_t dataLength;
    BM83_EVENT expectedResponseEvent;
    uint32_t receiveTimeoutTick; //timeout tick for ACK or reponse
    uint8_t previouslyFailed; //1 = send request failed, 2 = send error, 4 = ack timeout, 8 = response timeout
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
volatile uint8_t uart_receiveBuffer[UART_RECEIVE_BUFFER_LEN] __attribute__((address(UART_RECEIVE_BUFFER_VIRT_LOC), keep));
uint16_t uart_bufferReadPointer = 0;

//command queues
BM83_COMMAND_QUEUE unsentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE sentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE ackQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE responseQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE unhandledEventQueue = { NULL, NULL, 0 }; //events received that weren't expected by any command

uint32_t ackTimeoutTicks; //number of ticks equivalent to 200ms, calculated on init
uint32_t responseTimeoutTicks; //number of ticks equivalent to 1000ms, calculated on init

/********************/
/* Helper functions */
/********************/

//maps commands to their expected response events
BM83_EVENT bm83_getResponseEvent(BM83_COMMAND command) {
    switch (command) {
        
        default: return BM83_EVENT_NONE;
    }
}

void bm83_freeTask(BM83_COMMAND_TASK* task) {
    if (task == NULL) return;
    free(task->data);
    free(task);
}

void bm83_freeItem(BM83_QUEUE_ITEM* item) {
    if (item == NULL) return;
    bm83_freeTask(item->task);
    free(item);
}

/******************/
/* List functions */
/******************/

//append item at end of queue
void bm83_addToQueue(BM83_COMMAND_QUEUE* queue, BM83_QUEUE_ITEM* item) {
    item->next = NULL; //remove potential old list association
    if (queue->length == 0) queue->head = item; //if first item: set head
    else queue->tail->next = item; //otherwise: put behind current tail
    queue->tail = item;
    queue->length++;
}

//remove and return head of queue
BM83_QUEUE_ITEM* bm83_takeFromQueue(BM83_COMMAND_QUEUE* queue) {
    if (queue->length == 0) return NULL; //empty list catck
    BM83_QUEUE_ITEM* item = queue->head; //get result (current head)
    queue->head = item->next; //update head
    if (--queue->length == 0) queue->tail = NULL; //remove tail pointer if last item removed
    item->next = NULL; //unlink result from list
    return item;
}

void bm83_clearQueue(BM83_COMMAND_QUEUE* queue) {
    BM83_QUEUE_ITEM* item = queue->head;
    while (item != NULL) {
        bm83_freeItem(item);
        item = item->next;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
}

//remove arbitrary item from queue, also requires pointer to the predecessor
void bm83_removeFromQueue(BM83_COMMAND_QUEUE* queue, BM83_QUEUE_ITEM* item, BM83_QUEUE_ITEM* previous) {
    if (previous == NULL) queue->head = item->next; //no previous: removing head, update head pointer
    else previous->next = item->next; //previous present: update to skip removed item
    if (item->next == NULL) queue->tail = previous; //if item is tail: update tail pointer
    item->next = NULL; //unlink item from list
    queue->length--;
}

//find given event (or command expecting the given event) in the given queue and remove it
BM83_QUEUE_ITEM* bm83_findAndRemoveEvent(BM83_COMMAND_QUEUE* queue, BM83_EVENT event) {
    if (queue->length == 0) return NULL;
    
    BM83_QUEUE_ITEM* prev = NULL;
    BM83_QUEUE_ITEM* item = queue->head; //start at head
    while (item->task->expectedResponseEvent != event) { //iterate and look for correct item
        prev = item; //remember previous item
        item = item->next;
        if (item == NULL) return NULL; //this means the item wasn't found: return
    }
    
    bm83_removeFromQueue(queue, item, prev); //remove item from sent queue
    return item;
}

/******************************/
/* Command handling functions */
/******************************/

//callback for MFB pulse before command transmission
void bm83_cmd_time_callback(uintptr_t context) {
    if (unsentQueue.length == 0) return;
    
    BM_MFB_Clear();
    uint16_t itemsLeft;
    
    //iterate through all items in unsent queue *currently* (not items added back in the loop)
    for (itemsLeft = unsentQueue.length; itemsLeft > 0; itemsLeft--) {
        BM83_QUEUE_ITEM* item = bm83_takeFromQueue(&unsentQueue); //get item
        if (item == NULL) break;
        BM83_COMMAND_TASK* task = item->task;
        DRV_USART_WriteBufferAdd(drv, task->data, task->dataLength, &task->sendHandle); //try sending
        if (task->sendHandle == DRV_USART_BUFFER_HANDLE_INVALID) { //if send request failed:
            if (task->previouslyFailed & 1) { //already failed once: callback with error
                if (task->callback != NULL) task->callback(BM83_RESULT_OTHER_ERROR, NULL, 0, task->callbackContext);
                bm83_freeItem(item);
            } else { //not failed before: set failed flag, add back to queue
                task->previouslyFailed |= 1;
                bm83_addToQueue(&unsentQueue, item);
            }
            return;
        }
        task->previouslyFailed &= 0xfe; //after success: reset send request fail flag
        bm83_addToQueue(&sentQueue, item); //add item to sent queue
    }
}

//callback for UART send events
void bm83_cmd_send_callback(DRV_USART_BUFFER_EVENT event, DRV_USART_BUFFER_HANDLE bufferHandle, uintptr_t context) {
    if (sentQueue.length == 0 || event == DRV_USART_BUFFER_EVENT_PENDING) return;//ignore "pending" events, queue should not be empty
    
    //find sent item with given handle
    BM83_QUEUE_ITEM* prev = NULL;
    BM83_QUEUE_ITEM* item = sentQueue.head; //start at head
    while (item->task->sendHandle != bufferHandle) { //iterate and look for correct item
        prev = item; //remember previous item
        item = item->next;
        if (item == NULL) return; //this means the item wasn't found: return
    }
    
    bm83_removeFromQueue(&sentQueue, item, prev); //remove item from sent queue
    
    BM83_COMMAND_TASK* task = item->task;
    if (event == DRV_USART_BUFFER_EVENT_COMPLETE) { //if transfer successful:
        task->previouslyFailed &= 0xfc; //clear send error flag
        task->receiveTimeoutTick = SYS_TIME_CounterGet() + ackTimeoutTicks; //calculate ACK timeout
        bm83_addToQueue(&ackQueue, item); //add item to waiting-for-ACK queue
    } else { //if transfer not successful:
        if (task->previouslyFailed & 2) { //already failed once: callback with error
            if (task->callback != NULL) task->callback(BM83_RESULT_OTHER_ERROR, NULL, 0, task->callbackContext);
            bm83_freeItem(item);
        } else { //not failed before: set failed flag, add back to unsent queue
            task->previouslyFailed |= 2;
            bm83_addToQueue(&unsentQueue, item);
        }
    }
}

//send an event ACK for a given event
bool bm83_ack_event(BM83_EVENT event) {
    uint8_t id[] = { event };
    return BM83_Queue_Command(BM83_CMD_EventAck, id, 1);
}

//handle an event received from UART (starting at buffer pointer with the given total packet length)
void bm83_handle_event(uint8_t* buffer, uint16_t length, uint8_t checksum) {
    if (length < 5) return; //event must at least have 5 bytes (sync(1) + length(2) + eventcode(1) + checksum(1))
    BM83_EVENT event = buffer[3];
    
    if (event == BM83_EVENT_CommandAck) { //ACK event
        if (length != 7 || checksum != 0 || ackQueue.length == 0) return;
        BM83_COMMAND ackCmd = buffer[4];
        
        //find item waiting for ACK with this code
        BM83_QUEUE_ITEM* prev = NULL;
        BM83_QUEUE_ITEM* item = ackQueue.head; //start at head
        while (item->task->command != ackCmd) { //iterate and look for correct item
            prev = item; //remember previous item
            item = item->next;
            if (item == NULL) return; //this means the item wasn't found: return
        }
        
        bm83_removeFromQueue(&ackQueue, item, prev); //remove item from queue
        
        BM83_COMMAND_TASK* task = item->task;
        if (buffer[5] == 0) { //status 0: command complete (ACK)
            if (task->expectedResponseEvent == BM83_EVENT_NONE || task->callback == NULL) { //no response event or callback expected: finish with callback
                if (task->callback != NULL) task->callback(BM83_RESULT_SUCCESS, NULL, 0, task->callbackContext);
                bm83_freeItem(item);
            } else { //response event expected: add to corresponsing queue
                task->receiveTimeoutTick = SYS_TIME_CounterGet() + responseTimeoutTicks;
                bm83_addToQueue(&responseQueue, item);
            }
        } else { //any other status: NACK
            if (task->callback != NULL) task->callback(BM83_RESULT_NACK, NULL, 0, task->callbackContext);
            bm83_freeItem(item);
        }
    } else { //other event (not ACK)
        BM83_QUEUE_ITEM* item = bm83_findAndRemoveEvent(&responseQueue, event); //find command expecting event
        if (item == NULL) { //no command expecting event found:
            BM83_COMMAND_TASK* newTask = malloc(sizeof(BM83_COMMAND_TASK)); //allocate task object
            if (newTask == NULL) return;

            //fill in parameters
            newTask->command = BM83_CMD_NONE;
            newTask->expectedResponseEvent = event;
            newTask->data = malloc(length);
            if (newTask->data == NULL) return;
            newTask->dataLength = length;
            newTask->callback = NULL;
            newTask->callbackContext = 0;
            newTask->previouslyFailed = checksum; //reuse for checksum value
            newTask->receiveTimeoutTick = SYS_TIME_CounterGet() + responseTimeoutTicks;
            memcpy(newTask->data, buffer, length); //copy data
            
            BM83_QUEUE_ITEM* newItem = malloc(sizeof(BM83_QUEUE_ITEM)); //allocate queue item
            if (newItem == NULL) return;
            newItem->task = newTask;
            bm83_addToQueue(&unhandledEventQueue, newItem); //add item to unhandled event queue
        } else if (item->task->callback != NULL) { //expecting command found and has callback:
            BM83_COMMAND_TASK* task = item->task;
            
            uint8_t* responseData = malloc(length);
            if (responseData == NULL) {
                bm83_freeItem(item);
                return;
            }
            memcpy(responseData, buffer, length);
            
            if (checksum == 0) task->callback(BM83_RESULT_SUCCESS, responseData, length, task->callbackContext);
            else task->callback(BM83_RESULT_CHECKSUM_MISMATCH, responseData, length, task->callbackContext);
            bm83_freeItem(item);
        }
        bm83_ack_event(event); //send event ACK
    }
}

//queue a command for sending and request callback upon completion/error (returns: queue success)
bool BM83_Queue_Command_Callback(BM83_COMMAND command, uint8_t* params, uint16_t paramLength, BM83_COMMAND_CALLBACK callback, uintptr_t context) {
    BM83_COMMAND_TASK* task = malloc(sizeof(BM83_COMMAND_TASK)); //allocate task object
    if (task == NULL) return false;
    
    //fill in parameters
    task->command = command;
    task->dataLength = paramLength + 5; //sync(1) + length(2) + command(1) + params(paramLength) + checksum(1)
    task->data = malloc(task->dataLength); //allocate data array
    if (task->data == NULL) {
        bm83_freeTask(task);
        return false;
    }
    task->expectedResponseEvent = bm83_getResponseEvent(command);
    task->callback = callback;
    task->callbackContext = context;
    task->previouslyFailed = 0;
    
    task->data[0] = 0xAA; //write sync byte
    uint16_t sentLength = paramLength + 1; //sent "length" parameter: command(1) + params(paramLength)
    task->data[1] = sentLength >> 8;
    task->data[2] = sentLength & 0xff; //write length parameter
    task->data[3] = command; //write command code
    memcpy(task->data + 4, params, paramLength); //copy params
    
    //calculate checksum (negated sum of message without sync byte)
    uint8_t sum = 0;
    uint16_t i;
    for (i = 1; i < task->dataLength - 1; i++) sum += task->data[i];
    task->data[task->dataLength - 1] = -sum; //write checksum
    
    BM83_QUEUE_ITEM* item = malloc(sizeof(BM83_QUEUE_ITEM)); //allocate queue item
    if (item == NULL) {
        bm83_freeTask(task);
        return false;
    }
    item->task = task;
    bm83_addToQueue(&unsentQueue, item); //add item to unsent queue
    
    return true;
}

//queue a command for sending without callback (returns: queue success)
bool BM83_Queue_Command(BM83_COMMAND command, uint8_t* params, uint16_t paramLength) {
    return BM83_Queue_Command_Callback(command, params, paramLength, NULL, 0);
}

//periodic tasks routine
void BM83_Tasks() {
    uint32_t tick = SYS_TIME_CounterGet();
    
    //receive new data
    uint16_t writePtr = DCH3DPTR;
    if (writePtr != uart_bufferReadPointer) { //new bytes written to receive buffer: process received messages
        uint16_t msgPos = 0; //position in current message
        uint16_t payloadLength = 0; //payload length of current message
        uint8_t crcSum = 0; //checksum of current message (should be 0 at the end)
        
        //virtual end of new bytes, guaranteed to be greater than read start
        uint16_t virtualWritePtr = uart_bufferReadPointer < writePtr ? writePtr : writePtr + UART_RECEIVE_BUFFER_LEN;
        
        //iterate over new received bytes, decode message structure, process complete received messages
        uint16_t readPtr;
        for (readPtr = uart_bufferReadPointer; readPtr < virtualWritePtr; readPtr++) {
            uint16_t realReadPtr = readPtr % UART_RECEIVE_BUFFER_LEN; //real read pointer: may wrap around to start of buffer
            uint8_t data = uart_receiveBuffer[realReadPtr];
            if (msgPos == 0) { //first byte: only accept sync word
                if(data == 0xAA) msgPos++;
            } else if (msgPos == 1) { //second byte: high byte of length
                payloadLength = data << 8;
                crcSum = data;
                msgPos++;
            } else if (msgPos == 2) { //third byte: low byte of length
                payloadLength |= data;
                crcSum += data;
                msgPos++;
            } else if (msgPos < payloadLength + 3) { //fourth to second-to-last bytes: payload
                crcSum += data;
                msgPos++;
            } else { //last byte: checksum; process packet and reset values for next packet
                crcSum += data;
                bm83_handle_event((uint8_t*)(uart_receiveBuffer + uart_bufferReadPointer), payloadLength + 4, crcSum);
                uart_bufferReadPointer = realReadPtr + 1; //message processed: set read pointer to start of next message
                msgPos = 0;
                payloadLength = 0;
                crcSum = 0;
            }
        }
    }
    
    //iterate through commands waiting for ack and resend/timeout if necessary
    BM83_QUEUE_ITEM* prev = NULL;
    BM83_QUEUE_ITEM* item = ackQueue.head;
    while (item != NULL) {
        BM83_COMMAND_TASK* task = item->task;
        
        if (tick - task->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //task timed out:
            if (task->previouslyFailed & 4) { //previously failed on ACK: callback and discard
                if (task->callback != NULL) task->callback(BM83_RESULT_TIMEOUT, NULL, 0, task->callbackContext);
                bm83_removeFromQueue(&ackQueue, item, prev);
                bm83_freeItem(item);
            } else { //not failed before: set failed flag, add back to unsent queue
                task->previouslyFailed |= 4;
                bm83_addToQueue(&unsentQueue, item);
            }
        }
        
        prev = item;
        item = item->next;
    }
    
    //iterate through unhandled events and process or discard them
    BM83_QUEUE_ITEM* prevEvent = NULL;
    BM83_QUEUE_ITEM* eventItem = unhandledEventQueue.head;
    while (eventItem != NULL) {
        BM83_COMMAND_TASK* eventTask = eventItem->task;
        BM83_QUEUE_ITEM* cmdItem = bm83_findAndRemoveEvent(&responseQueue, eventTask->expectedResponseEvent); //find+remove expecting command
        
        if (cmdItem == NULL) { //no expecting command found:
            if (tick - eventTask->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //if timed out: discard
                bm83_removeFromQueue(&unhandledEventQueue, eventItem, prevEvent);
                bm83_freeItem(eventItem);
            }
        } else if (cmdItem->task->callback != NULL) { //expecting command found and has callback
            BM83_COMMAND_TASK* cmdTask = cmdItem->task;
            
            //previouslyFailed is re-used as checksum result in event task
            if (eventTask->previouslyFailed == 0) cmdTask->callback(BM83_RESULT_SUCCESS, eventTask->data, eventTask->dataLength, cmdTask->callbackContext);
            else cmdTask->callback(BM83_RESULT_CHECKSUM_MISMATCH, eventTask->data, eventTask->dataLength, cmdTask->callbackContext);
            bm83_freeItem(cmdItem);
            
            //remove and free event item but keep event data
            bm83_removeFromQueue(&unhandledEventQueue, eventItem, prevEvent);
            free(eventTask);
            free(eventItem);
        }
        
        prevEvent = eventItem;
        eventItem = eventItem->next;
    }
    
    //iterate through commands waiting for response and timeout if necessary
    prev = NULL;
    item = responseQueue.head;
    while (item != NULL) {
        BM83_COMMAND_TASK* task = item->task;
        
        if (tick - task->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //task timed out: callback and timeout
            if (task->callback != NULL) task->callback(BM83_RESULT_TIMEOUT, NULL, 0, task->callbackContext);
            bm83_removeFromQueue(&responseQueue, item, prev);
            bm83_freeItem(item);
        }
        
        prev = item;
        item = item->next;
    }
    
    //send new data
    if (unsentQueue.length > 0) { //if items need to be sent: set MFB, send after 3ms
        BM_MFB_Set();
        SYS_TIME_CallbackRegisterMS(bm83_cmd_time_callback, 0, 3, SYS_TIME_SINGLE);
    }
}

/****************************/
/* Initialization functions */
/****************************/

//callback for init timer events
void bm83_init_time_callback(uintptr_t context) {
    switch (context) {
        case 0: //after reset hold: set MFB, wait 20ms
        {
            BM_MFB_Set();
            SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 1, 20, SYS_TIME_SINGLE);
            break;
        }
        case 1: //after reset+MFB hold: release reset, wait 500ms for init
        {
            BM_RST_N_Set();
            SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 2, 500, SYS_TIME_SINGLE);
            break;
        }
        case 2: //after init wait: clear MFB, start sending commands
        {
            BM_MFB_Clear();
            
        }
    }
}

//initialization of peripherals and drivers
void BM83_IO_Init() {
    IEC4bits.DMA3IE = 0; //disable DMA interrupt
    
    DCH3CON = 0; //reset DMA config and disable channel
    DCH3ECON = 0; //reset DMA event config
    DCH3INT = 0; //disable and clear all DMA channel interrupts
    DCH3ECONbits.CHAIRQ = 0xFF; //no IRQ for DMA abort
    DCH3ECONbits.CHSIRQ = _UART2_RX_VECTOR; //DMA start on UART RX interrupt
    DCH3ECONbits.SIRQEN = 1; //enable DMA start IRQ
    DCH3SSA = 0x1F822230; //DMA source: U2RXREG (physical address)
    DCH3DSA = UART_RECEIVE_BUFFER_PHYS_LOC; //DMA dest: receive buffer (physical address)
    DCH3SSIZ = 1; //DMA source size: 1 byte
    DCH3DSIZ = UART_RECEIVE_BUFFER_LEN; //DMA dest size: (buffer length) bytes
    DCH3CSIZ = 1; //DMA cell size: 1 byte
    DCH3CONbits.CHAEN = 1; //DMA auto-enable
    DCH3CONbits.CHEN = 1; //enable DMA channel
    
    drv = DRV_USART_Open(DRV_USART_INDEX_0, DRV_IO_INTENT_EXCLUSIVE); //open driver
    DRV_USART_BufferEventHandlerSet(drv, bm83_cmd_send_callback, 0); //set event handler
    
    ackTimeoutTicks = SYS_TIME_MSToCount(200); //calculate ACK and response timeout ticks
    responseTimeoutTicks = SYS_TIME_MSToCount(1000);
}

//initialization of module and UART protocol
void BM83_Module_Init() {
    bm83_state = BM83_NOT_INITIALIZED; //reset state
    bm83_clearQueue(&unsentQueue); //clear all queues
    bm83_clearQueue(&sentQueue);
    bm83_clearQueue(&ackQueue);
    bm83_clearQueue(&responseQueue);
    bm83_clearQueue(&unhandledEventQueue);
    BM_MFB_Clear();
    BM_RST_N_Clear(); //clear MFB and reset, resetting module
    SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 0, 200, SYS_TIME_SINGLE); //after 200ms: begin init sequence
}


/* *****************************************************************************
 End of File
 */
