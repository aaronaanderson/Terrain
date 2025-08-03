#!/bin/sh

AUSYS=/Library/Audio/Plug-Ins/Components/Terrain.component
if [ -d "$AUSYS" ]; then
rm -r "$AUSYS"
fi

# Get the logged-in user
userHome=$(eval echo "~$USER")
targetDir="$userHome/Library/Audio/Presets/Aaron Anderson"

mkdir -p "$targetDir"
chmod 755 "$targetDir"

# Optional: copy your preset files into the folder
# cp -R "/path/to/your/presets/." "$targetDir"

exit 0