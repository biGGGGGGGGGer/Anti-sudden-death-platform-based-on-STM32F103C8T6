# Anti-sudden-death-platform based on STM32F103C8T6
## modules included:
- STM32F103C8T6

- MAX30102

- SIM800C

- 4-pin OLED

- NEO-6M


## HOW IT WORKS?
The STM32F103C8T6 gets the data of heartbeat rate from MAX30102,and the location data from NEO-6M.The OLEDwill show the heartbeat rate in real time. If the data of heartbeat rate is serious,it will end messages using SIM800C to the persion pointed.
