#include "MainProcessor.h"
#include "MainEditor.h"

#include "DefaultTreeGenerator.h"

juce::StringArray getTrajectoryChoices(juce::ValueTree trajectoriesTree)
{
    juce::StringArray sa;
    jassert (trajectoriesTree.getType() == id::TRAJECTORIES);
    for (int i = 0; i < trajectoriesTree.getNumChildren(); i++)
        sa.add (trajectoriesTree.getChild (i).getProperty (id::type).toString());
    return sa;
}
const juce::String MainProcessor::trajectoryNameFromIndex (int i)
{
    auto trajectories = state.getChildWithName (id::TRAJECTORIES);
    return trajectories.getChild (i).getProperty (id::type).toString();
}
//==============================================================================
MainProcessor::MainProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
        state (DefaultTree::create())
{
    addParameter (currentTrajectoryParameter = new tp::ChoiceParameter ("Current Trajectory", 
        getTrajectoryChoices (state.getChildWithName (id::TRAJECTORIES)), 
        "", 
        [&](int i){state.getChildWithName (id::TRAJECTORIES).setProperty (id::currentTrajectory, trajectoryNameFromIndex(i), &undoManager);}));

}

MainProcessor::~MainProcessor() {}

//==============================================================================
const juce::String MainProcessor::getName() const  { return JucePlugin_Name; }
bool MainProcessor::acceptsMidi() const            { return true; }
bool MainProcessor::producesMidi() const           { return false; }
bool MainProcessor::isMidiEffect() const           { return false; }
double MainProcessor::getTailLengthSeconds() const { return 0.0; }
int MainProcessor::getNumPrograms() { return 1; }

int MainProcessor::getCurrentProgram()             { return 0;  }
void MainProcessor::setCurrentProgram (int index)  { juce::ignoreUnused (index); }
const juce::String MainProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}
void MainProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }
//==============================================================================
void MainProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) { juce::ignoreUnused (sampleRate, samplesPerBlock); }
void MainProcessor::releaseResources() {}
bool MainProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
void MainProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}
//==============================================================================
bool MainProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* MainProcessor::createEditor() { return new MainEditor (*this); }
//==============================================================================
void MainProcessor::getStateInformation (juce::MemoryBlock& destData) 
{ 
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
void MainProcessor::setStateInformation (const void* data, int sizeInBytes)
{ 
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() == nullptr) return; // make sure we have data
    if (!xml->hasTagName (state.getType())) return; // make sure it's the right data

    state = juce::ValueTree::fromXml (*xml);
}
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MainProcessor(); }