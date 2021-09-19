# Mains Power Auto Switch
A controller to automatically power a shop vacuum. 

I have made this vacuum cleaner (shop vac) auto-switch. So when I operate the circular saw, or any other sawdust creation tool, the vacuum is switched on and sucks most of the dust away, it then switches off once the sawing is complete. There are three LEDs to indicate things, and a button to select a couple of different settings. The current sensor is being used to see that current is being drawn, rather than accurately measuring the amount flowing.

This has been written in the Visual Studio Code IDE with the PlatformIO extension for an Arduino Mini R5 (now discontinued), although any compatible 5V tolerant microcontroller will work - the current sense module operates at 5V.


### Mains Wiring:
<img src="https://github.com/kazzle101/MainsPowerAutoSwitch/blob/main/mains%20power%20connections.png?raw=true" width="700" alt="Mains Wiring">

### Microcontroller Connections:
<img src="https://github.com/kazzle101/MainsPowerAutoSwitch/blob/main/microcontroller%20connections.png?raw=true" width="700" alt="Microcontroller connections">
Note that I have used the Sparkfun ACS723 (5A) Current Sense module rather than the ACS712 shown in the diagram.

### YouTube video
Describing the operation of the device
https://www.youtube.com/watch?v=bf6Q4yfRd6U
