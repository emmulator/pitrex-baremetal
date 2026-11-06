The four screen-settings files
==============================

PiTrex programs read their screen settings (beam delays, scale limits, calibration) from vectrexInterface.ini in the
root of the SD card. Each Vectrex draws a little differently, so the right values differ from one Vectrex to the next.
This folder holds four versions of that file. None of them is "the best": try them and keep the one that gives you a
steady, readable picture, then fine-tune it with SETTINGS in the menu.

The files, oldest first. Every one is an unchanged copy of the original; the name says where it comes from.

  vectrexInterface-malban-2021-03.ini
      From Malban's March 2021 card (pitrexbaremetal.img). The oldest and shortest file: it only sets what Malban
      thought was needed then, and everything else falls back to the values built into the programs. Beam delays
      2 / 3 / 2 and 6, MINSCALE 1, calibration 0. In-game settings off.

  vectrexInterface-rpiteam-2021-12.ini
      From the RpiTeaM Edition image, which is Malban's December 2021 release. Same idea as the March 2021 file,
      a little different delays (2 / 1 / 1 and 1), MINSCALE 2, BUFFER_TYPE 2. In-game settings on.

  vectrexInterface-malban-2025-07.ini
      From Malban's July 2025 card (PiTrex_sd_Jul29_25). This is the file that is already on that card, and the
      one the installer leaves alone. It has the most settings, and its values look tuned for Malban's own Vectrex:
      all beam delays 0, MINSCALE 11, and six calibration values (4, 51, 61, 70, 98, 103) that correct vector
      lengths. In-game settings on.

  vectrexInterface-alt.ini   (with vectrex-alt.ini)
      The settings that the author of this menu (emmulator) ended up with after calibrating their own Vectrex (a
      "Buzz" model): beam delays 2 / 2 / 2 and 6, MINSCALE 20, calibration values 2, 3, 3, 0, 0, 0. In-game
      settings off. vectrex-alt.ini goes with it: it makes the Vectrex emulator use the original Vectrex BIOS
      (bios/7ADB.bin, with the Vectrex intro) instead of FASTBOOT.IMG. Only this one comes with a vectrex.ini.

Why try another one? If the picture on your Vectrex is garbled, badly scaled, shifted or too faint to find the
settings menu with the 2025 file, one of the older or the alternative files may suit your Vectrex better. One user
reported that Malban's 2021 and the RpiTeaM releases looked fine on their Vectrex while the 2025 one did not, and the
2021 values are the first thing to try. The 2025 file may of course be the right one for you.

How to try one
--------------

With the installer (it saves the file it replaces in the backup folder, and changes nothing else):

    ./install.sh --settings malban-2021 <card folder>          Linux, macOS
    install.bat --settings malban-2021 E:\                      Windows

The values are: malban-2021, rpiteam-2021, malban-2025 and alt. Run it again with another value to switch; use
--restore to go back to what the card had before.

By hand: copy the file you want to the root of the SD card and rename it to vectrexInterface.ini. Keep a copy of
the one it replaces. For the alt file also copy vectrex-alt.ini to vectrex.ini.

Leaving SETTINGS in the menu (Button 4) writes the current values to vectrexInterface.ini in the full format of the
2025 programs, whichever file you started with.
