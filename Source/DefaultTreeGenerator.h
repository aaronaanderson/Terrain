#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include "Identifiers.h"

struct TrajectoryVariablesTree
{
    static const juce::ValueTree create()
    {
        juce::ValueTree tree (id::TRAJECTORY_VARIABLES);
        tree.setProperty (id::size, 0.5f, nullptr);
        tree.setProperty (id::rotation, 0.0f, nullptr);
        tree.setProperty (id::translation_x, 0.0f, nullptr);
        tree.setProperty (id::translation_y, 0.0f, nullptr);
        
        tree.setProperty (id::attack, 80.0f, nullptr);
        tree.setProperty (id::decay, 20.0f, nullptr);
        tree.setProperty (id::sustain, 0.7f, nullptr);
        tree.setProperty (id::release, 400.0f, nullptr);
        tree.setProperty (id::envelopeSize, true, nullptr);

        tree.addChild (createFeedbackBranch(), -1, nullptr);
        return tree;
    }
private:
    static const juce::ValueTree createFeedbackBranch()
    {
        juce::ValueTree tree (id::FEEDBACK);
        tree.setProperty (id::feedbackScalar, 0.0, nullptr);
        tree.setProperty (id::feedbackTime, 200.0, nullptr);
        tree.setProperty (id::feedbackMix, 0.0, nullptr);
        tree.setProperty (id::feedbackCompression, 4.0f, nullptr);
        return tree;
    }
};
struct TrajectoriesTree
{
    static const juce::ValueTree create()
    {
        juce::ValueTree tree (id::TRAJECTORIES);
        tree.setProperty (id::currentTrajectory, "Ellipse", nullptr);
        tree.addChild (createTrajectoryType ("Ellipse", {0.5f}), -1, nullptr);
        tree.addChild (createTrajectoryType ("Limacon", {0.5f, 0.5f}), -1, nullptr);
        tree.addChild (createTrajectoryType ("Butterfly", {0.5f, 0.5f, 0.4f, 0.4f}), -1, nullptr);
        tree.addChild (createTrajectoryType ("Scarabaeus", {0.5f, 0.5f}), -1, nullptr);
        tree.addChild (createTrajectoryType ("Squarcle", {0.5f}), -1, nullptr);
        return tree;
    }
private:
    static const juce::ValueTree createTrajectoryType (juce::String name, const juce::Array<float> mods)
    {
        jassert (mods.size() <= 4);
        auto tree = juce::ValueTree (id::TRAJECTORY);
        tree.setProperty (id::type, name, nullptr);
        
        auto modifiersBranch = juce::ValueTree (id::MODIFIERS);
        juce::Array<juce::Identifier> modIDArray = {id::mod_A, id::mod_B, id::mod_C, id::mod_D};
        for (int i = 0; i < mods.size(); i++)
            modifiersBranch.setProperty (modIDArray[i], mods[i], nullptr);

        tree.addChild (modifiersBranch, -1, nullptr);
        return tree;
    }

};
struct TerrainVariablesTree
{
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::TERRAIN_VARIABLES);
        tree.setProperty (id::terrainSaturation, 1.0f, nullptr);

        return tree;
    }
};
struct TerrainsTree
{
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::TERRAINS);
        tree.setProperty (id::currentTerrain, "Sinusoidal", nullptr);
        tree.addChild (createTerrainType ("Sinusoidal", {0.5f, 0.5f}), -1, nullptr);
        tree.addChild (createTerrainType ("Wiggly", {0.5f, 0.5f}), -1, nullptr);
        tree.addChild (createTerrainType ("Wobbly", {0.5f, 0.2f}), -1, nullptr);
        tree.addChild (createTerrainType ("System 3", {0.3f}), -1, nullptr);
        tree.addChild (createTerrainType ("System 9", {0.5f, 0.5f, 0.5f, 0.5f}), -1, nullptr);

        return tree;
    }
    static const juce::ValueTree createTerrainType (juce::String name, const juce::Array<float> mods)
    {
        jassert (mods.size() <= 4);
        auto tree = juce::ValueTree (id::TERRAIN);
        tree.setProperty (id::type, name, nullptr);
        
        auto modifiersBranch = juce::ValueTree (id::MODIFIERS);
        juce::Array<juce::Identifier> modIDArray = {id::mod_A, id::mod_B, id::mod_C, id::mod_D};
        for (int i = 0; i < mods.size(); i++)
            modifiersBranch.setProperty (modIDArray[i], mods[i], nullptr);

        tree.addChild (modifiersBranch, -1, nullptr);
        return tree;
    }
};
struct ControlsTree
{
    static juce::ValueTree create()
    {
        juce::ValueTree tree (id::CONTROLS);
        tree.setProperty (id::oversampling, 1, nullptr);

        tree.setProperty (id::filterFrequency, 200.0f, nullptr);
        tree.setProperty (id::filterResonance, 0.5f, nullptr);
        tree.setProperty (id::filterOnOff, false, nullptr);
        return tree;
    }
};
struct DefaultTree
{
    static const juce::ValueTree create()
    {
        auto tree = juce::ValueTree (id::TERRAINSYNTH);
        tree.addChild (TrajectoriesTree::create(), -1, nullptr);
        tree.addChild (TrajectoryVariablesTree::create(), -1, nullptr);
        tree.addChild (TerrainsTree::create(), -1, nullptr);
        tree.addChild (TerrainVariablesTree::create(), -1, nullptr);
        tree.addChild (ControlsTree::create(), -1, nullptr);
        return tree;
    }
};