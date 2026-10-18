# pitrex-baremetal
![PiRrex](http://vide.malban.de/wp-content/uploads/2020/01/pitrex-1-1024x843.png)

## This fork: a new, card-driven menu

This fork adds an optional list menu that is built from what is on the SD card (nothing is compiled in), plus
the tools to put it on Malban's card.

* **Just want to use it?** Read [docs/README.md](docs/README.md): what to download, how to install, and which ROM
  files it looks for and where they go.
* **Want to change the menu or add your own games?** Read [NEW_MENU_README.md](NEW_MENU_README.md).
* Development notes, findings and tools: [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md). Licensing notes: [docs/LICENSING.md](docs/LICENSING.md).

The rest of this page is the original project's README.

Read more about PiTrex in general at: [PiTrex Wiki](http://www.ombertech.com/cnk/pitrex/wiki/index.php).

Read more about PiTrex Baremetal at: [Malbans PiTrex baremetal pages](http://vide.malban.de/pitrex) (and the pages accessable by the menu).

To develop baremetal PiTrex you can download a prepared virtual machine for Virtual box (Ubuntu 16.04). Don't forget to update the sources with the ones found here (on github):  [PiTrex baremetal development virtual machine](http://vide.malban.de/pitrex/pitrex-baremetal-quick-start-unfinished).


# Structure of this repository

## Baremetal "library"
All stuff under the directory "pitrex" is the actual "baremetal boot source" to build baremetal programs. For a short description of the "bootup" look at: [PiTrex – Baremetal knowledge](http://vide.malban.de/pitrex/pitrex-baremetal-knowledge).

## Baremetal "vectrexInterface"
All stuff under the directory "pitrex/vectrex" is the actual "interface" to the vectrex. For a short description at: [
PiTrex baremetal: vectrexInterface.c](http://vide.malban.de/pitrex/pitrex-baremetal-vectrexinterface-c).

## Other
All other "single" directories within the repository represent single "programs" runnable under the baremetal environment. Within each directory is a single Makefiles.baremetal to create baremetal images.
