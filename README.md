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


#### Setting shortcut for Windows 10/11 (Way 1)
1. Right-click the blank area on the desktop and select New -> Shortcut. Type the location of the item and click Next. If you don’t know the path of the app, you can click the Browse button. Then you can select the target app’s executable file and click OK. Type a name for the shortcut and click Finish to create a shortcut for the app.
2. Then you can right-click the app’s shortcut and select Properties.
3. In the Properties window, you can click the Shortcut tab.
4. Next to the Shortcut key field, you can set a keyboard shortcut that you want to use to open the app. Click OK to save the setting.

#### Setting shortcut for Windows 10/11 (Way 2)
1. You can click Start Menu on your Windows 10 computer. For Windows 11, you need to click the All apps icon to view all apps in the Start Menu.
2. Next, you can scroll down to find the target app. Right-click the app and select Pin to taskbar or More -> Pin to taskbar to add the program to the Windows taskbar. Alternatively, you can also directly drag the app onto the taskbar to pin the program to the taskbar. You can follow the same operation to pin other apps to the taskbar.
3. Then you can use the Windows key along with the corresponding number key to open the programs on the taskbar with a keyboard shortcut. Based on the position of the pinned apps on the taskbar, they get a number from 1 to 9 from the left to the right. For instance, you can press Windows + 1 keyboard shortcut to open the first pinned app on the taskbar.

[Source](https://www.minitool.com/news/open-a-program-with-keyboard-shortcut-win-10-11.html)

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
- [x] Add smooth zoom for flashlight
- [x] When resizing and zooming in and not panning, the cropped section is changing width and height when it should stay the same.
- [ ] Add load dialog box
- [ ] Add command line argument for inputted file, window width/height/scale, borderless/fullscreen, window position,
      flashlight color, zoom factor, zoom speed
- [ ] Add show grid
- [ ] When the image is cropped and zoomed in on color mode. A square is rendered when you are one off the width or height
- [ ] If in color mode and zoomed in enough to render a square. If the mouse is outside the capture, the cursor should be shown.