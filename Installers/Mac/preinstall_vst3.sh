#!/bin/sh

VST3SYS=/Library/Audio/Plug-Ins/VST3/Terrain.vst3
if [ -d "$VST3SYS" ]; then
rm -r "$VST3SYS"
fi
