#pragma once

#include <juce_data_structures/juce_data_structures.h>

static juce::ValueTree getCurrentTrajectoryBranch (juce::ValueTree trajectoriesBranch)
{
    jassert (trajectoriesBranch.getType() == id::TRAJECTORIES);
    auto trajectoryType = trajectoriesBranch.getProperty (id::currentTrajectory).toString();
    for (int i = 0; i < trajectoriesBranch.getNumChildren(); i++)
        if (trajectoriesBranch.getChild (i).getProperty (id::type).toString() == trajectoryType)
            return trajectoriesBranch.getChild (i);

    jassertfalse;
    return {};
}
