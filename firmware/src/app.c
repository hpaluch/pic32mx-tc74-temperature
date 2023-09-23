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
#define APP_VERSION 102 // 123 = 1.23
#define LED_BLINK_RATE_MS         500
// TC74 I2C Address - WARNING! You have to read it from package and
// use proper address. My is TC74A0
// Where A0 according to datasheet is 0x48
// I2C defines 0b 1001 101 => 0100 1101
#define APP_TC74_SLAVE_ADDR_A0 0x48
#define APP_TC74_SLAVE_ADDR_A5 0x4D
#define APP_TC74_SLAVE_ADDR APP_TC74_SLAVE_ADDR_A0
// short version of __FILE__ without path
static const char *APP_FILE = "app.c";
// improved macro that will print file and line of message
#define APP_CONSOLE_PRINT(fmt,...) SYS_CONSOLE_PRINT("%s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)
// simple form  without any added content
#define APP_CONSOLE_PRINT_RAW(fmt,...) SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#define APP_ERROR_PRINT(fmt,...) SYS_DEBUG_PRINT(SYS_ERROR_ERROR, "ERROR: %s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)

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
    appData.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    appData.transferStatus = APP_TRANSFER_STATUS_ERROR;
    appData.rxData[0] = 0;
    appData.txData[0] = 0;
    appData.txData[1] = 0;
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
            APP_CONSOLE_PRINT("Before DRV_I2C_Open()");
            
            APP_CHECK_ERROR(appData.drvI2CHandle,
                    DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE),
                    DRV_HANDLE_INVALID, InitI2cErrorJump);
            APP_CONSOLE_PRINT("OK: After DRV_I2C_Open() handle=0x%" PRIxPTR,
                    appData.drvI2CHandle);
            
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
                APP_CONSOLE_PRINT("OK: TC74 response from ADDR=0x%x. Data=0x%x",
                        APP_TC74_SLAVE_ADDR, appData.rxData[0]);
                appData.state = APP_STATE_SERVICE_TASKS;
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
            
        case APP_STATE_SERVICE_TASKS:
        {
            static bool messagePrinted = false;
            if (!messagePrinted){
                messagePrinted=true;
                APP_CONSOLE_PRINT("TODO: Implement state: APP_STATE_SERVICE_TASKS.");
            }
            break;
        }

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
