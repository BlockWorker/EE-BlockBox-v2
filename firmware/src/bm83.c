/* ************************************************************************** */

#include "bm83.h"
#include "app.h"

/*****************************/
/* Definitions and variables */
/*****************************/

#define UART_RECEIVE_BUFFER_LEN 0x2000
#define UART_RECEIVE_BUFFER_PHYS_LOC 0x12000
#define UART_RECEIVE_BUFFER_VIRT_LOC (UART_RECEIVE_BUFFER_PHYS_LOC | 0x80000000)
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
BM83_SAMPLERATE bm83_samplerate = BM83_SR_UNKNOWN;
uint32_t bm83_samplerate_number = 0;
BM83_CODEC_STATUS bm83_codec_status = BM83_CODEC_IDLE;
bool bm83_pairing = false;
bool bm83_playing = false;
bool bm83_avrcp_connected = false;
bool bm83_abs_vol_supported = false;
uint8_t bm83_abs_vol = 0x00;
char* bm83_track_title = NULL;
char* bm83_track_artist = NULL;
uint32_t bm83_track_length_ms = 0;
uint32_t bm83_track_pos_ms = 0;

SUCCESS_CALLBACK bm83_init_callback = NULL;
bool bm83_sendQueued = false;

BM83_STATE_CHANGE_CALLBACK bm83_stateChangeCallback = NULL;

DRV_HANDLE bm83_drv;
volatile uint8_t uart_receiveBuffer[UART_RECEIVE_BUFFER_LEN] __attribute__((address(UART_RECEIVE_BUFFER_VIRT_LOC), keep, coherent));
uint16_t uart_bufferReadPointer = 0;

//command queues
BM83_COMMAND_QUEUE bm83_unsentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE bm83_sentQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE bm83_ackQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE bm83_responseQueue = { NULL, NULL, 0 };
BM83_COMMAND_QUEUE bm83_unhandledEventQueue = { NULL, NULL, 0 }; //events received that weren't expected by any command

uint32_t bm83_ackTimeoutTicks; //number of ticks equivalent to 200ms, calculated on init
uint32_t bm83_responseTimeoutTicks; //number of ticks equivalent to 1000ms, calculated on init

static uint32_t bm83_lastSendTick;

/********************/
/* Helper functions */
/********************/

//maps commands to their expected response events
BM83_EVENT bm83_getResponseEvent(BM83_COMMAND command) {
    switch (command) {
        case BM83_CMD_Read_BTM_Version: return BM83_EVENT_Read_BTM_Version_Reply;
        case BM83_CMD_AVC_Vendor_Dependent_Cmd: return BM83_EVENT_AVC_Vendor_Dependent_Response;
        default: return BM83_EVENT_NONE;
    }
}

