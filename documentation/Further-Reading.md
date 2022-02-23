# Further Reading
This section simply provides links to the majority of the documentation utilized
when developing this project.
## MSP432 Documentation
- [MSP432P4 SDK](https://dev.ti.com/tirex/explore/node?node=ADoDWIZc5WTuUbxxkAgevQ__z-lQYNj__LATEST)
    - **Most valuable documentation**. Documents the drivers and real time OS used to develop this firmware. Assessable through TI resource explorer.
    - Examples folder has examples for how to do almost anything with TI RTOS
    - Documents folder has documentation for TI drivers and TI RTOS
        - Also accessible from the `docs` folder of an installed copy of the MSP432 Simplelink SDK
    - **This documentation is most of what is required to extend this firmware.**
- [MSP432 Eval Board User Guide](https://www.ti.com/lit/pdf/slau597)
    - Not very useful for development beyond pinouts for the chip and the debugger
- [MSP432 Technical Reference Manual](https://www.ti.com/lit/pdf/slau356)
    - The *big* one. This has all the registers required for register level programming of the MSP432
- [MSP432P401R Data Sheet](https://www.ti.com/lit/gpn/msp432p401r)
    - Very useful for device pinouts and during custom motherboard development.
    - On the software side, mostly good for a detailed pinout

## Radar Board Documentation
- [mmWave Industial Toolbox](https://dev.ti.com/tirex/explore/node?node=AJoMGA2ID9pCPWEKPi16wg__VLyFKFf__LATEST)
    - Essentially, the radar firmware is a modified version of the 'High Accuracy Level Sensing' demo in the 'Labs' folder
    - This is also accessible via TI resource explorer
- [IWR1443BOOST User Guide](https://www.ti.com/lit/pdf/swru518)
    - Useful for pinouts of the radar board
- [IWR1443BOOST Schematics](https://www.ti.com/lit/zip/sprr254)
    - Useful to visualize where to place solder bridges and remove resistors when modifying the radar board

## SIM7000 Documentation
These documents were all used when building the library to interface with the SIM7000 LTE module.
- [SIM7000 AT Command Manual](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_AT%20Command%20Manual_V1.04.pdf)
- [SIM7000 HTTPS Application Note](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_HTTPS_Application%20Note_V1.00.pdf)
- [Botletics Wiki (SIM7000 Module Designer)](https://github.com/botletics/SIM7000-LTE-Shield)
- [SIM7000 NTP Application Note](https://github.com/botletics/SIM7000-LTE-Shield/blob/master/SIM7000%20Documentation/Technical%20Documents/SIM7000%20Series_NTP_Application%20Note_V1.01.pdf)
- [SIM7000 TCP Application Note](https://github.com/botletics/SIM7000-LTE-Shield/blob/master/SIM7000%20Documentation/Technical%20Documents/SIM7000%20Series_TCPIP_Application%20Note_V1.02.pdf)
- [SIM7000 Technical Documentation (Botletics)](https://github.com/botletics/SIM7000-LTE-Shield/tree/master/SIM7000%20Documentation/Technical%20Documents)