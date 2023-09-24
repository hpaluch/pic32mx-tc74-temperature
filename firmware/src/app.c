/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
  Main Application logic.
  - Timer example follows:
    - https://microchip-mplab-harmony.github.io/core/GUID-EE734734-7914-41BF-8AF2-8275506BBED4.html
  - I2C based on example:
    - https://microchip-mplab-harmony.github.io/core/GUID-8916AA7D-64C7-4477-8D26-664F6B3C242A.html
    - https://github.com/Microchip-MPLAB-Harmony/core_apps_pic32mx/tree/master/apps/driver/i2c/async/i2c_eeprom
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

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define APP_VERSION 104 // 123 = 1.23
#define LED_BLINK_RATE_MS         500
#define APP_MINIMUM_PAUSE_US 1000
// TC74 I2C Address - WARNING! You have to read it from package and
// use proper address. My is TC74A0
// Where A0 according to datasheet is 0x48
#define APP_TC74_SLAVE_ADDR_A0 0x48
#define APP_TC74_SLAVE_ADDR_A5 0x4D
#define APP_TC74_SLAVE_ADDR APP_TC74_SLAVE_ADDR_A0
// TEMPerature register in TC74
#define APP_TC74_REG_TEMP 0
// CONFIG register in TC74
#define APP_TC74_REG_CONFIG 1
#define APP_TC74_CONFIG_STANDBY_MASK 0x80
#define APP_TC74_CONFIG_READY_MASK 0x40
#define APP_TC74_CONFIG_ZERO_MASK 0x3f
#define APP_TC74_CONFIG_STATUS_MASK (APP_TC74_CONFIG_STANDBY_MASK|APP_TC74_CONFIG_READY_MASK)
// short version of __FILE__ without path
static const char *APP_FILE = "app.c";
// improved macro that will print file and line of message
#define APP_CONSOLE_PRINT(fmt,...) SYS_CONSOLE_PRINT("%s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)
// simple form  without any added content
#define APP_CONSOLE_PRINT_RAW(fmt,...) SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#define APP_ERROR_PRINT(fmt,...) SYS_DEBUG_PRINT(SYS_ERROR_ERROR, "ERROR: %s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)