//updates samplerate number
inline void bm83_update_sr() {
    switch (bm83_samplerate) {
        case BM83_SR_8K:
            bm83_samplerate_number = 8000;
            break;
        case BM83_SR_16K:
            bm83_samplerate_number = 16000;
            break;
        case BM83_SR_32K:
            bm83_samplerate_number = 32000;
            break;
        case BM83_SR_48K:
            bm83_samplerate_number = 48000;
            break;
        case BM83_SR_44_1K:
            bm83_samplerate_number = 44100;
            break;
        case BM83_SR_88K:
            bm83_samplerate_number = 88000;
            break;
        case BM83_SR_96K:
            bm83_samplerate_number = 96000;
            break;
        default:
            bm83_samplerate_number = 0;
            break;
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

inline void bm83_callStateChange(BM83_STATE_CHANGE_TYPE type) {
    if (bm83_stateChangeCallback) bm83_stateChangeCallback(type);
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
    if (queue->length == 0) return NULL; //empty list catch
    BM83_QUEUE_ITEM* item = queue->head; //get result (current head)
    if (item == NULL) { //catch error case
        queue->length = 0;
        return NULL;
    }
    queue->head = item->next; //update head
    if (--queue->length == 0) queue->tail = NULL; //remove tail pointer if last item removed
    item->next = NULL; //unlink result from list
    return item;
}

//clear queue, freeing all memory
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
    if (queue->length == 0 || queue->head == NULL) return NULL;
    
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
    if (bm83_unsentQueue.length == 0 || !bm83_sendQueued) return;
    
    BM_MFB_Clear();
    //uint16_t itemsLeft;
    
    //iterate through all items in unsent queue *currently* (not items added back in the loop)
    //for (itemsLeft = bm83_unsentQueue.length; itemsLeft > 0; itemsLeft--) {
        BM83_QUEUE_ITEM* item = bm83_takeFromQueue(&bm83_unsentQueue); //get item
        //if (item == NULL) break;
        if (item == NULL) {
            bm83_sendQueued = false;
            return;
        }
        BM83_COMMAND_TASK* task = item->task;
        DRV_USART_WriteBufferAdd(bm83_drv, task->data, task->dataLength, &task->sendHandle); //try sending
        if (task->sendHandle == DRV_USART_BUFFER_HANDLE_INVALID) { //if send request failed:
            if (task->previouslyFailed & 1) { //already failed once: callback with error
                if (task->callback != NULL) task->callback(BM83_RESULT_OTHER_ERROR, NULL, 0, task->callbackContext);
                bm83_freeItem(item);
            } else { //not failed before: set failed flag, add back to queue
                task->previouslyFailed |= 1;
                bm83_addToQueue(&bm83_unsentQueue, item);
            }
            bm83_sendQueued = false;
            return;
        }
        task->previouslyFailed &= 0xfe; //after success: reset send request fail flag
        bm83_addToQueue(&bm83_sentQueue, item); //add item to sent queue
    //}
    
    bm83_sendQueued = false; //send now performed
}

//callback for UART send events
void bm83_cmd_send_callback(DRV_USART_BUFFER_EVENT event, DRV_USART_BUFFER_HANDLE bufferHandle, uintptr_t context) {
    if (bm83_sentQueue.length == 0 || event == DRV_USART_BUFFER_EVENT_PENDING) return;//ignore "pending" events, queue should not be empty
    
    //find sent item with given handle
    BM83_QUEUE_ITEM* prev = NULL;
    BM83_QUEUE_ITEM* item = bm83_sentQueue.head; //start at head
    while (item->task->sendHandle != bufferHandle) { //iterate and look for correct item
        prev = item; //remember previous item
        item = item->next;
        if (item == NULL) return; //this means the item wasn't found: return
    }
    
    bm83_removeFromQueue(&bm83_sentQueue, item, prev); //remove item from sent queue
    
    BM83_COMMAND_TASK* task = item->task;
    if (event == DRV_USART_BUFFER_EVENT_COMPLETE) { //if transfer successful:
        if (task->command == BM83_CMD_EventAck) return; //if command is EventAck: no further response expected
        task->previouslyFailed &= 0xfc; //clear send error flag
        task->receiveTimeoutTick = SYS_TIME_CounterGet() + bm83_ackTimeoutTicks; //calculate ACK timeout
        bm83_addToQueue(&bm83_ackQueue, item); //add item to waiting-for-ACK queue
    } else { //if transfer not successful:
        if (task->previouslyFailed & 2) { //already failed once: callback with error
            if (task->callback != NULL) task->callback(BM83_RESULT_OTHER_ERROR, NULL, 0, task->callbackContext);
            bm83_freeItem(item);
        } else { //not failed before: set failed flag, add back to unsent queue
            task->previouslyFailed |= 2;
            bm83_addToQueue(&bm83_unsentQueue, item);
        }
    }
}

//send an event ACK for a given event
bool bm83_ack_event(BM83_EVENT event) {
    uint8_t id[] = { event };
    return BM83_Queue_Command(BM83_CMD_EventAck, id, 1);
}

bool bm83_request_track_data() {
    uint8_t params[] = { 0x00, 0x20, 0x00, 0x00, 0x15, 
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
                         0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07 };
    return BM83_Queue_Command(BM83_CMD_AVC_Vendor_Dependent_Cmd, params, 26);
}

bool bm83_request_caps() {
    uint8_t params[] = { 0x00, 0x10, 0x00, 0x00, 0x01, 0x03 };
    return BM83_Queue_Command(BM83_CMD_AVC_Vendor_Dependent_Cmd, params, 6);
}

void bm83_register_AVC_notifications(uintptr_t context) {
    /*uint8_t params1[] = { 0x00, 0x31, 0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00 };
    uint8_t params2[] = { 0x00, 0x31, 0x00, 0x00, 0x05, 0x05, 0x00, 0x00, 0x00, 0x01 };
    BM83_Queue_Command(BM83_CMD_AVC_Vendor_Dependent_Cmd, params1, 10);
    BM83_Queue_Command(BM83_CMD_AVC_Vendor_Dependent_Cmd, params2, 10);*/
    //bm83_request_track_data();
    bm83_request_caps();
}

bool bm83_init_finish_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context); //predef

