<h1>
Cappy
<img align="left" width="150" height="150" src="assets/icon.png">
</h1>
</img>

A screen capping tool that lets you immediately explore the pixels of the your monitor screen(s).

<br />

## Features
* Smooth panning/zooming of the captured screen(s).
* Inspect pixel data of the currently hovered pixel, then copy that data to the clipboard.
* Crop out sections and/or save the capture to a PNG.
* A grid mode to better see the nice pixels.
* A flashlight mode!

### Demo
The demo is heavily compressed to save space. When you run the application it will run smoother and be at your native resolution. :)

<img src="assets/demo.gif">

### Build
Cappy is built using CMake. CMake will take care of downloading all necessary dependencies. It may take a while to build because everything is built from scratch and statically linked.
``` bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  # or Debug for debug build
cmake --build build
```

The compiled executable will be stored in `./build/release/bin/cappy` for release, and `./build/debug/bin/cappy` for debug. Since this program is statically linked, there is no external dependencies and it can be stored anywhere on your PC. You can use the install command to move it to an appropriate folder on your OS. 

### Install
``` bash
cd build
cmake --install .
```

### Setting global shortcut
Cappy is best used with a global keyboard shortcut so it can be launched any time you need it.
The way to do this depends on the OS. Find the path to the executable. 

#### Setting shortcut for Ubuntu
1. Open the Activities overview and start typing Settings.
2. Click on Settings.
3. Click Keyboard in the sidebar to open the panel.
4. In the Keyboard Shortcuts section, select View and Customize Shortcuts.
5. Add a keyboard shortcut by clicking the + button towards the bottom of the list.
6. Set the Name to ```Cappy```.
7. Set the Command to the path of the executable.
8. Set the shortcut to whatever you'd like. I like using ```ctrl+left_shift+z```.


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

### Configuration
A configuration file is automatically generated when the program is ran for the first time. The location depends on the platform. On Linux: `$HOME/.config/cappy/cappy.ini`, on Windows: `$HOME\AppData\Roaming\cappy\cappy.ini`, and on Mac: `$HOME/Library/Application Support/cappy/cappy.ini`. All possible configuration is as follows:

| Option                        | Description                                                                | Default        |
| ----------------------------- | -------------------------------------------------------------------------- |--------------- |
| window_fullscreen             | Sets the window to fullscreen, otherwise sets it as fullscreen borderless. | `false`          |
| window_pre_crop               | Pre-crop the image at initial startup. Requires 4 integers in the format: X Y WIDTH HEIGHT. If WIDTH is 0, then it is replaced with the capture width. If HEIGHT is 0, then it is replaced with the capture height.                                                                                   | `0 0 0 0`        |
| flashlight_size               | The initial flashlight radius in pixels.                                                                   | `150`            |
| flashlight_center_inner_color | The center color of the flashlight. Requires 4 integers between 0-255 in the format: REG GREEN BLUE ALPHA. | `255 255 204 25` |
| flashlight_center_outer_color | The center color of the flashlight. Requires 4 integers between 0-255 in the format: REG GREEN BLUE ALPHA. | `255 255 204 25` |
| flashlight_outer_color        | The center color of the flashlight. Requires 4 integers between 0-255 in the format: REG GREEN BLUE ALPHA. | `51 51 0 50`     |
| background_color              | The center color of the flashlight. Requires 3 integers between 0-255 in the format: REG GREEN BLUE.       | `50 50 50`       |
| grid_size                     | The size of the grid in pixels.                                                                            | `100`            |
| grid_color                    | The color of the grid. Requires 3 integers between 0-255 in the format: REG GREEN BLUE.                    | `200 200 200`    |


### Controls

#### Main

| Key          | Description                |
| ------------ | -------------------------- |
| C            | Enter/Exit color mode      |
| F            | Enter/Exit flashlight mode |
| R            | Reset capture              |
| G            | Toggle grid                |
| M            | Minimize window            |
| Right Click  | Enter crop drawing mode    |
| Left Drag    | Pan                        |
| Scroll Wheel | Zoom                       |
| Ctrl+S       | Save capture               |

#### Color Mode

You can hover your mouse over a pixel and a pop-up of the color will appear near the mouse.  

| Key          | Description                                                                       |
| ------------ | --------------------------------------------------------------------------------- |
| Ctrl+D       | Copy color to clipboard as a decimal number                                       |
| Ctrl+Shift+D | Copy color to clipboard as a decimal number, each channel separated by commas     |
| Ctrl+H       | Copy color to clipboard as a hexadecimal number                                   |
| Ctrl+Shift+H | Copy color to clipboard as a hexadecimal number, each channel separated by commas |
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