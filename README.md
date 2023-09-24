# PIC32MX Temperature meter with TC74

Here is my project that uses PIC32MX with TC74 to measure temperature and print
it on UART.  MCC Harmony asynchronous Components (Debug, Console, I2C and other
services) are utilized in this project which is new experience for me.

Status:
- project finished as of Sep 24 2023
- now I will polish documentation and fix bugs.
- however "wake up" code and "Busy wait" code is untested (I never experienced
  that state)

When configured properly there should be UART output like this:
```
app.c:209 Starting app v1.04
app.c:253 OK: I2C ACK response from dev at ADDR=0x48. Data=0x1f
app.c:293 Data from TC74 at ADDR=0x48: CONFIG=0x40 UP READY zero mask: 0x0
app.c:386 #1 Temp=31 Celsius (raw=0x1F)
app.c:386 #2 Temp=32 Celsius (raw=0x20)
app.c:386 #3 Temp=33 Celsius (raw=0x21)
app.c:386 #4 Temp=34 Celsius (raw=0x22)
app.c:386 #5 Temp=34 Celsius (raw=0x22)
```
(I was holding finger on TC74 to quickly change temperature)


> NOTE!
>
> This project is hit by Hardware bug in PIC32MX Revision
> A1 - see [PIC32MX Errata][PIC32MX Errata].
> The RA0 (LED) and RA1 GPIO pins stop working when I2C1 module
> is enabled (!).
>
> Fortunately recommended workaround is working well, summarized:
> ```diff
> diff --git a/firmware/src/config/default/peripheral/i2c/master/plib_i2c1_master.c b/firmware/src/config
> /default/peripheral/i2c/master/plib_i2c1_master.c
> index 89c42e7..e7f04cc 100644
> --- a/firmware/src/config/default/peripheral/i2c/master/plib_i2c1_master.c
> +++ b/firmware/src/config/default/peripheral/i2c/master/plib_i2c1_master.c
> @@ -74,7 +74,7 @@ void I2C1_Initialize(void)
>      I2C1BRG = 235;
> 
>      I2C1CONCLR = _I2C1CON_SIDL_MASK;
> -    I2C1CONCLR = _I2C1CON_DISSLW_MASK;
> +    I2C1CONSET = _I2C1CON_DISSLW_MASK;
>      I2C1CONCLR = _I2C1CON_SMEN_MASK;
> 
>      /* Clear master interrupt flag */
> ```
>
> See [Commit bdaf154](https://github.com/hpaluch/pic32mx-tc74-temperature/commit/bdaf15443f2f8d2f4590a2bca19931275af74e68) on GitHub for details.

# Program details

This project uses MCC Harmony "Core" Components in Asynchronous mode
(main polling thread - "cooperative multitasking") - e.g. RTOS = No.

* Using FRC = 8Mhz, with FRCPLL scaled to PBCLK = 48 MHz
* MCC Harmony components:
  * [Harmony Core Library][Harmony Core Library] with state machine
  * [System Timer Service][System Timer Service]
    using tickless Core Timer (Core Timer
    is MIPS CPU feature). Tickless means that there is no fixed
    interrupt rate as time base, but `SYS_TIME` schedules
    one shot Timer as needed by consuming clients.
  * [Console System Service][Console System Service] - provides
    Console output via UART2 PLIB
  * [Debug System Service][Debug System Service] - provides
    formatted debug messages API (like printf), requires Console
  * [I2C Driver][I2C Driver] - wrapper around async. I2C PLIB
* Peripherals CORE Timer
* `RA0_LED` is blinking at 1s rate (500ms interrupt rate) using
  [System Timer Service][System Timer Service]
* UART2 PLIB
* I2C1 PLIB

# Required Hardware

* [Microstick II][PIC Microstick II] board
* [PIC32MX250F128B SPDIP][PIC32MX250F128B] - included with my
  Microstick board, but not guaranteed to be included with yours.
* [TC74 I2C Temperature sensor][TC74] recommended 
  "5-lead TO-220" package.
* [USB Console Cable #954][cable954] - or any other usable USB to UART adapter.
  WARNING! To ensure 3.3V compatibility with PIC, connect only Input (White)
  and Ground (Black). Keep Output (Green) not connected to avoid 5V on 3.3V pin
  (this USB Version of PIC32MX have pins 21,22 NOT 5V tolerant!)
* 2 pull-up resistors around 2k2 kOhm for SCA and SDL signals on I2C. Connected
  to VDD (3.3V test point on board).

Microstick II Configurtion:
- closed LED Jumper J3 (so LED is connected to PIN2 RA0
- programming switch S1 in `A` position (where PINS 4 & 5 used for programming
  and debugging)

Required Wiring of Microstick II:

| Microstick II Pin | Signal | Target | Detail |
| ---: | --- | --- | --- |
| 17 | SCL1 | 2K pull-up | I2C1 clock |
| 18 | SDA1 | 2K pull-up | I2C1 data |
| 21 | U2TX | Console #954, White | UART2 TX (PIC Output, PC Input) |
| 22 | U2RX | NC | UART2 RX (PIC Input, PC Output) - not connected |
| 27 | GND | Console #954, Black | - |

Required wiring for TC74 in 5-lead TO-220:

| TC74 pin | Signal | Microstick II pin |
| ---: | --- | ---: |
| 2 | SDA | I2C data |  18 |
| 3 | GND | Ground | 27 |
| 4 | SCLK | I2C Clock | 17 |
| 5 | VDD | +3.3V | "VDD" Test pin |

# Software requirements

* [XC32 compiler][XC compilers] - tested version v4.30
* [MPLAB X IDE][MPLAB X IDE] - tested version v6.15

WARNING! Before opening this project you should inspect Manifest file
[firmware/src/config/default/harmony-manifest-success.yml](firmware/src/config/default/harmony-manifest-success.yml)
and ensure that your Harmony repository contains specified components and
exactly same versions.

- currently the only reliable way is to do this (I'm not kidding!):
- create Empty MCC Harmony project and ensure that you specified
  proper CPU [PIC32MX250F128B SPDIP][PIC32MX250F128B] 
- when Wizard finishes it will automatically invoke MCC Content
  Manager
- you have to select `MCC Harmony` (the only available choice)
- next you have to select exactly same components and
  versions as listed in Manifest [firmware/src/config/default/harmony-manifest-success.yml](firmware/src/config/default/harmony-manifest-success.yml) 
- once MCC finished Downloading and starts you can:
- Close MCC
- Close that empty project
- Open this project - MCC should work without any complaint

Before running this program you need to define proper I2C address
of your TC74 sensor in app.c. You have to read your package name:
- in my case it is `TC74A0`
- where A0 means that I2C address is `0x48`
- it is also default address in `firmware/src/app.c`:

```c
#define APP_TC74_SLAVE_ADDR_A0 0x48
#define APP_TC74_SLAVE_ADDR_A5 0x4D
#define APP_TC74_SLAVE_ADDR APP_TC74_SLAVE_ADDR_A0
```

So if your TC74 has suffix different from `A0` you need to look
into [TC74 datasheet][TC74] and `#define` proper I2C address.
Otherwise there will be error message on UART like this:

```
app.c:209 Starting app v1.03
ERROR: app.c:258 I2C Read from ADDR=0x48 failed. Is TC74 connected? i2cEvent=-1
ERROR: app.c:407 SYSTEM HALTED due error. appState=9999
```

## Software requirements

* [XC32 compiler][XC compilers] - tested version v4.30
* [MPLAB X IDE][MPLAB X IDE] - tested version v6.15


# Resources

This code is based on several Internet resources including:
- Timer example:
  - https://microchip-mplab-harmony.github.io/core/GUID-EE734734-7914-41BF-8AF2-8275506BBED4.html
- I2C Driver based on example:
  - https://microchip-mplab-harmony.github.io/core/GUID-8916AA7D-64C7-4477-8D26-664F6B3C242A.html
  - https://github.com/Microchip-MPLAB-Harmony/core_apps_pic32mx/tree/master/apps/driver/i2c/async/i2c_eeprom


PIC32MX is `MIPS32 M4K` based CPU Core with peripherals from Microchip.
Please see links below for more information:
- datasheet: [PIC32MX250F128B][PIC32MX250F128B]
- official splash page: https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/32-bit-mcus/pic32-32-bit-mcus/pic32mx
- [MIPS32 M4K Manual][MIPS32 M4K Manual] from mips.com
- [MIPS32 M4K Core Datasheet][MIPS32 M4K DTS] from mips.com
- [MIPS32 Instruction Set][MIPS32 BIS] from mips.com
- [MIPS32 Instruction Set Quick Reference][MIPS32 QRC] from mips.com
- [PIC32MX Interrutp handling][PIC32MX S11 INT]

[PIC32MX Errata]: https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/Errata/PIC32MX1XX-2XX-28-36-44-pin-Family-Errata-DS80000531Q.pdf
[I2C Driver]: https://microchip-mplab-harmony.github.io/core/GUID-4321CAFA-57B5-4633-9D43-0AE24B87C101.html
[Debug System Service]: https://microchip-mplab-harmony.github.io/core/GUID-4F625306-2206-49B1-8846-60C97E40A440.html
[Console System Service]: https://microchip-mplab-harmony.github.io/core/GUID-C8EFF72A-1BBB-416E-BF89-EEA2B23EB27D.html
[I2C Driver]: https://microchip-mplab-harmony.github.io/core/GUID-A420B807-5F28-4CED-9759-6E0F87209108.html
[Console System Service]: https://microchip-mplab-harmony.github.io/core/GUID-177E8C6B-6F6F-4E94-9096-38134597D79A.html
[Harmony Core Library]: https://microchip-mplab-harmony.github.io/core/
[System Timer Service]: https://microchip-mplab-harmony.github.io/core/GUID-9D474B7C-D749-4DD6-A012-FE94C039324E.html
[TC74]: https://www.microchip.com/en-us/product/tc74
[PIC32MX S11 INT]: http://ww1.microchip.com/downloads/en/DeviceDoc/61108B.pdf
[MIPS32 M4K Manual]: https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00249-2B-M4K-SUM-02.03.pdf
[MIPS32 M4K DTS]: https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00247-2B-M4K-DTS-02.01.pdf
[MIPS32 BIS]: https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00086-2B-MIPS32BIS-AFP-05.04.pdf
[MIPS32 QRC]: https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00565-2B-MIPS32-QRC-01.01.pdf 
[Harmony]: https://www.microchip.com/mplab/mplab-harmony
[XC compilers]: https://www.microchip.com/mplab/compilers
[MPLAB X IDE]: https://www.microchip.com/mplab/mplab-x-ide
[PIC32MX250F128B]: https://www.microchip.com/wwwproducts/en/PIC32MX250F128B
[PIC Microstick II]: https://www.microchip.com/DevelopmentTools/ProductDetails/dm330013-2
[cable954]: https://www.modmypi.com/raspberry-pi/communication-1068/serial-1075/usb-to-ttl-serial-cable-debug--console-cable-for-raspberry-pi