bool bm83_handle_AVC_event(uint8_t* buffer, uint16_t length) {
    if (length < 17 || buffer[5] < 0x0D) return false;
    uint8_t pduID = buffer[11];
    uint8_t numAttributes;
    
    switch (pduID) {
        case 0x20: //element attributes
            numAttributes = buffer[15];
            if (length < 17 + 8 * numAttributes) return false;
            int offset = 16;
            int i;
            for (i = 0; i < numAttributes; i++) {
                uint32_t attrID = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
                uint16_t attrLength = (buffer[offset + 6] << 8) | buffer[offset + 7];
                offset += 8;
                switch (attrID) {
                    
                }
                offset += attrLength;
            }
            return true;
        case 0x31: //notification
            switch(buffer[15]) {
                case 0x01:
                    if (length < 18) return false;
                    bm83_playing = buffer[16] == 0x01;
                    bm83_callStateChange(BM83_CHANGE_PLAYBACK);
                    break;
                case 0x02:
                    bm83_request_track_data();
                    break;
                case 0x05:
                    if (length < 21) return false;
                    bm83_track_pos_ms = (buffer[16] << 24) | (buffer[17] << 16) | (buffer[18] << 8) | buffer[19];
                    if (bm83_track_pos_ms == 0xFFFFFFFF) bm83_track_pos_ms = 0;
                    bm83_callStateChange(BM83_CHANGE_PLAYBACK);
                    break;
            }
            return true;
        default: return false;
    }
}

//handle an event that happens asynchronously (as in, not as a command response)
bool bm83_handle_async_event(BM83_EVENT event, uint8_t* buffer, uint16_t length, uint8_t checksum) {
    const uint8_t bufferSize[] = { 0x04, 0x00 };
    
    if (checksum != 0) return false; //reject any checksum mismatch
    switch (event) {
        case BM83_EVENT_BTM_Status: //BTM status: update status or ignore if irrelevant
            if (length < 7) return false;
            switch (buffer[4]) {
                case 0x00:
                    bm83_state = BM83_OFF;
                    break;
                case 0x01:
                    bm83_pairing = true;
                    break;
                case 0x02:
                    bm83_state = BM83_BUSY;
                    break;
                case 0x03:
                case 0x04:
                    bm83_pairing = false;
                    break;
                case 0x06:
                    bm83_state = BM83_CONNECTED;
                    break;
                case 0x08:
                case 0x0F:
                    bm83_state = BM83_IDLE;
                    break;
                case 0x0B:
                    bm83_avrcp_connected = true;
                    //SYS_TIME_CallbackRegisterMS(bm83_register_AVC_notifications, 0, 2000, SYS_TIME_SINGLE);
                    break;
                case 0x0C:
                    bm83_avrcp_connected = false;
                    bm83_abs_vol_supported = false;
                    break;
                default:
                    return true;
            }
            bm83_callStateChange(BM83_CHANGE_STATE);
            return true;
        case BM83_EVENT_Ringtone_Status_Indication: //ringtone status: ignore for now
            return true;
        case BM83_EVENT_Report_Type_Codec: //codec status report
            if (length < 7) return false;
            bm83_samplerate = buffer[4];
            bm83_update_sr();
            bm83_codec_status = buffer[5];
            bm83_callStateChange(BM83_CHANGE_CODEC);
            return true;
        case BM83_EVENT_Report_BTM_Initial_Status: //initialization
            if (!BM83_Queue_Command_Callback(BM83_CMD_Rx_Buffer_Size, (uint8_t*)bufferSize, 2, bm83_init_finish_callback, NULL)) {
                if (bm83_init_callback) bm83_init_callback(false);
            }
            return true;
        case BM83_EVENT_Read_Linked_Device_Information_Reply: //here only AVRCP supported function reply
            if (length < 8 || buffer[5] != 0x03) return false;
            bm83_abs_vol_supported = buffer[6] >> 1;
            bm83_callStateChange(BM83_CHANGE_VOLUME);
            return true;
        case BM83_EVENT_Report_AVRCP_ABS_Volume_Level: //absolute volume
            if (length < 7) return false;
            bm83_abs_vol = buffer[5];
            bm83_callStateChange(BM83_CHANGE_VOLUME);
            return true;
        case BM83_EVENT_AVC_Vendor_Dependent_Response:
            return bm83_handle_AVC_event(buffer, length);
        default: return false;
    }
}

