/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "dap.h"


/***************************/
/* Variables & Definitions */
/***************************/

#define DAP_I2C_ADDRESS 0x1a
#define I2C_TIMEOUT_MAX_DIFF 100000000 //difference (tick - timeoutTick) is compared to this, greater than this means not timed out (very high value means negative difference)
#define DAP_TASK_LIST_LENGTH 100

typedef struct {
    uint8_t* data;
    uint16_t dataLength;
    bool write;
    uint32_t delay;
    uint8_t errorCount;
    DAP_COMMAND_CALLBACK callback;
    uintptr_t callbackContext;
    //uint8_t index;
    bool free;
    bool sent;
    bool waiting;
    uint32_t timeoutTick;
    DRV_I2C_TRANSFER_HANDLE handle;
} DAP_COMMAND_TASK;

/*typedef struct dap_queueitem {
    DAP_COMMAND_TASK* task;
    struct dap_queueitem* next;
} DAP_QUEUE_ITEM;

typedef struct {
    DAP_QUEUE_ITEM* head;
    DAP_QUEUE_ITEM* tail;
    uint16_t length;
} DAP_COMMAND_QUEUE;*/

const uint8_t __attribute__((keep)) inputMixerCfg[] = //input mixer: 0.5 * A + 0.5 * B (stereo mixed into mono)
{ 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t __attribute__((keep)) eqHifi_treble[] = //hifi EQ coefficients for treble channel
{ 0x00, 0x74, 0xAE, 0xDF, 0xFF, 0x16, 0xA2, 0x42, 0x00, 0x74, 0xAE, 0xDF, 0x00, 0xE8, 0x5D, 0x19, 0xFF, 0x95, 0xA1, 0x9C,
  0x00, 0x58, 0x54, 0x58, 0xFF, 0xB8, 0x0A, 0x85, 0x00, 0x25, 0x03, 0xAA, 0x00, 0x19, 0xF9, 0xCF, 0xFF, 0xD4, 0xAC, 0x52,
  0x00, 0x81, 0x0D, 0xDA, 0xFF, 0x05, 0xAC, 0xF7, 0x00, 0x7C, 0x16, 0xA7, 0x00, 0xFA, 0x53, 0x09, 0xFF, 0x82, 0xDB, 0x80,
  0x00, 0x80, 0x84, 0x19, 0xFF, 0x07, 0x39, 0x09, 0x00, 0x7C, 0x3D, 0xC2, 0x00, 0xF8, 0xC6, 0xF7, 0xFF, 0x83, 0x3E, 0x25,
  0x00, 0x8E, 0x24, 0x9D, 0xFF, 0x3A, 0xFB, 0x76, 0x00, 0x44, 0x5D, 0xFF, 0x00, 0xC5, 0x04, 0x8A, 0xFF, 0xAD, 0x7D, 0x65,
  0x00, 0x85, 0x17, 0x80, 0xFF, 0x6A, 0x74, 0x03, 0x00, 0x55, 0xAC, 0xA0, 0x00, 0x95, 0x8B, 0xFD, 0xFF, 0xA5, 0x3B, 0xE0,
  0x00, 0x1C, 0x2F, 0x57, 0x00, 0x38, 0x5E, 0xAE, 0x00, 0x1C, 0x2F, 0x57, 0x00, 0x27, 0x5E, 0x56, 0xFF, 0xE7, 0xE4, 0x4E };

const uint8_t __attribute__((keep)) eqHifi_bass[] = //hifi EQ coefficients for bass channel
{ 0x00, 0x00, 0x80, 0x53, 0x00, 0x01, 0x00, 0xA6, 0x00, 0x00, 0x80, 0x53, 0x00, 0xE8, 0x5D, 0x19, 0xFF, 0x95, 0xA1, 0x9C,
  0x00, 0x44, 0xC7, 0x60, 0xFF, 0x76, 0xBB, 0x80, 0x00, 0x44, 0x7D, 0x48, 0x00, 0xFF, 0x9A, 0xBD, 0xFF, 0x80, 0x65, 0x1B,
  0x00, 0x80, 0xB8, 0x6C, 0xFF, 0x01, 0xCD, 0x4C, 0x00, 0x7D, 0x85, 0x9B, 0x00, 0xFE, 0x32, 0xB4, 0xFF, 0x81, 0xC1, 0xF9,
  0x00, 0x80, 0x04, 0x17, 0xFF, 0x00, 0x69, 0x5E, 0x00, 0x7F, 0x9A, 0x6A, 0x00, 0xFF, 0x96, 0xA2, 0xFF, 0x80, 0x61, 0x7E,
  0x00, 0x80, 0x0A, 0xA1, 0xFF, 0x01, 0x6B, 0x82, 0x00, 0x7E, 0xCC, 0x30, 0x00, 0xFE, 0x94, 0x7E, 0xFF, 0x81, 0x29, 0x30,
  0x00, 0x80, 0x72, 0x0D, 0xFF, 0x01, 0xD0, 0x20, 0x00, 0x7E, 0x1F, 0x1F, 0x00, 0xFE, 0x2F, 0xE0, 0xFF, 0x81, 0x6E, 0xD5,
  0x00, 0x81, 0xE7, 0x34, 0xFF, 0x07, 0x26, 0x72, 0x00, 0x78, 0x7D, 0x53, 0x00, 0xF8, 0xD9, 0x8E, 0xFF, 0x85, 0x9B, 0x79 };

const uint32_t __attribute__((keep)) eqHifi_treble_volume = 0x054; //-3dB
const uint32_t __attribute__((keep)) eqHifi_bass_volume = 0x048; //0dB

const uint8_t __attribute__((keep)) eqPower_treble[] = //power EQ coefficients for treble channel
{ 0x00, 0x74, 0xAE, 0xDF, 0xFF, 0x16, 0xA2, 0x42, 0x00, 0x74, 0xAE, 0xDF, 0x00, 0xE8, 0x5D, 0x19, 0xFF, 0x95, 0xA1, 0x9C,
  0x00, 0x2A, 0x59, 0xE7, 0x00, 0x54, 0xB3, 0xCE, 0x00, 0x2A, 0x59, 0xE7, 0x00, 0x0B, 0xDA, 0xDE, 0xFF, 0xCA, 0xBD, 0x86,
  0x00, 0x7A, 0x9A, 0x31, 0xFF, 0x99, 0x22, 0x14, 0x00, 0x53, 0x21, 0xA8, 0x00, 0x66, 0xDD, 0xEC, 0xFF, 0xB2, 0x44, 0x27,
  0x00, 0x7B, 0x3C, 0x45, 0xFF, 0x5D, 0x4A, 0xE9, 0x00, 0x51, 0xDA, 0x4C, 0x00, 0xA2, 0xB5, 0x17, 0xFF, 0xB2, 0xE9, 0x6E,
  0x00, 0x80, 0x7D, 0x67, 0xFF, 0x07, 0x1A, 0x06, 0x00, 0x7B, 0x41, 0x66, 0x00, 0xF8, 0xE5, 0xFA, 0xFF, 0x84, 0x41, 0x32,
  0x00, 0x7F, 0xA2, 0x92, 0xFF, 0x09, 0xC2, 0x50, 0x00, 0x7B, 0xC3, 0x44, 0x00, 0xF6, 0x3D, 0xB0, 0xFF, 0x84, 0x9A, 0x2B,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t __attribute__((keep)) eqPower_bass[] = //power EQ coefficients for bass channel
{ 0x00, 0x00, 0x80, 0x53, 0x00, 0x01, 0x00, 0xA6, 0x00, 0x00, 0x80, 0x53, 0x00, 0xE8, 0x5D, 0x19, 0xFF, 0x95, 0xA1, 0x9C,
  0x00, 0x5A, 0xAC, 0xD3, 0xFF, 0x4B, 0x61, 0xBE, 0x00, 0x59, 0xF2, 0x30, 0x00, 0xFF, 0x21, 0x6F, 0xFF, 0x80, 0xDD, 0xD0,
  0x00, 0x80, 0x84, 0x96, 0xFF, 0x01, 0xCF, 0x63, 0x00, 0x7D, 0xB6, 0x0A, 0x00, 0xFE, 0x30, 0x9D, 0xFF, 0x81, 0xC5, 0x60,
  0x00, 0x80, 0xDD, 0x67, 0xFF, 0x04, 0x92, 0xFF, 0x00, 0x7A, 0xF1, 0x3B, 0x00, 0xFB, 0x6D, 0x01, 0xFF, 0x84, 0x31, 0x5E,
  0x00, 0x7F, 0xC7, 0xE5, 0xFF, 0x02, 0xAF, 0x8D, 0x00, 0x7E, 0x2D, 0x9F, 0x00, 0xFD, 0x50, 0x73, 0xFF, 0x82, 0x0A, 0x7C,
  0x00, 0x80, 0xCD, 0xE2, 0xFF, 0x05, 0xD2, 0x39, 0x00, 0x7A, 0xC8, 0xB2, 0x00, 0xFA, 0x2D, 0xC7, 0xFF, 0x84, 0x69, 0x6C,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint32_t __attribute__((keep)) eqPower_treble_volume = 0x060; //-6dB
const uint32_t __attribute__((keep)) eqPower_bass_volume = 0x048; //0dB

const uint8_t __attribute__((keep)) eqHifiOld_bass[] = //hifi old speaker EQ coefficients for bass channel
{ 0x00, 0x01, 0xD7, 0xE8, 0x00, 0x03, 0xAF, 0xD0, 0x00, 0x01, 0xD7, 0xE8, 0x00, 0xD1, 0x05, 0xEC, 0xFF, 0xA7, 0x9A, 0x74,
  0x00, 0x5A, 0xBF, 0xC7, 0xFF, 0x4C, 0x2B, 0x17, 0x00, 0x59, 0x19, 0x04, 0x00, 0xFE, 0x05, 0xAA, 0xFF, 0x81, 0xF6, 0x74,
  0x00, 0x7D, 0x02, 0x8F, 0xFF, 0x17, 0x7E, 0x16, 0x00, 0x72, 0x75, 0x67, 0x00, 0xE8, 0x81, 0xEA, 0xFF, 0x90, 0x88, 0x0A,
  0x00, 0x80, 0x32, 0xB9, 0xFF, 0x05, 0xCD, 0xB1, 0x00, 0x7A, 0x42, 0xDE, 0x00, 0xFA, 0x32, 0x4F, 0xFF, 0x85, 0x8A, 0x69,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t __attribute__((keep)) eqHifiOld_treble[] = //hifi old speaker EQ coefficients for treble channel
{ 0x00, 0x6A, 0x5A, 0xDE, 0xFF, 0x2B, 0x4A, 0x44, 0x00, 0x6A, 0x5A, 0xDE, 0x00, 0xD1, 0x05, 0xEC, 0xFF, 0xA7, 0x9A, 0x74,
  0x00, 0x81, 0x3C, 0x8C, 0xFF, 0x0D, 0x0C, 0x8C, 0x00, 0x7A, 0x66, 0x64, 0x00, 0xF2, 0xF3, 0x74, 0xFF, 0x84, 0x5D, 0x10,
  0x00, 0x80, 0xB2, 0xE6, 0xFF, 0x15, 0x97, 0x19, 0x00, 0x78, 0xD6, 0x11, 0x00, 0xEA, 0x68, 0xE7, 0xFF, 0x86, 0x77, 0x09,
  0x00, 0x82, 0x9D, 0x51, 0xFF, 0x8F, 0x11, 0x4E, 0x00, 0x6C, 0xF4, 0x8B, 0x00, 0x70, 0xEE, 0xB2, 0xFF, 0x90, 0x6E, 0x24,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint32_t __attribute__((keep)) eqHifiOld_treble_volume = 0x06e; //-9.5dB
const uint32_t __attribute__((keep)) eqHifiOld_bass_volume = 0x048; //0dB

const uint8_t __attribute__((keep)) bassTrebleBypass[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00 }; //bass and treble set to inline mode

static DRV_HANDLE dap_drv;

static uint32_t dap_sendTimeoutTicks; //how many ticks a transfer takes to time out
static uint32_t dap_sendPauseTicks; //how many ticks between transfers

static DAP_COMMAND_TASK dap_transferTasks[DAP_TASK_LIST_LENGTH];

bool dap_shutDown = true;
uint16_t dap_volume = 0x098;
bool dap_muted = true;

static bool dap_initInProgress = false;
static SUCCESS_CALLBACK dap_initSuccessCallback = NULL;

static uint32_t dap_sendNextAt;

/********************/
/* Helper functions */
/********************/

void dap_freeTask(DAP_COMMAND_TASK* task) {
    if (task == NULL) return;
    free(task->data);
    task->free = true;
}

DAP_COMMAND_TASK* dap_getFreeTask() {
    int i;
    for (i = 0; i < DAP_TASK_LIST_LENGTH; i++) {
        DAP_COMMAND_TASK* task = dap_transferTasks + i;
        if (task->free) return task;
    }
    return NULL;
}

void dap_clearTaskList() {
    int i;
    for (i = 0; i < DAP_TASK_LIST_LENGTH; i++) {
        DAP_COMMAND_TASK* task = dap_transferTasks + i;
        //task->index = i;
        task->free = true;
    }
}

/**********************/
/* Transfer functions */
/**********************/

//timer callback for command callback delay
void dap_callback_delay(uintptr_t context) {
    DAP_COMMAND_TASK* task = (DAP_COMMAND_TASK*)context;
    task->callback(true, task->data, task->dataLength, task->callbackContext);
    dap_freeTask(task);
}

//I2C transfer callback
void onI2CTransferEvent(DRV_I2C_TRANSFER_EVENT event, DRV_I2C_TRANSFER_HANDLE transferHandle, uintptr_t context) {
    if (event == DRV_I2C_TRANSFER_EVENT_PENDING) return; //ignore pending event
    
    DAP_COMMAND_TASK* task = NULL;
    int i;
    for (i = 0; i < DAP_TASK_LIST_LENGTH; i++) { //find corresponding task
        DAP_COMMAND_TASK* t = dap_transferTasks + i;
        if (!t->free && t->handle == transferHandle && t->sent) {
            task = t;
            break;
        }
    }
    if (task == NULL) return;
    
    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE) { //completed transfer: callback success
        if (task->callback) {
            if (task->delay > 0) { //if delay requested: set delay callback, otherwise callback immediately
                task->waiting = true;
                SYS_TIME_CallbackRegisterMS(dap_callback_delay, (uintptr_t)task, task->delay, SYS_TIME_SINGLE);
                return;
            } else task->callback(true, task->data, task->dataLength, task->callbackContext);
        }
    } else if (task->errorCount++ < 5) { //not completed: try again
        task->sent = false;
        return;
    } else if (task->callback) { //not completed: callback fail
        task->callback(false, task->data, task->dataLength, task->callbackContext);
    }
    
    dap_freeTask(task);
}

//write from buffer with callback
bool DAP_WriteBufferCallback(uint8_t subaddress, uint8_t* buffer, uint16_t length, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs) {
    DAP_COMMAND_TASK* task = dap_getFreeTask(); //get task object
    if (task == NULL) return false;
    
    task->free = false;
    task->dataLength = length + 1;
    task->data = malloc(task->dataLength); //allocate data array
    if (task->data == NULL) {
        dap_freeTask(task);
        return false;
    }
    task->write = true;
    task->delay = callbackDelayMs;
    task->callback = callback;
    task->callbackContext = context;
    task->errorCount = 0;
    task->sent = false;
    task->waiting = false;
    
    task->data[0] = subaddress;
    memcpy(task->data + 1, buffer, length); //copy data
    
    return true;
}

//write from buffer without callback
bool DAP_WriteBuffer(uint8_t subaddress, uint8_t* buffer, uint16_t length) {
    return DAP_WriteBufferCallback(subaddress, buffer, length, NULL, NULL, 0);
}

//write word with callback
bool DAP_WriteWordCallback(uint8_t subaddress, uint32_t value, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs) {
    uint8_t buffer[] = { value >> 24, (value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff };
    return DAP_WriteBufferCallback(subaddress, buffer, 4, callback, context, callbackDelayMs);
}

//write word without callback
bool DAP_WriteWord(uint8_t subaddress, uint32_t value) {
    return DAP_WriteWordCallback(subaddress, value, NULL, NULL, 0);
}

//write byte with callback
bool DAP_WriteByteCallback(uint8_t subaddress, uint8_t value, DAP_COMMAND_CALLBACK callback, uintptr_t context, uint32_t callbackDelayMs) {
    uint8_t buffer[] = { value };
    return DAP_WriteBufferCallback(subaddress, buffer, 1, callback, context, callbackDelayMs);
}

//write byte without callback
bool DAP_WriteByte(uint8_t subaddress, uint8_t value) {
    return DAP_WriteByteCallback(subaddress, value, NULL, NULL, 0);
}

//read data with callback
bool DAP_Read(uint8_t subaddress, uint16_t length, DAP_COMMAND_CALLBACK callback, uintptr_t context) {
    DAP_COMMAND_TASK* task = dap_getFreeTask(); //get task object
    if (task == NULL) return false;
    
    task->free = false;
    task->dataLength = length + 1;
    task->data = malloc(task->dataLength); //allocate data array
    if (task->data == NULL) {
        dap_freeTask(task);
        return false;
    }
    task->write = false;
    task->callback = callback;
    task->callbackContext = context;
    task->errorCount = 0;
    task->sent = false;
    task->waiting = false;
    
    task->data[0] = subaddress;
    
    return true;
}

void DAP_Tasks() {
    uint32_t tick = SYS_TIME_CounterGet();
    
    int i;
    for (i = 0; i < DAP_TASK_LIST_LENGTH; i++) {
        DAP_COMMAND_TASK* task = dap_transferTasks + i;
        if (task->free) continue;
        if (task->sent) {
            if (tick - task->timeoutTick < I2C_TIMEOUT_MAX_DIFF && !task->waiting) {
                task->sent = false;
                task->errorCount++;
            }
        } else if (tick - dap_sendNextAt < I2C_TIMEOUT_MAX_DIFF) {
        //} else {
            if (task->write) DRV_I2C_WriteTransferAdd(dap_drv, DAP_I2C_ADDRESS, task->data, task->dataLength, &task->handle); //write: simple write task
            else DRV_I2C_WriteReadTransferAdd(dap_drv, DAP_I2C_ADDRESS, task->data, 1, task->data + 1, task->dataLength - 1, &task->handle); //read: write subaddress, read data
            if (task->handle == DRV_I2C_TRANSFER_EVENT_HANDLE_INVALID) continue;
            task->sent = true;
            task->timeoutTick = tick + dap_sendTimeoutTicks;
            dap_sendNextAt = tick + dap_sendPauseTicks;
        }
    }
    if (tick - dap_sendNextAt < I2C_TIMEOUT_MAX_DIFF) dap_sendNextAt = tick;
}

/***********************/
/* Interface functions */
/***********************/

void dap_setVolumeCallback(bool success, uint8_t* buffer, uint16_t bufferLength, uintptr_t context) {
    if (success) {
        dap_volume = (buffer[3] << 8) | buffer[4];
    }
    if (context) ((SUCCESS_CALLBACK)context)(success);
}

bool DAP_SetVolume(uint16_t volume, SUCCESS_CALLBACK callback) {
    if (dap_shutDown) return false;
    if (dap_volume == volume) return false;
    return DAP_WriteWordCallback(0xd9, volume, dap_setVolumeCallback, (uintptr_t)callback, 70);
}

void dap_shutdown_timer_callback(uintptr_t context) {
    DAP_PDN_N_Clear();
    dap_shutDown = true;
    if (context) ((SUCCESS_CALLBACK)context)(true);
}

bool DAP_ShutDown(SUCCESS_CALLBACK callback) {
    if (dap_shutDown) return false;
    if (SYS_TIME_CallbackRegisterMS(dap_shutdown_timer_callback, (uintptr_t)callback, 70, SYS_TIME_SINGLE) != SYS_TIME_HANDLE_INVALID) {
        DAP_MUTE_N_Clear();
        return true;
    } else return false;
}

void dap_startup_timer_callback(uintptr_t context) {
    DAP_MUTE_N_Set();
    dap_shutDown = false;
    if (context) ((SUCCESS_CALLBACK)context)(true);
}

bool DAP_StartUp(SUCCESS_CALLBACK callback) {
    if (!dap_shutDown) return false;
    if (SYS_TIME_CallbackRegisterMS(dap_startup_timer_callback, (uintptr_t)callback, 10, SYS_TIME_SINGLE) != SYS_TIME_HANDLE_INVALID) {
        DAP_PDN_N_Set();
        return true;
    } else return false;
}

void dap_mute_timer_callback(uintptr_t context) {
    dap_muted = true;
    if (context) ((SUCCESS_CALLBACK)context)(true);
}

bool DAP_Mute(SUCCESS_CALLBACK callback) {
    if (dap_shutDown) return false;
    if (dap_muted) return false;
    if (SYS_TIME_CallbackRegisterMS(dap_mute_timer_callback, (uintptr_t)callback, 70, SYS_TIME_SINGLE) != SYS_TIME_HANDLE_INVALID) {
        DAP_MUTE_N_Clear();
        return true;
    } else return false;
}

void dap_unmute_timer_callback(uintptr_t context) {
    dap_muted = false;
    if (context) ((SUCCESS_CALLBACK)context)(true);
}

bool DAP_Unmute(SUCCESS_CALLBACK callback) {
    if (dap_shutDown) return false;
    if (!dap_muted) return false;
    if (SYS_TIME_CallbackRegisterMS(dap_unmute_timer_callback, (uintptr_t)callback, 70, SYS_TIME_SINGLE) != SYS_TIME_HANDLE_INVALID) {
        DAP_MUTE_N_Set();
        return true;
    } else return false;
}

/****************************/
/* Initialization functions */
/****************************/

void DAP_IO_Init() {
    dap_drv = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    DRV_I2C_TransferEventHandlerSet(dap_drv, onI2CTransferEvent, 0);
    
    dap_sendTimeoutTicks = SYS_TIME_MSToCount(1000);
    dap_sendPauseTicks = SYS_TIME_MSToCount(50);
}

void dap_init_cmd_callback(bool success, uint8_t* buffer, uint16_t bufferLength, uintptr_t context) {
    if (!dap_initInProgress) return;
    
    if (!success) {
        dap_initInProgress = false;
        if (dap_initSuccessCallback != NULL) dap_initSuccessCallback(false);
    }
    
    bool sendSuccess = true;
    //context 0 means no further action
    switch (context) {
        case 0: return;
        case 1:
            sendSuccess = DAP_WriteBufferCallback(0x47, (uint8_t*)inputMixerCfg, 32, dap_init_cmd_callback, 0, 0) //channel 7 input mixer
                        && DAP_WriteBufferCallback(0x48, (uint8_t*)inputMixerCfg, 32, dap_init_cmd_callback, 2, 0); //channel 8 input mixer
            break;
        case 2:
            sendSuccess = DAP_WriteBufferCallback(0x7b, (uint8_t*)eqHifiOld_treble, 140, dap_init_cmd_callback, 0, 0) //channel 7 biquad coefficients
                        && DAP_WriteBufferCallback(0x82, (uint8_t*)eqHifiOld_bass, 140, dap_init_cmd_callback, 3, 0); //channel 8 biquad coefficients
            break;
        case 3:
            sendSuccess = DAP_WriteBufferCallback(0x8f, (uint8_t*)bassTrebleBypass, 8, dap_init_cmd_callback, 0, 0) //channel 7 bass/treble inline
                        && DAP_WriteBufferCallback(0x90, (uint8_t*)bassTrebleBypass, 8, dap_init_cmd_callback, 0, 0) //channel 8 bass/treble inline
                        && DAP_WriteWordCallback(0xd7, eqHifiOld_treble_volume, dap_init_cmd_callback, 4, 70); //channel 7 volume
            break;
        case 4:
            sendSuccess = DAP_WriteWordCallback(0xd8, eqHifiOld_bass_volume, dap_init_cmd_callback, 5, 70); //channel 8 volume
            break;
        case 5:
            sendSuccess = DAP_WriteWordCallback(0xd9, 0x098, dap_init_cmd_callback, 6, 70); //master volume (init to -20dB)
            break;
        case 6:
            sendSuccess = DAP_WriteByteCallback(0x03, 0x80, dap_init_cmd_callback, 7, 0); //syscon1: soft unmute from error, enable channels
            break;
        case 7: //init done
            dap_initInProgress = false;
            dap_shutDown = false;
            dap_muted = true;
            dap_volume = 0x098;
            if (dap_initSuccessCallback != NULL) dap_initSuccessCallback(true);
            return;
    }
    if (!sendSuccess) {
        dap_initInProgress = false;
        if (dap_initSuccessCallback != NULL) dap_initSuccessCallback(false);
    }
}

//timer callback for chip init
void dap_init_time_callback(uintptr_t context) {
    bool success;
    switch (context) {
        case 0: //reset low, wait 10ms
            DAP_PDN_N_Clear();
            DAP_RESET_N_Clear();
            SYS_TIME_CallbackRegisterMS(dap_init_time_callback, 1, 10, SYS_TIME_SINGLE);
            break;
        case 1: //reset up, wait 10ms
            DAP_RESET_N_Set();
            SYS_TIME_CallbackRegisterMS(dap_init_time_callback, 2, 10, SYS_TIME_SINGLE);
            break;
        case 2: //PDN high, wait 10ms
            DAP_PDN_N_Set();
            SYS_TIME_CallbackRegisterMS(dap_init_time_callback, 3, 10, SYS_TIME_SINGLE);
            break;
        case 3: //reset done: do first writes
            dap_shutDown = false;
            success = DAP_WriteByteCallback(0x04, 0x02, dap_init_cmd_callback, 0, 0) //syscon2: disable SDOUT
                    && DAP_WriteByteCallback(0x14, 0x24, dap_init_cmd_callback, 0, 0) //input automute: -78dB threshold, 14.9ms delay
                    && DAP_WriteByteCallback(0x19, 0x22, dap_init_cmd_callback, 0, 0) //modulation limit: 97.7% for channels 7, 8
                    && DAP_WriteByteCallback(0x27, 0x3f, dap_init_cmd_callback, 1, 0); //channel shutdown: keep channels 1-6 shut down
            if (!success) {
                dap_initInProgress = false;
                if (dap_initSuccessCallback != NULL) dap_initSuccessCallback(false);
            }
            break;
    }
}

void DAP_Chip_Init(SUCCESS_CALLBACK callback) {
    dap_initInProgress = true;
    dap_initSuccessCallback = callback;
    
    dap_clearTaskList();
    dap_sendNextAt = SYS_TIME_CounterGet();
    
    DAP_MUTE_N_Clear(); //mute first, start reset after 100ms
    SYS_TIME_CallbackRegisterMS(dap_init_time_callback, 0, 1000, SYS_TIME_SINGLE);
}


/* *****************************************************************************
 End of File
 */
