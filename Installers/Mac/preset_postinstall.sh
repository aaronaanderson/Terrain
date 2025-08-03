#!/bin/bash

# Get the logged-in user's name and home directory
loggedInUser=$(stat -f "%Su" /dev/console)
userHome=$(dscl . -read /Users/$loggedInUser NFSHomeDirectory | awk '{print $2}')
targetDir="$userHome/Library/Audio/Presets/Aaron Anderson"

# Create the target directory
mkdir -p "$targetDir"

# Copy preset files into the target
cp -R /tmp/PresetPayload/. "$targetDir"

# Make the user the owner of the folder and its contents
chown -R "$loggedInUser" "$targetDir"

# Set: user = read/write, group = read-only, others = read-only
chmod -R 644 "$targetDir"/*        # files: -rw-r--r--
find "$targetDir" -type d -exec chmod 755 {} \;  # folders: drwxr-xr-x

# Optional: strip inherited ACLs to avoid hidden overrides
chmod -RN "$targetDir"

# Clean up
rm -rf /tmp/PresetPayload

exit 0