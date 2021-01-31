# Calico
Calico is a port of the [Atari Jaguar DOOM](https://doomwiki.org/wiki/Atari_Jaguar) 
codebase back to the PC, aiming to support playing the game in its original 
form on that console with a minimum number of required changes. It accepts 
either the game's ROM file or the extracted jagdoom.wad file from inside it.
You will need a backup device for Jaguar ROM cartridges in order to legally 
play the game.

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
to start. 

You can also identify the file to use by using the `-iwad` command-line 
parameter, which works the same as for most other source ports. By using this, 
you can have the file be located wherever you wish and it can be named whatever
you want.

Note that any uncompressed Jaguar ROM format should be supported, but only files
named `doom.jag` will currently be autodetected. The format of the file will be
detected at runtime and is not dependent on the name of the file.

The configuration file and emulated EEPROM storage are written to a writeable area
of your user directory, which depends on the operating system in use. For Microsoft
Windows, this will be a path of the form 
`C:\Users\<username>\AppData\Roaming\Team Eternity\Calico`

## Default Controls
Keyboard, mouse, and gamepad input are fully supported. The default controls, 
which can be rebound via the configuration file or using the calico-config 
utility, are as follows:

### Keyboard
* WASD : Move the player up/down; strafe left or right
* Left/Right Arrows : Turn left/right
* 1 - 8 : Select weapon
* [ : Previous weapon
* ] : Next weapon
* Speed on : Left Shift
* Strafe on : Right Alt
* Fire : Right Ctrl
* Use : Spacebar
* Toggle menu : Escape
* Pause game : Pause
* Toggle automap : Tab
* Jaguar # Keypad button : Keypad /
* Jaguar * Keypad button : Kepad *
* Jaguar 0 Keypad button : 0

It is possible to use an authentic control scheme by binding the Jaguar A, B, 
and C button actions to keys. When using this control style, strafe and use are
not separate. Use the calico-config utility to bind these keys if desired.

### Mouse
* Left Button : Attack
* Middle Button : Strafe on
* Right Button : Use
* X1 Button : Toggle map
* X2 Button : Pause
* Mouse wheel down/up : Previous/next weapon

### Gamepad
Note: Any device which is recognized by SDL as a game controller can be used.
Game controllers operate on an Xbox-style abstraction which maps buttons and 
axes to the layout of an XInput-compatible controller. For best experience,
use a controller with at least four face buttons, four shoulder triggers, a
directional control pad, and at least one analog stick. Choose and configure
your device using the calico-config utility.

* A : Use
* B : Speed
* X : Strafe-on
* Y : Toggle automap
* Back : Pause
* Start : Toggle menu
* Left stick click : Jaguar * button
* Right stick click : Jaguar # button
* Left shoulder button : Previous weapon
* Right shoulder button : Next weapon
* DPad : Move player up/down/left/right; control menus
* Right trigger : Attack

## Command-line Parameters
* `-devparm` : Enable the original Jaguar game's debug mode.
* `-nosfx` : Disable digital sound effects.
* `-warp x` : Immediately start a new game on level "x", with x between 1 and 25.
* `-skill x` : When warping, use skill level 1 through 5 (from "I'm A Wimp" to "Nightmare!").
* `-fast` : Enable fast monsters regardless of skill level (monsters are always fast on Nightmare skill).
* `-nomonsters` : Do not spawn any monsters (note, this includes lost souls in the Jaguar version).
* `-iwad` : Specify the IWAD file to use - must be a Jaguar Doom IWAD (.WAD or Jaguar ROM format files accepted).
* `-shader` : Specify the base name of a shader (in the shader path) to use instead of "default". GL4 renderer only.
* `-shaderpath` : Path to a directory containing shader files to use. Default: "./shaders". GL4 renderer only.

## Credits
* Programming and Reverse Engineering : James Haley
* Additional Code By : Samuel Villarreal, Rebecca Heineman, Max Waine
* Original Jaguar DOOM Source : John Carmack, id Software
* Special Thanks : Erick194, AXDOOMER, AlexMax
* Setup Program : Derived from chocolate-setup by Simon Howard

## License
All original code, as well as code derived from the 3DO source code, is
available under the MIT license. Code taken from Jaguar DOOM is still under the
original license for that release, which is unfortunately not compatible with
free software licenses (if a source file does not have a license header stating
otherwise, then it is covered by the Jaguar Doom source license).

The calico-config utility is available under the terms of the GNU General Public
License.

## Changes
Calico v2.0 contains the following new features and fixes:
* Gamepad support
* Mouse support
* Enhanced control options: separate use, strafe-on, directional strafing, previous/next weapon actions
* Optional GL 4.0 renderer with shader support; enabled by default
* Support for -fast, -nomonsters, -warp, and -skill command-line parameters
* Ability to quit current game and exit program from internal menus
* Fixed a crash caused by the game's non-working status bar gib face
* Audio code corrected for Windows 10 WASAPI compatibility
* Corrected bullet tracers to use correct line-side checking routine (thanks Erick194)
* Calico Configurator setup utility now included

## Known Issues
This program is in development, so some things are not finished or have known
issues:
* There is no support for music yet.
* There is no multiplayer yet.
* Demos still desync; reason is currently unknown.
