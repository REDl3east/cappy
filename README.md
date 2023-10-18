# cappy

A screen capping tool that lets you immediately explore the pixels of the current screen. You can pan/zoom around and extract the colors of each pixel.

### Controls

#### Main

**C**            - Enter/Exit color mode  
**F**            - Enter/Exit flashlight mode  
**R**            - Reset capture and camera
**Right Click**  - Enter crop drawing mode
**Left Drag**    - Pan  
**Scroll Wheel** - Zoom  

#### Color Mode

You can hover your mouse over a pixel and a pop-up of the color will appear near the mouse.  

**Ctrl+D**       - Copy color to clipboard as a decimal number  
**Ctrl+Shift+D** - Copy color to clipboard as a decimal number, each channel separated by commas  
**Ctrl+H**       - Copy color to clipboard as a hexidecimal number  
**Ctrl+Shift+H** - Copy color to clipboard as a hexidecimal number, each channel separated by commas  
**Ctrl+B**       - Copy color to clipboard as a binary number  
**Ctrl+Shift+B** - Copy color to clipboard as a binary number, each channel separated by commas  

#### Flashlight Mode

You can focus in on a specific point in flashlight mode

**LShift+Scroll Wheel**    - Increase/Decrease flashlight size   

#### Crop Drawing Mode

Select a region of the capture.

**Release Right Click** - Finish drawing crop region  
**Drag**                - Continue drawing region  
**Right Click (Rapid)** - Exit Crop Drawing Mode  
**X**                   - Crop capture

## TODO
- [x] Show color data, with text, near color panel
- [x] Add ability to save screenshot
- [x] Add screen selection
- [x] Async save dialog 
- [x] Add ability to update selection
- [x] Add command line load image 
- [ ] Add load dialog  box
- [ ] Add command line argument for inputted file, window width/height/scale, borderless/fullscreen, window position
- [ ] Show text that shows the size of the selection
- [ ] Add mouse position text
- [ ] Add smooth zoom for camera and flashlight