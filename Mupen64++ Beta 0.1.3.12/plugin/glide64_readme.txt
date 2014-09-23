--------------------
Glide64 readme

Glide64 
Version "Wonder++"
Developer: Gonetz
Original author: Dave2001
Other coders: Gugaman

Neither I, nor anyone who is working with me, take ANY responsibility for your
actions or if this program causes harm to anything, including you and your
computer, in any way, physically or mentally.
--------------------

Table of Contents
1. What is Glide64?
2. Why Glide?
3. Will Glide64 work on my computer?
4. What games work with Glide64?
5. Help! What do all these configuration options mean?
6. Can I use Glide64 with a glide wrapper?
7. Can I help with the project?
8. About hardware frame buffer emulation 
9. Troubleshooting FAQ
10. Contacting me
11. Glide64 homepage

-------------------
1. What is Glide64?
-------------------

Glide64 is a graphics plugin for Nintendo 64 emulators that allows 3dfx glide
card users to run games better than they would be able to with other plugins.

-------------
2. Why Glide?
-------------

Glide is an awesome API Because...

* It is standard throughout all 3dfx cards so that no matter which card you run
it on, it will look the same.
* It is insanely fast.
* It is very easy to learn and program in.
* Voodoo cards do not have very good compatability with Direct3D or OpenGL.

------------------------------------
3. Will Glide64 work on my computer?
------------------------------------

Glide64 should work on any computer that uses a 3dfx glide based card.  These are:

3dfx Voodoo Banshee
3dfx Voodoo 1
3dfx Voodoo 2
3dfx Voodoo 3
3dfx Voodoo 4
3dfx Voodoo 5

or any other card based on these.

If you have a card other than the ones listed above, use another plugin instead, 
or use the a glide wrapper. See section 6 for more details on this.

Note: If you are using a Voodoo 2, you cannot use a resolution higher than
800x600 because it doesn't have enough memory for the z buffer.

--------------------------------
4. What games work with Glide64?
--------------------------------

Check the compatibility list.

------------------------------------------------------
5. Help! What do all these configuration options mean?
------------------------------------------------------

You can hold your mouse over any option in the configuration window to open a
tooltip window describing it. Also shown is the recommended (and default)
setting.

------------------------------------------
6. Can I use Glide64 with a glide wrapper?
------------------------------------------

Yes. Recommended glide wrapper is included into this release.
It's designed by Hacktarux, so it's called as Hacktarux's glide wrapper. 
This wrapper designed specially for Glide64 and it currently provides the best image
quality, especially on modern cards. You should also try very promising 
zeckensack's Glide wrapper: http://www.zeckensack.de/glide/

Glide3x is a completely different API than Glide2x, so using a Glide2x wrapper
(the ones that worked with UltraHLE) will not work.

-------------------------------
7. Can I help with the project?
-------------------------------

Sure. Help is always appreciated. In order to help, you need to know or learn
C++ and Glide ON YOUR OWN, and download my sources from my webpage
(http://glide64.emuxhaven.net). You can also find information on how to debug
there. Do not email me until you have actually done something to improve it.
Read the coding rules on the downloads page before mailing me.

----------------------------------------
8. About hardware frame buffer emulation
----------------------------------------

Glide64 now supports hardware frame buffer emulation. What does this mean?
N64 games often use auxiliary frame buffers to implement special game effects
like motion blur or for optimization purposes. The console can allocate as many
auxiliary frame buffers as needed and then use them as usual textures. PC video
cards usually use only one buffer for rendering, thus emulation of frame buffer
effects is always a hard problem. The usual method of emulation, which
theoretically should always work, is to render auxiliary frame buffer into main
video buffer and then read it from video memory into main memory and put into
the structure representing N64 memory (RDRAM). This approach has 2 main
disadvantages:

1) auxiliary frame buffers may be visible when they should not.

2) reading from video memory is a very slow operation, thus many frame buffer effects will be slow.

Besides, image taken from video memory must be scaled down to N64 native
resolution (usually 320x240), thus quality lost is inevitable. To avoid these
problems, Glide64 uses the Glide3x extension which allows it to create auxiliary
frame buffers right into video memory, like the N64 does. This allows the plugin
to run frame buffer effects without speed drop or quality lost. Hardware frame
buffer emulation greatly improves many games, but it has some restrictions:

