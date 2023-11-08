# cappy

A screen capping tool that lets you immediately explore the pixels of the current screen. You can pan/zoom around and extract the colors of each pixel.

### Build
``` bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. # or Debug for debug build
make -j 4
```

### Install
``` bash
make install
```

### Setting global shortcut
Cappy is best used with a global keyboard shortcut so it can be launched any time you need it.
The way to do this depends on the OS.

#### Setting shortcut for Ubuntu
1. Open the Activities overview and start typing Settings.
2. Click on Settings.
3. Click Keyboard in the sidebar to open the panel.
4. In the Keyboard Shortcuts section, select View and Customize Shortcuts.
5. Add a keyboard shortcut by clicking the + button towards the bottom of the list
6. Set the Name tp ```Cappy```
7. Set the Command to ```cappy```
8. Set the shortcut to whatever you'd like. I like using ```ctrl+lshift+z```

#### Setting shortcut for Windows
1. TBD

### Controls

#### Main

| Key          | Description                |
| ------------ | -------------------------- |
| C            | Enter/Exit color mode      |
| F            | Enter/Exit flashlight mode |
| R            | Reset capture              |
| Right Click  | Enter crop drawing mode    |
| Left Drag    | Pan                        |
| Scroll Wheel | Zoom                       |



#### Color Mode

You can hover your mouse over a pixel and a pop-up of the color will appear near the mouse.  

| Key          | Description                                                                       |
| ------------ | --------------------------------------------------------------------------------- |
| Ctrl+D       | Copy color to clipboard as a decimal number                                       |
| Ctrl+Shift+D | Copy color to clipboard as a decimal number, each channel separated by commas     |
| Ctrl+H       | Copy color to clipboard as a hexidecimal number                                   |
| Ctrl+Shift+H | Copy color to clipboard as a hexidecimal number, each channel separated by commas |
| Ctrl+B       | Copy color to clipboard as a binary number                                        |
| Ctrl+Shift+B | Copy color to clipboard as a binary number, each channel separated by commas      |

#### Flashlight Mode

You can focus in on a specific point in flashlight mode.

| Key                 | Description                       |
| ------------------- | --------------------------------- |
| LShift+Scroll Wheel | Increase/Decrease flashlight size |

#### Crop Drawing Mode

Select a region of the capture.

| Key                 | Description                         |
| ------------------- | ----------------------------------- |
| Release Right Click | Finish drawing crop region          |
| Drag                | Continue drawing region             |
| Shift+Drag          | Continue drawing region as a square |
| Right Click (Rapid) | Exit Crop Drawing Mode              |
| X                   | Crop capture                        |

## TODO
- [x] Show color data, with text, near color panel
- [x] Add ability to save screenshot
- [x] Add screen selection
- [x] Async save dialog 
- [x] Add ability to update selection
- [x] Add command line load image 
- [x] Add smooth zoom for camera
- [x] Show text that shows the size of the selection
- [x] Add smooth zoom for flashlight
- [x] Add mouse position text
- [ ] Add load dialog box
- [ ] Add command line argument for inputted file, window width/height/scale, borderless/fullscreen, window position,
      flashlight color, zoom factor, zoom speed
- [ ] Add smooth zoom for flashlight
- [ ] Add show grid