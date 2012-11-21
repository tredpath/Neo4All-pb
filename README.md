Neo4All-pb
==========

A Neo Geo CD emulator for the Blackberry Playbook.

It has the same basic controls as the Neo Geo.

In order to use this emulator a Neo Geo CD BIOS is required. The BIOS must be named neocd.bin (case sensitive) and be placed in shared/misc/NEOCD/bios.

The ISO images should be at least an ISO and a CUE file each with the same filename (TOC and other file types are not fully supported although the game should be playable with them). There are 2 types of audio in Neo Geo CD games: CDDA and SFX. Some games, like Bust a Move use SFX for all audio, in which case no action is required. If the game uses CDDA then along with the ISO and CUE files there will be a set of MP3 files. The CUE file will tell the emulator where these MP3 files are. So, if the game uses CDDA there may be an entry such as

FILE "MS2/MetalSlug202.mp3" MP3

in the CUE file which means that in the folder MS2 (relative to shared/misc/NEOCD/iso) there is a file named MetalSlug202.mp3. The games are completely playable without these files but if you notice that there is no background music where there should be these file names may be the problem.

If a game doesn't work before sending me a message please view:

http://chui.dcemu.co.uk/neo4all.html#GameCompatibilityList

to make sure that the game that you are playing is supported by the emulator. Although most games are supported there are still some that are not.


Neo4All
=======

Neo4All is an open source project that is based on NeoCD.

Neo4All can be found at:

http://chui.dcemu.co.uk/neo4all.html

and the original NeoCD project can be found at:

http://pacifi3d.retrogames.com/neocdsdl/


Required for Building
=====================

The following projects have been included in this distribution although some may not be needed for core functionality:

https://github.com/blackberry/TouchControlOverlay

https://github.com/blackberry/SDL

https://github.com/asimonov-rim/SDL_mixer

https://github.com/asimonov-rim/SDL_image

https://github.com/asimonov-rim/vorbis

https://github.com/asimonov-rim/ogg
