# 1 wire algorithm implementation using UART
 tail-end recursion method is used to perform the task
## Usage
initiate rom search command by:
```C
owRomSearchInit();
```
get the result of rom searching by either :

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