//handle an event that arrived asynchronously, is orphaned (command not acknowledged yet) or rejected by callback
void bm83_handleOrphanedEvent(BM83_EVENT event, uint8_t* buffer, uint16_t length, uint8_t checksum) {
    if (!bm83_handle_async_event(event, buffer, length, checksum)) { //check async event handling, if not handled:
        BM83_COMMAND_TASK* newTask = malloc(sizeof(BM83_COMMAND_TASK)); //allocate task object
        if (newTask == NULL) return;

        //fill in parameters
        newTask->command = BM83_CMD_NONE;
        newTask->expectedResponseEvent = event;
        newTask->data = malloc(length);
        if (newTask->data == NULL) {
            bm83_freeTask(newTask);
            return;
        }
        newTask->dataLength = length;
        newTask->callback = NULL;
        newTask->callbackContext = 0;
        newTask->previouslyFailed = checksum; //reuse for checksum value
        newTask->receiveTimeoutTick = SYS_TIME_CounterGet() + bm83_responseTimeoutTicks;
        memcpy(newTask->data, buffer, length); //copy data

        BM83_QUEUE_ITEM* newItem = malloc(sizeof(BM83_QUEUE_ITEM)); //allocate queue item
        if (newItem == NULL) {
            bm83_freeTask(newTask);
            return;
        }
        newItem->task = newTask;
        bm83_addToQueue(&bm83_unhandledEventQueue, newItem); //add item to unhandled event queue
    }
}

