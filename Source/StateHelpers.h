// #pragma once

// #include <juce_data_structures/juce_data_structures.h>

// static juce::ValueTree getCurrentTrajectoryBranch (juce::ValueTree trajectoriesBranch)
// {
//     jassert (trajectoriesBranch.getType() == id::TRAJECTORIES);
//     auto trajectoryType = trajectoriesBranch.getProperty (id::currentTrajectory).toString();
//     for (int i = 0; i < trajectoriesBranch.getNumChildren(); i++)
//         if (trajectoriesBranch.getChild (i).getProperty (id::type).toString() == trajectoryType)
//             return trajectoriesBranch.getChild (i);

//     jassertfalse;
//     return {};
// }
// static juce::ValueTree getCurrentTerrainBranch (juce::ValueTree terrainsBranch)
// {
//     jassert (terrainsBranch.getType() == id::TERRAINS);
//     auto trajectoryType = terrainsBranch.getProperty (id::currentTerrain).toString();
//     for (int i = 0; i < terrainsBranch.getNumChildren(); i++)
//         if (terrainsBranch.getChild (i).getProperty (id::type).toString() == trajectoryType)
//             return terrainsBranch.getChild (i);

//     jassertfalse;
//     return {};
// }
