## TES5Edit Stub Executable

This is a small executable stub I wrote in order provide the ability for launching FO4Edit from the [Nexus Mod Manager](http://www.nexusmods.com/games/mods/modmanager/)
application. While NMM does have [TES5Edit](https://github.com/TES5Edit/TES5Edit) as one of the tools listed in its
Fallout 4 settings under the **Supported Tools** tab, it expects the executable name to be **TES5Edit.exe**. Since using
TES5Edit for Fallout 4 modding requires you to rename the executable to **FO4Edit.exe**, it becomes impossible to launch
the launcher in FO4 mode.

To fix this, I wrote this stub which executes the **FO4Edit.exe** from the same folder, (passing along any
command-line arguments that it received) waits for the editor window to open, then brings it the foreground. 

### Building

Running the [build.cmd](build.cmd) should build the executable with the newest version of Visual Studio it finds. (Aims
to support Visual Studio 2005-2015, but 2005 and 2008 are probably broken since I was too lazy to look up the available
compiler and linker command-line options. Instead, I used the options from 2010.)

It should probably be possible to build the executable with some other compiler, (icc, mingw, etc) but I haven't attempted
to do so.

### Installation

Build it yourself or download the release.

After you've renamed the real **TES5Edit.exe** to **FO4Edit.exe**, copy this stub executable into the same folder as
**FO4Edit.exe** and set the tool up in Nexus Mod Manager. Once NMM has detected an existing **TES5Edit.exe** file,
the tool will show up in the **Supported Tools** drop down. (screenshot below)

![TES5Edit in Supported Tools](http://i.imgur.com/uCtOBq8.png)

### Credits & License

Everything in this folder besides the [TES5Edit icon](TES5Edit.ico) are under WTFPL 2.0, meaning you can do whatever you
want with them and don't need my permission. The icon was taken from the original TES5Edit in order to show the right
tool icon in NMM.