//handle an event received from UART (starting at buffer pointer with the given total packet length)
void bm83_handle_event(uint8_t* buffer, uint16_t length, uint8_t checksum) {
    if (length < 5) return; //event must at least have 5 bytes (sync(1) + length(2) + eventcode(1) + checksum(1))
    BM83_EVENT event = buffer[3];
    
    if (event == BM83_EVENT_CommandAck) { //ACK event
        if (length != 7 || checksum != 0 || bm83_ackQueue.length == 0) return;
        BM83_COMMAND ackCmd = buffer[4];
        
        //find item waiting for ACK with this code
        BM83_QUEUE_ITEM* prev = NULL;
        BM83_QUEUE_ITEM* item = bm83_ackQueue.head; //start at head
        while (item->task->command != ackCmd) { //iterate and look for correct item
            prev = item; //remember previous item
            item = item->next;
            if (item == NULL) return; //this means the item wasn't found: return
        }
        
        bm83_removeFromQueue(&bm83_ackQueue, item, prev); //remove item from queue
        
        BM83_COMMAND_TASK* task = item->task;
        if (buffer[5] == 0) { //status 0: command complete (ACK)
            if (task->expectedResponseEvent == BM83_EVENT_NONE || task->callback == NULL) { //no response event or callback expected: finish with callback
                if (task->callback != NULL) task->callback(BM83_RESULT_SUCCESS, NULL, 0, task->callbackContext);
                bm83_freeItem(item);
            } else { //response event expected: add to corresponsing queue
                task->receiveTimeoutTick = SYS_TIME_CounterGet() + bm83_responseTimeoutTicks;
                bm83_addToQueue(&bm83_responseQueue, item);
            }
        } else { //any other status: NACK
            if (task->callback != NULL) task->callback(BM83_RESULT_NACK, NULL, 0, task->callbackContext);
            bm83_freeItem(item);
        }
    } else { //other event (not ACK)
        BM83_QUEUE_ITEM* item = bm83_findAndRemoveEvent(&bm83_responseQueue, event); //find command expecting event
        if (item && item->task->callback) { //expecting command found and has callback:
            BM83_COMMAND_TASK* task = item->task;
            
            uint8_t* responseData = malloc(length);
            if (responseData == NULL) {
                bm83_freeItem(item);
                return;
            }
            memcpy(responseData, buffer, length);
            
            bool handled;
            if (checksum == 0) handled = task->callback(BM83_RESULT_SUCCESS, responseData, length, task->callbackContext);
            else handled = task->callback(BM83_RESULT_CHECKSUM_MISMATCH, responseData, length, task->callbackContext);
            if (handled) {
                bm83_freeItem(item);
                free(responseData);
            } else {
                bm83_addToQueue(&bm83_responseQueue, item);
                bm83_handleOrphanedEvent(event, buffer, length, checksum);
            }
        } else bm83_handleOrphanedEvent(event, buffer, length, checksum);
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
    bm83_addToQueue(&bm83_unsentQueue, item); //add item to unsent queue
    
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
                else uart_bufferReadPointer = realReadPtr + 1; //not sync word: skip byte
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
    BM83_QUEUE_ITEM* item = bm83_ackQueue.head;
    while (item != NULL) {
        BM83_COMMAND_TASK* task = item->task;
        
        if (tick - task->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //task timed out:
            if (task->previouslyFailed & 4) { //previously failed on ACK: callback and discard
                if (task->callback != NULL) task->callback(BM83_RESULT_TIMEOUT, NULL, 0, task->callbackContext);
                bm83_removeFromQueue(&bm83_ackQueue, item, prev);
                bm83_freeItem(item);
            } else { //not failed before: set failed flag, add back to unsent queue
                task->previouslyFailed |= 4;
                bm83_addToQueue(&bm83_unsentQueue, item);
            }
        }
        
        prev = item;
        item = item->next;
    }
    
    //iterate through unhandled events and process or discard them
    BM83_QUEUE_ITEM* prevEvent = NULL;
    BM83_QUEUE_ITEM* eventItem = bm83_unhandledEventQueue.head;
    while (eventItem != NULL) {
        BM83_COMMAND_TASK* eventTask = eventItem->task;
        BM83_QUEUE_ITEM* cmdItem = bm83_findAndRemoveEvent(&bm83_responseQueue, eventTask->expectedResponseEvent); //find+remove expecting command
        
        if (cmdItem == NULL) { //no expecting command found:
            if (tick - eventTask->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //if timed out: discard
                bm83_removeFromQueue(&bm83_unhandledEventQueue, eventItem, prevEvent);
                bm83_freeItem(eventItem);
            }
        } else if (cmdItem->task->callback != NULL) { //expecting command found and has callback
            BM83_COMMAND_TASK* cmdTask = cmdItem->task;
            
            //previouslyFailed is re-used as checksum result in event task
            if (eventTask->previouslyFailed == 0) cmdTask->callback(BM83_RESULT_SUCCESS, eventTask->data, eventTask->dataLength, cmdTask->callbackContext);
            else cmdTask->callback(BM83_RESULT_CHECKSUM_MISMATCH, eventTask->data, eventTask->dataLength, cmdTask->callbackContext);
            bm83_freeItem(cmdItem);
            
            //remove and free event item but keep event data
            bm83_removeFromQueue(&bm83_unhandledEventQueue, eventItem, prevEvent);
            free(eventTask);
            free(eventItem);
        }
        
        prevEvent = eventItem;
        eventItem = eventItem->next;
    }
    
    //iterate through commands waiting for response and timeout if necessary
    prev = NULL;
    item = bm83_responseQueue.head;
    while (item != NULL) {
        BM83_COMMAND_TASK* task = item->task;
        
        if (tick - task->receiveTimeoutTick < UART_TIMEOUT_MAX_DIFF) { //task timed out: callback and timeout
            if (task->callback != NULL) task->callback(BM83_RESULT_TIMEOUT, NULL, 0, task->callbackContext);
            bm83_removeFromQueue(&bm83_responseQueue, item, prev);
            bm83_freeItem(item);
        }
        
        prev = item;
        item = item->next;
    }
    
    if (SYS_TIME_CounterGet() - bm83_lastSendTick - bm83_ackTimeoutTicks < UART_TIMEOUT_MAX_DIFF) { //time out stuck send queue
        bm83_sendQueued = false;
    }
    
    //send new data
    if (bm83_unsentQueue.length > 0 && !bm83_sendQueued) { //if items need to be sent: set MFB, send after 3ms
        BM_MFB_Set();
        bm83_lastSendTick = SYS_TIME_CounterGet();
        SYS_TIME_CallbackRegisterMS(bm83_cmd_time_callback, 0, 3, SYS_TIME_SINGLE);
        bm83_sendQueued = true; //prevent more calls before timeout
    }
}

void BM83_SetStateChangeCallback(BM83_STATE_CHANGE_CALLBACK callback) {
    bm83_stateChangeCallback = callback;
}

/******************************/
/* Compound command functions */
/******************************/

//final callback for simple action sequence: call back success/failure
bool bm83_action_final_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context) {
    if (context == NULL) return true;
    if (result == BM83_RESULT_SUCCESS) {
        ((SUCCESS_CALLBACK)context)(true);
    } else {
        ((SUCCESS_CALLBACK)context)(false);
    }
    return true;
}

