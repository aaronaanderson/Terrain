#!/bin/bash

# Get the currently logged-in user's home directory
loggedInUser=$(stat -f "%Su" /dev/console)
userHome=$(dscl . -read /Users/$loggedInUser NFSHomeDirectory | awk '{print $2}')
targetDir="$userHome/Library/Audio/Presets/Aaron Anderson/Terrain"

# Create folder if needed
mkdir -p "$targetDir"
chmod 755 "$targetDir"

# Copy the presets into the user folder
cp -R /tmp/Presets/. "$targetDir"

# Clean up temp files
rm -rf /tmp/Presets

exit 0