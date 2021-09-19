# Mains Power Auto Switch
A controller to automatically power a shop vacuum  

I have made this vaccuum cleaner (shop vac) auto-switch. So when I operate the circular saw, or any other sawdust creation tool, the vaccuum is switched on and sucks most of the dust away, it then switches off once the sawing is complete. There are three LED's to indicate things, and a button to select a couple of different settings. 

This has been written in the Visual Studio Code IDE with the PlatformIO extention for an Ardunio Mini R5 (now discontinued), although any compatable 5V tolerant microcontroller will work - the current sense module operates at 5V.

### Mains Wiring:
<img src="https://github.com/kazzle101/MainsPowerAutoSwitch/blob/main/mains%20power%20connections.png?raw=true" width="700" alt="Mains Wiring">

### Microcontroller Connections:
<img src="https://github.com/kazzle101/MainsPowerAutoSwitch/blob/main/microcontroller%20connections.png?raw=true" width="700" alt="Microcontroller connections">