//simple MMI action call
inline void bm83_MMI_action(const uint8_t* action, BM83_COMMAND_CALLBACK cmd_cb, SUCCESS_CALLBACK suc_cb) {
    if (!BM83_Queue_Command_Callback(BM83_CMD_MMI_Action, (uint8_t*)action, 2, cmd_cb, (uintptr_t)suc_cb)) {
        if (suc_cb) suc_cb(false);
    }
}

//first callback for off sequence: release on button or call back failure
bool bm83_pwrOn_first_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context) {
    const uint8_t action[] = { 0x00, 0x52 };
    SUCCESS_CALLBACK cb = (SUCCESS_CALLBACK)context;
    if (result == BM83_RESULT_SUCCESS) bm83_MMI_action(action, bm83_action_final_callback, cb);
    else if (cb) cb(false);
    return true;
}

//power on module: press on button
void BM83_PowerOn(SUCCESS_CALLBACK callback) {
    const uint8_t action[] = { 0x00, 0x51 };
    bm83_MMI_action(action, bm83_pwrOn_first_callback, callback);
}

//first callback for off sequence: release off button or call back failure
bool bm83_pwrOff_first_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context) {
    const uint8_t action[] = { 0x00, 0x54 };
    SUCCESS_CALLBACK cb = (SUCCESS_CALLBACK)context;
    if (result == BM83_RESULT_SUCCESS) bm83_MMI_action(action, bm83_action_final_callback, cb);
    else if (cb != NULL) cb(false);
    return true;
}

//power off module: press off button
void BM83_PowerOff(SUCCESS_CALLBACK callback) {
    const uint8_t action[] = { 0x00, 0x53 };
    bm83_MMI_action(action, bm83_pwrOff_first_callback, callback);
}

void BM83_EnterPairing(SUCCESS_CALLBACK callback) {
    const uint8_t action[] = { 0x00, 0x5D };
    bm83_MMI_action(action, bm83_action_final_callback, callback);
}

void BM83_ExitPairing(SUCCESS_CALLBACK callback) {
    const uint8_t action[] = { 0x00, 0x6B };
    bm83_MMI_action(action, bm83_action_final_callback, callback);
}

void BM83_DisconnectLink(SUCCESS_CALLBACK callback) {
    uint8_t params[] = { 0x3E };
    if (!BM83_Queue_Command_Callback(BM83_CMD_Disconnect, params, 1, bm83_action_final_callback, (uintptr_t)callback)) {
        if (callback) callback(false);
    }
}

void BM83_SetAbsVolume(uint8_t absVol, SUCCESS_CALLBACK callback) {
    if (!bm83_abs_vol_supported || absVol > 0x7F) {
        if (callback) callback(false);
        return;
    }
    uint8_t params[] = { 0x00, 0x01, 0x04, absVol, 0x00, 0x00 };
    if (!BM83_Queue_Command_Callback(BM83_CMD_Set_Overall_Gain, params, 6, bm83_action_final_callback, (uintptr_t)callback)) {
        if (callback) callback(false);
        return;
    }
    bm83_abs_vol = absVol;
}

void BM83_PlayPause(SUCCESS_CALLBACK callback) {
    uint8_t params[] = { 0x00, 0x07 };
    if (!BM83_Queue_Command_Callback(BM83_CMD_Music_Control, params, 2, bm83_action_final_callback, (uintptr_t)callback)) {
        if (callback) callback(false);
    }
}

