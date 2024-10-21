xcrun notarytool submit \
    "build/Terrain.pkg" \
    --keychain-profile "Aaron" \
    --wait

xcrun stapler staple \
    "build/Terrain.pkg"