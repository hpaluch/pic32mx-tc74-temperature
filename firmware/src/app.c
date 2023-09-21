/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
  Main Application logic.
  - Timer example follows:
    https://microchip-mplab-harmony.github.io/core/GUID-EE734734-7914-41BF-8AF2-8275506BBED4.html

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
#define LED_BLINK_RATE_MS         500

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
            if (APP_Init_LED_Timer()){
                appData.state = APP_STATE_SERVICE_TASKS;
            } else {
                appData.state = APP_STATE_FATAL_ERROR;
            }
            break;
        }

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