void BM83_NextTrack(SUCCESS_CALLBACK callback) {
    uint8_t params[] = { 0x00, 0x09 };
    if (!BM83_Queue_Command_Callback(BM83_CMD_Music_Control, params, 2, bm83_action_final_callback, (uintptr_t)callback)) {
        if (callback) callback(false);
    }
}

void BM83_PrevTrack(SUCCESS_CALLBACK callback) {
    uint8_t params[] = { 0x00, 0x0A };
    if (!BM83_Queue_Command_Callback(BM83_CMD_Music_Control, params, 2, bm83_action_final_callback, (uintptr_t)callback)) {
        if (callback) callback(false);
    }
}

/****************************/
/* Initialization functions */
/****************************/

//callback for initialization finish
bool bm83_init_finish_callback(BM83_COMMAND_RESULT result, uint8_t* response, uint16_t responseLength, uintptr_t context) {
    if (result == BM83_RESULT_SUCCESS) { //successfully set buffer size: go to off state, callback with success
        bm83_state = BM83_OFF;
        if (bm83_init_callback != NULL) bm83_init_callback(true);
        bm83_callStateChange(BM83_CHANGE_STATE);
    } else { //failed: callback with fail
        if (bm83_init_callback != NULL) bm83_init_callback(false);
    }
    return true;
}

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
        case 2: //after init wait: clear MFB
        {
            BM_MFB_Clear();
            break;
        }
    }
}

//initialization of peripherals and drivers
void BM83_IO_Init() {
    IEC4bits.U2RXIE = 0; //disable UART receive interrupt
    IEC4bits.DMA3IE = 0; //disable DMA interrupt
    
    DCH3CON = 0; //reset DMA config and disable channel
    DCH3ECON = 0; //reset DMA event config
    DCH3INT = 0; //disable and clear all DMA channel interrupts
    DCH3ECONbits.CHAIRQ = 0xFF; //no IRQ for DMA abort
    DCH3ECONbits.CHSIRQ = _UART2_RX_VECTOR; //DMA start on UART RX interrupt
    DCH3ECONbits.SIRQEN = 1; //enable DMA start IRQ
    DCH3SSA = KVA_TO_PA(&U2RXREG); //0x1F822230; //DMA source: U2RXREG (physical address)
    DCH3DSA = UART_RECEIVE_BUFFER_PHYS_LOC; //DMA dest: receive buffer (physical address)
    DCH3SSIZ = 1; //DMA source size: 1 byte
    DCH3DSIZ = UART_RECEIVE_BUFFER_LEN; //DMA dest size: (buffer length) bytes
    DCH3CSIZ = 1; //DMA cell size: 1 byte
    DCH3CONbits.CHAEN = 1; //DMA auto-enable
    DCH3CONbits.CHEN = 1; //enable DMA channel
    
    uart_bufferReadPointer = 0; //reset buffer read pointer
    
    bm83_drv = DRV_USART_Open(DRV_USART_INDEX_0, DRV_IO_INTENT_EXCLUSIVE); //open driver
    DRV_USART_BufferEventHandlerSet(bm83_drv, bm83_cmd_send_callback, 0); //set event handler
    
    bm83_ackTimeoutTicks = SYS_TIME_MSToCount(200); //calculate ACK and response timeout ticks
    bm83_responseTimeoutTicks = SYS_TIME_MSToCount(1000);
}

//initialization of module and UART protocol
void BM83_Module_Init(SUCCESS_CALLBACK callback) {
    bm83_state = BM83_NOT_INITIALIZED; //reset state
    bm83_clearQueue(&bm83_unsentQueue); //clear all queues
    bm83_clearQueue(&bm83_sentQueue);
    bm83_clearQueue(&bm83_ackQueue);
    bm83_clearQueue(&bm83_responseQueue);
    bm83_clearQueue(&bm83_unhandledEventQueue);
    bm83_init_callback = callback;
    
    BM_MFB_Clear();
    BM_RST_N_Clear(); //clear MFB and reset, resetting module
    SYS_TIME_CallbackRegisterMS(bm83_init_time_callback, 0, 200, SYS_TIME_SINGLE); //after 300ms: begin init sequence
}


/* *****************************************************************************
 End of File
 */
