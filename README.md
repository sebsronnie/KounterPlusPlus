# Kounter++

Coding Camp II, Project 3. Team Foxtrot: Ronald Sebuhinja and Kelvin Maritim.

Counts objects in a still picture or a live video, measures them, tells different kinds apart, and lets you tune how strict the detection is. Written in C++ on openFrameworks 0.12.1 with the OpenCV that ships inside the `ofxOpenCv` addon. Nothing needs to be installed beyond openFrameworks itself.

## Build on macOS (Xcode)

1. Install Xcode from the App Store and open it once so it installs its command-line tools.
2. Download `of_v0.12.1_osx_release.zip` from openframeworks.cc/download and unzip it, for example to `~/openFrameworks`.
3. Copy this folder to `~/openFrameworks/apps/myApps/KounterPlusPlus` (the path must be two levels below `apps`, as for every openFrameworks project).
4. Open `~/openFrameworks/projectGenerator/projectGenerator.app`. Click **Import**, choose the `KounterPlusPlus` folder, make sure `ofxOpenCv` and `ofxGui` are ticked in the addons list (they are read from `addons.make`), and click **Update**. Then click **Open in IDE**.
5. In Xcode, select the `KounterPlusPlus Debug` or `Release` scheme and press Run. The first build compiles openFrameworks itself and takes a few minutes; later builds take seconds.

The application starts with a sample image already counted. Press **H** in the window for the key map.

If the camera does not start, check System Settings > Privacy & Security > Camera and allow the application; the permission dialog appears the first time you press **C**.

## Build on Windows (Visual Studio 2022 or newer)

1. Download `of_v0.12.1_vs_64_release.zip`, unzip, copy this folder to `apps\myApps\KounterPlusPlus`.
2. Run `projectGenerator\projectGenerator.exe`, Import the folder, Update, Open in IDE.
3. If Visual Studio offers to retarget the solution to a newer toolset, accept. Build and run the x64 Debug or Release configuration.

## Using it

| Action | Panel | Key |
|---|---|---|
| Open an image | Load image | O, or drag a file onto the window |
| Use the webcam | Use camera | C |
| Open a video | Load video | V, or drag a file onto the window |
| Segmentation mode | Mode slider | keys 1-5 select modes 0-4: Otsu, manual, adaptive, colour pick, background removal |
| Pick a colour (mode 4) | | left click on the picture |
| Force grayscale | toggle | G |
| Invert (objects brighter than background) | toggle | I |
| Split touching objects | toggle | X |
| Teach a kind of object | | T, click an object, type a name, Enter |
| View original / mask / overlay | | Tab |
| Reset the distinct-object counter | Reset video counter | R |
| Save screenshot to `bin/data` | Save screenshot | S |
| Export measurements as CSV to `bin/data` | Export CSV | E |
| Pause a video | | Space |
| Help | | H |

Sample images and a sample video are in `bin/data/images` and `bin/data/videos`.

## Layout

```
src/vision    the OpenCV pipeline (no openFrameworks dependency)
src/sources   image, camera and video sources
src/app       control panel, overlay drawing, oF <-> OpenCV bridge
```