#define APP_ERROR_PRINT_AND_STATE(fmt,...) \
    do{ APP_ERROR_PRINT(fmt, ##__VA_ARGS__); \
        appData.state = APP_STATE_FATAL_ERROR; \
    }while(0)


#define APP_ERROR_PRINT_AND_JUMP(lab,fmt,...) \
    do{ APP_ERROR_PRINT(fmt, ##__VA_ARGS__); \
        appData.state = APP_STATE_FATAL_ERROR; \
        goto lab; \
    }while(0)
// assigns result of function "fn" to "ret" and reports
// and change state to error if "ret" == "err"
#define APP_CHECK_ERROR(ret,fn,err,lab) \
    if ( ((ret) = (fn)) == err  ){ \
        APP_ERROR_PRINT("%s failed with %s. appState=%d",#fn,#err,appData.state);  \
        appData.state = APP_STATE_FATAL_ERROR; \
        goto lab; \
    }

// fail with error when ret != ok
#define APP_CHECK_ERROR_NEQ(ret,fn,ok,lab) \
    if ( ((ret) = (fn)) != ok  ){ \
        APP_ERROR_PRINT("%s failed with != %s. appState=%d",#fn,#ok,appData.state);  \
        appData.state = APP_STATE_FATAL_ERROR; \
        goto lab; \
    }

// variant where function returns void and we have to check other variable
#define APP_CHECK_ERROR_VOID(ret,fn,err,lab) \
    (fn); \
    if ( (ret) == err  ){ \
        APP_ERROR_PRINT("%s failed with %s. appState=%d",#fn,#err,appData.state);  \
        appData.state = APP_STATE_FATAL_ERROR; \
        goto lab; \
    }

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

void Timer1_Callback ( uintptr_t context )
{
    if (appData.state != APP_STATE_FATAL_ERROR){
        RA0_LED_Toggle();
    }
}

// from harmony-repo\core_apps_pic32mx\apps\driver\i2c\async\i2c_eeprom\firmware\src\app.c
void APP_I2CEventHandler (
    DRV_I2C_TRANSFER_EVENT event,
    DRV_I2C_TRANSFER_HANDLE transferHandle,
    uintptr_t context
)
{
    APP_TRANSFER_STATUS* transferStatus = (APP_TRANSFER_STATUS*)context;

    appData.i2cEvent = event;
    if (event == DRV_I2C_TRANSFER_EVENT_COMPLETE){
        if (transferStatus){
            *transferStatus = APP_TRANSFER_STATUS_SUCCESS;
        }
    } else {
        if (transferStatus){
            *transferStatus = APP_TRANSFER_STATUS_ERROR;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

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

void APP_Initialize ( void )
{
    // NOTE: Only data should be initialized here, because it is called
    // very early from initialization.c!
    // Do not attempt to call API from here!
    appData.state = APP_STATE_INIT;
    appData.ledTimerHandle = SYS_TIME_HANDLE_INVALID;
    appData.drvI2CHandle = DRV_HANDLE_INVALID;
    appData.pauseTimer = SYS_TIME_HANDLE_INVALID;
    appData.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    appData.transferStatus = APP_TRANSFER_STATUS_ERROR;
    appData.rxData[0] = 0;
    appData.txData[0] = 0;
    appData.txData[1] = 0;
    appData.iter = 0;
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            APP_CONSOLE_PRINT_RAW("\r\n");
            APP_CONSOLE_PRINT("Starting app v%d.%02d",
                    APP_VERSION/100,APP_VERSION%100);
            
            APP_CHECK_ERROR(appData.ledTimerHandle,
                    SYS_TIME_CallbackRegisterMS(Timer1_Callback,
                        0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC),
                    SYS_TIME_HANDLE_INVALID,InitErrorJump);
            
            appData.state = APP_STATE_INIT_I2C;
            // here jumps APP_CHECK_ERROR() macro in case of error:
            InitErrorJump:;
        }
        break;

        case APP_STATE_INIT_I2C:
        {
            APP_CHECK_ERROR(appData.drvI2CHandle,
                    DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE),
                    DRV_HANDLE_INVALID, InitI2cErrorJump);
            
            appData.transferStatus = APP_TRANSFER_STATUS_IDLE;
            /* Register the I2C Driver event Handler */
            DRV_I2C_TransferEventHandlerSet(
                appData.drvI2CHandle,
                APP_I2CEventHandler,
                (uintptr_t)&appData.transferStatus
            );

            APP_CHECK_ERROR_VOID(appData.transferHandle,
                DRV_I2C_ReadTransferAdd(
                    appData.drvI2CHandle, APP_TC74_SLAVE_ADDR,
                    (void *)appData.rxData, 1, &appData.transferHandle),
                DRV_I2C_TRANSFER_HANDLE_INVALID, InitI2cErrorJump);
            appData.state = APP_STATE_I2C_TEST_READ;
            // here jumps APP_CHECK_ERROR() macro in case of error:
            InitI2cErrorJump:;
        }
        break;

        case APP_STATE_I2C_TEST_READ:
        {
            // test if any I2C device responds with TC74 slave address
            // read should be non-destructive for all I2C devices.
            if(appData.transferStatus == APP_TRANSFER_STATUS_SUCCESS){
                APP_CONSOLE_PRINT("OK: I2C ACK response from dev at ADDR=0x%x. Data=0x%x",
                        APP_TC74_SLAVE_ADDR, appData.rxData[0]);
                appData.state = APP_STATE_I2C_QUERY_CONFIG;
            } else if (appData.transferStatus == APP_TRANSFER_STATUS_ERROR){
                APP_ERROR_PRINT_AND_JUMP(I2cTestReadErrorJump,
                        "I2C Read from ADDR=0x%x failed. Is TC74 connected? i2cEvent=%d",
                        APP_TC74_SLAVE_ADDR, appData.i2cEvent);
            } else {
                // other states: transfer in progress, do nothing...
            }
            I2cTestReadErrorJump:;
        }
        break;

        case APP_STATE_I2C_QUERY_CONFIG:
        {
            // select and query CONFIG register to know if TC74 is Up and
            // and Ready to read TEMPerature
            appData.txData[0] = APP_TC74_REG_CONFIG; // register we want to select
            appData.rxData[0] = 0; // clean read buffer
            appData.transferStatus = APP_TRANSFER_STATUS_IDLE;
            APP_CHECK_ERROR_VOID(appData.transferHandle,
                DRV_I2C_WriteReadTransferAdd(appData.drvI2CHandle, APP_TC74_SLAVE_ADDR,
                    appData.txData, 1, appData.rxData, 1,&appData.transferHandle),
                DRV_I2C_TRANSFER_HANDLE_INVALID, I2cQueryConfigErrorJump);
            appData.state = APP_STATE_I2C_QUERY_CONFIG_READ;
            I2cQueryConfigErrorJump:;
        }
        break;

        case APP_STATE_I2C_QUERY_CONFIG_READ:
        {
            static uint8_t oldCfg = ~0;
            // test if CONFIG register was read without error
            if(appData.transferStatus == APP_TRANSFER_STATUS_SUCCESS){
                uint8_t cfg = appData.rxData[0];
                if (cfg != oldCfg){
                    APP_CONSOLE_PRINT("Data from TC74 at ADDR=0x%x: CONFIG=0x%x %s %s zero mask: 0x%x",
                            APP_TC74_SLAVE_ADDR, cfg,
                            cfg & APP_TC74_CONFIG_STANDBY_MASK ? "STANDBY" : "UP",
                            cfg & APP_TC74_CONFIG_READY_MASK ? "READY" : "BUSY",
                            cfg & APP_TC74_CONFIG_ZERO_MASK);
                    oldCfg = cfg;
                }
                if (cfg & APP_TC74_CONFIG_ZERO_MASK){
                APP_ERROR_PRINT_AND_JUMP(I2cQueryConfigReadErrorJump,
                        "Invalid CONFIG=0x%x LSB bits==0x%x - should be 0. Is target device TC74?",
                        cfg, cfg & APP_TC74_CONFIG_ZERO_MASK);
                }
                if ( (cfg & APP_TC74_CONFIG_STATUS_MASK) == APP_TC74_CONFIG_READY_MASK ){
                    appData.state = APP_STATE_I2C_QUERY_TEMP;
                } else if (cfg & APP_TC74_CONFIG_STANDBY_MASK){
                    appData.state = APP_STATE_I2C_WAKEUP_TC74;
                } else if ((cfg & APP_TC74_CONFIG_READY_MASK)==0){
                    appData.state = APP_STATE_I2C_BUSY_TC74;
                } else {
                    APP_ERROR_PRINT_AND_JUMP(I2cQueryConfigReadErrorJump,
                        "Unexpected Config state=0x%x CONFIG=0x%x - INERNAL ERROR",
                        cfg & APP_TC74_CONFIG_STATUS_MASK, cfg);
                }
            } else if (appData.transferStatus == APP_TRANSFER_STATUS_ERROR){
                APP_ERROR_PRINT_AND_JUMP(I2cQueryConfigReadErrorJump,
                        "I2C Read CONFIG from ADDR=0x%x failed. Is target device TC74? i2cEvent=%d",
                        APP_TC74_SLAVE_ADDR, appData.i2cEvent);
            } else {
                // other states: transfer in progress, do nothing...
            }
            I2cQueryConfigReadErrorJump:;
        }
        break;

        case APP_STATE_I2C_WAKEUP_TC74:
        {
            appData.txData[0] = APP_TC74_REG_CONFIG; // register we want to select
            appData.txData[1] = 0; // clean SHUTDOWN bit;
            appData.transferStatus = APP_TRANSFER_STATUS_IDLE;
            APP_CHECK_ERROR_VOID(appData.transferHandle,
                DRV_I2C_WriteTransferAdd(appData.drvI2CHandle, APP_TC74_SLAVE_ADDR,
                    appData.txData, 2,&appData.transferHandle),
                DRV_I2C_TRANSFER_HANDLE_INVALID, I2cWakeUpErrorJump);
            appData.state = APP_STATE_I2C_WAKEUP_TC74_NEXT;
            I2cWakeUpErrorJump:;
        }
        break;

        case APP_STATE_I2C_WAKEUP_TC74_NEXT:
        {
            if(appData.transferStatus == APP_TRANSFER_STATUS_SUCCESS){
                appData.pauseUs = 3000000;
                appData.state = APP_STATE_PAUSE;
                APP_CONSOLE_PRINT("TC74 Waking Up!: waiting 300ms before retry.");
            } else if (appData.transferStatus == APP_TRANSFER_STATUS_ERROR){
                APP_ERROR_PRINT_AND_JUMP(I2cWakeUpNexErrorJump,
                        "Writing CONFIG register at ADDR=0x%x failed. i2cEvent=%d",
                        APP_TC74_SLAVE_ADDR, appData.i2cEvent);
            } else {
                // other states: transfer in progress, do nothing...
            }
            I2cWakeUpNexErrorJump:;
        }
        break;

        case APP_STATE_I2C_BUSY_TC74:
        {
            // maximum busy time should be 250ms, we will wait 300ms and try again
            appData.pauseUs = 3000000;
            appData.state = APP_STATE_PAUSE;
            APP_CONSOLE_PRINT("TC74 busy: waiting 300ms before retry.");
        }
        break;
        
        case APP_STATE_I2C_QUERY_TEMP:
        {
            // select and query TEMPERATURE register to get current temperature
            appData.txData[0] = APP_TC74_REG_TEMP; // register we want to select
            appData.rxData[0] = 0; // clean read buffer
            appData.transferStatus = APP_TRANSFER_STATUS_IDLE;
            APP_CHECK_ERROR_VOID(appData.transferHandle,
                DRV_I2C_WriteReadTransferAdd(appData.drvI2CHandle, APP_TC74_SLAVE_ADDR,
                    appData.txData, 1, appData.rxData, 1,&appData.transferHandle),
                DRV_I2C_TRANSFER_HANDLE_INVALID, I2cQueryTempErrorJump);
            appData.state = APP_STATE_I2C_QUERY_TEMP_READ;
            I2cQueryTempErrorJump:;
        }
        break;

        case APP_STATE_I2C_QUERY_TEMP_READ:
        {
            // test if CONFIG register was read without error
            if(appData.transferStatus == APP_TRANSFER_STATUS_SUCCESS){
                // temperature is signed !
                int8_t temp = (int8_t)appData.rxData[0];
                appData.iter++;
                APP_CONSOLE_PRINT("#%u Temp=%d Celsius (raw=0x%X)",
                        appData.iter, temp, appData.rxData[0]);
                // Wait and measure again
                appData.pauseUs = 2000000;
                appData.state = APP_STATE_PAUSE;
            } else if (appData.transferStatus == APP_TRANSFER_STATUS_ERROR){
                APP_ERROR_PRINT_AND_JUMP(I2cQueryTempReadErrorJump,
                        "I2C Read TEMP from ADDR=0x%x failed. Is target device TC74? i2cEvent=%d",
                        APP_TC74_SLAVE_ADDR, appData.i2cEvent);
            } else {
                // other states: transfer in progress, do nothing...
            }
            I2cQueryTempReadErrorJump:;
        }
        break;

        case APP_STATE_PAUSE:
        {
            SYS_TIME_RESULT res;
            if (appData.pauseUs < APP_MINIMUM_PAUSE_US){
                APP_ERROR_PRINT_AND_JUMP(PauseErrorJump,
                    "Invalid app.pauseUs=%u must be >= %u - INTERNAL ERROR",
                    appData.pauseUs, APP_MINIMUM_PAUSE_US);
            }
            APP_CHECK_ERROR_NEQ(res,
                SYS_TIME_DelayUS(appData.pauseUs, &appData.pauseTimer),
                SYS_TIME_SUCCESS,I2cQueryTempReadErrorJump);
            appData.state = APP_STATE_PAUSE_NEXT;
            PauseErrorJump:;
        }
        break;
        
        case APP_STATE_PAUSE_NEXT:
        {
            if (SYS_TIME_DelayIsComplete(appData.pauseTimer)){
                appData.state = APP_STATE_I2C_QUERY_CONFIG;
            }
        }
        break;

        case APP_STATE_SERVICE_TASKS:
        {
            // state Stub for prototype code
            static bool messagePrinted = false;
            if (!messagePrinted){
                messagePrinted=true;
                APP_CONSOLE_PRINT("TODO: Implement state: APP_STATE_SERVICE_TASKS.");
            }
        }
        break;

        /* TODO: implement your application state machine.*/

        /* Currently we use this to catch all errors. */
        default:
        {
            static bool errorPrinted=false;
            RA0_LED_Set(); // LED on forever on error
            if (!errorPrinted){
                errorPrinted=true;
                APP_ERROR_PRINT("SYSTEM HALTED due error. appState=%d",appData.state);
            }
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
