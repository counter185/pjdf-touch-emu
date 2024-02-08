# pjdf-touch-emu
This is a PS Vita plugin that emulates swipes on the rear touch pad with analog stick movements, intended for Project Diva f.

Tested on a Vita 1000, would appreciate if someone tested on a PSTV.

## Installation
Copy the plugin to your `tai` folder, then add it to `config.txt` under the game's ID. For the EU version, this will be:

```
*PCSB00419
ur0:tai/divaf-touch-emu.suprx
```

In the game, go to `Other` -> `Options` and set `Scratch Target` to either `Rear Touch Pad` or `Both`.

## Building

```bash
mkdir build
cd build
cmake ..
make
```
