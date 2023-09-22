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
#define APP_VERSION 101 // 123 = 1.23
#define LED_BLINK_RATE_MS         500
// short version of __FILE__ without path
static const char *APP_FILE = "app.c";
// improved macro that will print file and line of message
#define APP_CONSOLE_PRINT(fmt,...) SYS_CONSOLE_PRINT("%s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)
// simple form  without any added content
#define APP_CONSOLE_PRINT_RAW(fmt,...) SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#define APP_ERROR_PRINT(fmt,...) SYS_DEBUG_PRINT(SYS_ERROR_ERROR, "ERROR: %s:%d " fmt "\r\n", APP_FILE, __LINE__, ##__VA_ARGS__)
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

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

// returns true on success
bool APP_Init_LED_Timer(void){    
    appData.ledTimerHandle = SYS_TIME_CallbackRegisterMS(Timer1_Callback,
            0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);
    return appData.ledTimerHandle != SYS_TIME_HANDLE_INVALID;    
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

void APP_Initialize ( void )
{
    // NOTE: Only data should be initialized here, because it is called
    // very early from initialization.c !!!
    appData.state = APP_STATE_INIT;
    appData.ledTimerHandle = SYS_TIME_HANDLE_INVALID;
    appData.drvI2CHandle = DRV_HANDLE_INVALID;
    appData.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    appData.transferStatus = APP_TRANSFER_STATUS_ERROR;
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
            if (APP_Init_LED_Timer()){
                appData.state = APP_STATE_INIT_I2C;
            } else {
                appData.state = APP_STATE_FATAL_ERROR;
                APP_ERROR_PRINT("APP_Init_LED_Timer() failed.");
            }
            break;
        }

        case APP_STATE_INIT_I2C:
        {
            APP_CONSOLE_PRINT("Before DRV_I2C_Open()");
            appData.drvI2CHandle = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);
            APP_CONSOLE_PRINT("After DRV_I2C_Open() handle=0x%" PRIxPTR,appData.drvI2CHandle);
            if (appData.drvI2CHandle == DRV_HANDLE_INVALID){
                APP_ERROR_PRINT("DRV_I2C_Open() failed.");
                appData.state = APP_STATE_FATAL_ERROR;
                goto exit_init_i2c;
            }
            appData.state = APP_STATE_SERVICE_TASKS;
           exit_init_i2c:;
        }
            break;
        
        case APP_STATE_SERVICE_TASKS:
        {
            // TODO: I2C init ....
            break;
        }

        /* TODO: implement your application state machine.*/


        /* Currently we use this to catch all errors. */
        default:
        {
            RA0_LED_Set(); // LED on forever on error
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
