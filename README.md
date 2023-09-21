# PIC32MX Temperature meter with TC74

Here is my project that uses PIC32MX with TC74 to measure
temperature and print it on UART. I plan to fully utilize
`MCC Harmony` libraries and framework to learn how to use them in
practice.

Status: Just started

# Required Hardware

* [Microstick II][PIC Microstick II] board
* [PIC32MX250F128B SPDIP][PIC32MX250F128B] - included with my
  Microstick board, but not guaranteed to be included with yours.
* [TC74 I2C Temperature sensor][TC74] recommended 
  "5-lead TO-220" package.
* [USB Console Cable #954][cable954] - or any other usable USB to UART adapter.
  Recommended 3.3V TTL version (PIC32MX has UART pins 5V tolerant
  but there are some limitations).
* 2 pull-up resistors 2 kOhm for SCA and SDL signals on I2C. Connected
  to VDD (3.3V on board).

# Software requirements

* [XC32 compiler][XC compilers] - tested version v4.30
* [MPLAB X IDE][MPLAB X IDE] - tested version v6.15

## Software requirements

* [XC32 compiler][XC compilers] - tested version v4.30
* [MPLAB X IDE][MPLAB X IDE] - tested version v6.15


# Resources

PIC32MX is `MIPS32 M4K` based CPU Core with peripherals from Microchip.
Please see links below for more information:
- datasheet: [PIC32MX250F128B][PIC32MX250F128B]
- official splash page: https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/32-bit-mcus/pic32-32-bit-mcus/pic32mx
- [MIPS32 M4K Manual][MIPS32 M4K Manual] from mips.com
- [MIPS32 M4K Core Datasheet][MIPS32 M4K DTS] from mips.com
- [MIPS32 Instruction Set][MIPS32 BIS] from mips.com
- [MIPS32 Instruction Set Quick Reference][MIPS32 QRC] from mips.com
- [PIC32MX Interrutp handling][PIC32MX S11 INT]

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
