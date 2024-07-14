STL Blueprint View

I wanted a blueprint style view for my ships in From The Depths. Bring your ship out of the game or other STL into a neat Graphic.
![](./photosForREADME/simpleShipBluePrint.png?raw=true )
You will be greeted by a simple ship. Press O to open an explorer prompt and find your own STL.
![](./photosForREADME/openDialog.png?raw=true )
Fit the application to your screen or drag it to any size you want.
Use left click and right click to section off a rectangular area of your object.
![](./photosForREADME/pointSelection.png?raw=true )
You may hide the colored markers or show them using space bar.
Press enter to rerender your object zoomed into the area of interest. Another Enter keypress returns you to the zoomed our view from which you can zoom in again.
![](./photosForREADME/smallMissiles.png?raw=true )

This application is built on top of https://github.com/meemknight/cmakeSetup , a repo meant to simplify 2D game creation.
You could extract the class which handles the STL in STLParser.h and read function in gameLayer.cpp should that be what your own project needs.
I made an excersise out of sticking to the style of this repo and keeping my work to gameLayer.cpp.
Feel free to make whatever you wish from this repo. Send me the blueprint view of your Flag Ships :)
