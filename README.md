# Calico
Calico is a port of the Jaguar Doom codebase back to the PC, aiming to support playing the game in its original form on that 
console with a minimum number of required changes. When finished, it will accept either the game's ROM file or the extracted 
jagdoom.wad file from inside it. You will need a backup device for Jaguar ROM cartridges in order to legally play the game.

The project has just started and does not currently compile. I am still in the process of reverse engineering the GAS assembly files
for the renderer, which are written in a custom assembly language understood only by the custom Tom and Jerry ASICs on the Jaguar 
motherboard. Gameplay GAS files have already been replaced by cross-referencing against the 3DO source code and the Doom64 EX 
project, both of which share about 99% of that code but already in a C form.

Calico is named in honor of my late cat, Nikki Fritz, who was a calico tabby mix, because it is a "little kitty" of a Doom port.
