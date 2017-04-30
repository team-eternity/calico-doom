# Calico
Calico is a port of the Atari Jaguar DOOM codebase back to the PC, aiming to
support playing the game in its original form on that console with a minimum 
number of required changes. It accepts either the game's ROM file or the 
extracted jagdoom.wad file from inside it. You will need a backup device for
Jaguar ROM cartridges in order to legally play the game.

I have undertaken reverse engineering of the GAS assembly files for the 
renderer, which are written in a custom assembly language understood only by
the custom Tom and Jerry ASICs on the Jaguar motherboard. Gameplay GAS files
were additionally verified by cross-referencing against the 3DO source code
and the Doom64 EX project, both of which share about 99% of that code but 
already in a C form.

Calico is named in honor of my late cat, Nikki Fritz, who was a calico tabby
mix, because it is a "little kitty" of a Doom port.

## How to Use
After unpacking the binary build, either add a `doom.jag` Jaguar ROM file or 
`jagdoom.wad` to the same directory and simply double-click the executable file
to start. You can also identify the file to use by using the `-iwad` 
command-line parameter, which works the same as for most other source ports.
Note that any Jaguar ROM format should be supported, but only files named 
`doom.jag` will currently be autodetected. The format of the file will be
detected at runtime and is not dependent on the name of the file.

## Default Controls
Currently only keyboard input is supported; mouse and gamepad support will be 
added in future versions. The default controls, which can be rebound via the 
configuration file, are as follows, when using control scheme "A":
* 1 - 9: Select weapon
* 0: Toggle automap
* Right Ctrl: Fire weapon
* Right Alt: Press to use; hold to strafe
* Right Shift: Hold to run
* Arrow keys: Move the player
* Escape: Toggle menus
* Pause: Pause the game
* Jaguar # Keypad button: Keypad /
* Jaguar * Keypad button: Keypad *
Control schemes A through C are available as always through the game's menu.

## Command-line Parameters
* `-devparv` : Enable the original Jaguar game's debug mode
* `-nosfx` : Disable digital sound effects

## Configuration File Options
Eventually a configuration program will be provided, but for now the following 
options are available through the configuration file only:
* aspectnum : The numerator of the fraction for the desired aspect ratio.
* aspectdenom : The denominator of the fraction for the desired aspect ratio.
* fullscreen : Set to "1" for true fullscreen, or "-1" for desktop-sized window.
* linear_filtering : Set to "1" to linear filter the upscaled video output.
* monitornum : Set to a value > 0 to move the program to a different monitor.
* screenwidth : Set the width of the display resolution.
* screenheight : Set the height of the display resolution.

## License
All original code, as well as code derived from the 3DO source code, is
available under the MIT license. Code taken from Jaguar DOOM is still under the
original license for that release, which is unfortunately not compatible with
free software licenses (if a source file does not have a license header stating
otherwise, then it is covered by the Jaguar Doom source license).

## Known Issues
This program is currently in beta so some things are not finished or have known
issues:
* There is no support for music yet.
* Demos may desync; reason is currently unknown.
