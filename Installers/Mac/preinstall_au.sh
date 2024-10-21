#!/bin/sh

AUSYS=/Library/Audio/Plug-Ins/Components/Terrain.component
if [ -d "$AUSYS" ]; then
rm -r "$AUSYS"
fi