1) Hardware frame buffer emulation is in fact a hack - auxiliary frame buffers
are not put into N64 memory. When a game uses texture located in area in RDRAM
corresponding to auxiliary frame buffer, the plugin uses texture located in video
memory instead. The game may use this area in RDRAM for different textures
later, but the plugin will use it's auxiliary buffer anyway, which leads to
serious glitches. I have tried to reduce probability of this situation, but it
is still possible. If you encounter it, switch to windowed mode, and then back
to full screen again.

2) The Glide extension used for hardware frame buffer emulation is fully supported
by Voodoo 4/5 only. Banshee and Voodoo3 also support it, but only for small
auxiliary frame buffers (e.g. shadows). Voodoo1-2 do not support it at all!

3) Hardware frame buffer emulation may not work for Voodoo4/5 if the
anti-aliasing option is set to "fastest performance". Use any other option
there.

4) Anti-aliasing is not applicable to auxiliary frame buffers.

5) Hardware frame buffer emulation is supported by current version of Hacktarux's glide wrapper,
included into this release. However, DirectX9 video card with at least 128mb of video memory 
is required for that.

----------------------
9. Troubleshooting FAQ
----------------------

1) Glide64 won't initialize or crashes when I try to switch to fullscreen mode.
2) I get a message that says "Error: uCode crc not found in INI..."
3) Why don't I see anything?  The PJ64 window stays blank!

-----------------------------------------------------

Question #1
Q) Glide64 won't initialize or crashes when I try to switch to fullscreen mode.

A) First of all, if you do not have one of these cards, or a card based on one
of these, you must use a Glide3x wrapper, otherwise Glide64 will not work:

3dfx Voodoo Banshee
3dfx Voodoo 1
3dfx Voodoo 2
3dfx Voodoo 3
3dfx Voodoo 4
3dfx Voodoo 5

If you have one of these cards and it still won't work:
1. UPDATE YOUR DRIVERS at www.voodoofiles.com! This is one of the most common
problems people have. Even if you think you have the most current, update them again!
2. Are you in fullscreen mode?  You MUST be in fullscreen mode to see anything!
3. If you have a Voodoo2, make sure you are not running on a resolution higher than 800x600
4. Look in the compatibility list to make sure that the game you are trying to run is compatible with Glide64
5. Post the problem on the Glide64 message board
6. Contact me (see below)

If you don't have one of these cards, place glide wrapper dll (glide3x.dll) into emulator's root folder.
Emulator's root folder is the folder where main executable file (e.g. Project64.exe or 1964.exe) is located.
zeckensack's wrapper has it's own configuration utility. Hacktarux's wrapper can be configured via Glide64
configuration dialog.
Remember, if you are using wrapper, expect Glide64 to run worse than it would on a 3dfx-based card.


Question #2
Q) I get a message that says "Error: uCode crc not found in INI..."

A) This is because you did not extract the Glide64.ini file into the plugins
directory along with the dll, causing the plugin to create a default ini without
the uCode autodetection section. You will not be able to autodetect uCodes
unless you extract the INI from the zip file.

Question #3
Q) Why don't I see anything?  The emulator window stays blank!

A) This is because Glide cannot be run in a window. Once you switch to
fullscreen mode (Alt+Enter), you will be able to see things. Also, you can set
the 'On loading a ROM go to fullscreen'.

----------------
9. Contacting me
----------------

Post the problem on the message board instead of contacting me if possible!

Only contact me if you really have to!

RULES:
1. NO ROM REQUESTS.
2. NO BETA REQUESTS.
3. No asking if <enter game here> works.
4. No reporting of graphics errors, Glide64 should work mostly the same on all
voodoo cards, if it happens on your computer, it also happens on mine. I can
understand breaking this rule if it's something really good.
5. No graphic error reporting while using a Glide3x wrapper. 

If you still need to contact me, my email address is Gonetz@ngs.ru

--------------------
10. Glide64 homepage
--------------------

The Glide64 homepage is located at:
http://www.emuxhaven.net/~glide64/

The Glide64 IRC channel is located on EFnet at #Glide64
NO ROM REQUESTS, NO BETA REQUESTS
