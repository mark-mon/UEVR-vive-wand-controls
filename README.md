This plugin is a combination of a .json interaction profile and dll plugin for UEVR to fix the broken
vive wand controls in openxr. By default, you have no way to move as the left and right sticks aren't
attached, the dpad and L3/R3 buttons are inaccessible. 

This project fixes the controls in the following ways:
- See the controller mapping photo for control layout.
- Turns left trackpad touch actions into left analog stick
- Turns left trackpade push actions into dpad.
- Turns center of left trackpad into L3
- Turns right trackpad touch into right anlog stick. 
- Reserves edges of the right touchpad for button presses so X, Y, A, B presses don't get right stick events.
- Turns the edges of the right touchpad click into the 4 face buttons A, B, X, Y
- Turns the left menu button (above trackpad) into a shift button for shift operations.
- Turns the right menu button into select.
- Turns the right menu button shifted into start.
- Adds ability to send F2, F3, F4, and Insert via shift+ A, B, X, Y
- Adds ability to swap LT with right grip for common shooter games where LT is aim. Now you can aim with squeezing
  right grip instead. To use, use Shift + LT, the controls will vibrate letting you know LT and RB have swapped.
  To revert, shift + LT again.
- Adds a haptic on right trigger for shooting games. To enable, shift + RT, right controller vibrates acknowledging.

This covers the full xbox controller layout allowing for all 4 face buttons to be accessed on the right hand.

This is a must for anyone that has a Varjo Aero and index controllers
as this is not a native steamvr device and performs much better on openxr. There may be other uses
for this.

This installs in a global locatio for UEVR so it's used in all games with one install.
Requires nightly build of UEVR 839 or newer.
Go to the global directory of UEVR (%appdata%\UnrealVRMod).Create a folder there called UEVR if it does not exist.
Extract this zip into that folder so that you have UEVR\Plugins and UEVR\Profiles. 
This will load into all games so it's a UEVR-wide mod in this configuration.

Delete any existing older versions of the mod in the individual game folders.


