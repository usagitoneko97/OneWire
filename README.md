# 1-wire device communication for stm32l0xx mcu
 1-wire protocol is used to communicate with certain device that support 1-wire
 such as *ds1820b temperature sensor*. Interrupt based event-driven state machine is used to perform the task.
 
 ## Requirements
 OneWire requires a single 4.7K pullup resistor, connected between the pin and +5 volts. Then just connect each 1-wire device to the pin and ground. Please refer to the specifications for the 1-wire devices you are using. If you wish to seperate the tx rx line (implement using full duplex uart) of 1 wire of the mcu, you can connect a buffer in between. More information can be found [here](https://www.maximintegrated.com/en/app-notes/index.mvp/id/214)
 
 ![1wirePullUptxRx](https://github.com/usagitoneko97/OneWire/blob/master/image/1wirePullup.gif)
 
 
_1-Wire bus interface circuitry_

Otherwise, if uart is configured to txrx mode, 1 wire can be connect simply as below: 
![1wireMasterSlave](https://github.com/usagitoneko97/OneWire/blob/master/image/1_wire_parasite.png)

## Example usage
this library provide very easy to use api to help communicate with 1 wire device. for example of performing **rom searching** to retrieve unique id of 1 wire device on multiple slave (command code 0xf0):

**initiate rom search command by:** 
```C 
owRomSearchInit(); 
``` 
**get the result of rom searching by either :**
 
* polling 
```C 
uint8_t *romUID = getResultRomUid(); 
``` 
* non-blocking 
```C 
if(owSearchRomGetResult(ptrToRomUid)){ 
    //romUid is ready here 
} 
``` 
