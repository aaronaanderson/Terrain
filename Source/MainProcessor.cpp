#include "MainProcessor.h"
#include "MainEditor.h"

#include "DefaultTreeGenerator.h"


static juce::StringArray getChoices (juce::ValueTree tree)
{
    juce::StringArray sa;
    jassert (tree.getType() == id::TRAJECTORIES ||
             tree.getType() == id::TERRAINS);
    for (int i = 0; i < tree.getNumChildren(); i++)
        sa.add (tree.getChild (i).getProperty (id::type).toString());
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

    //======================================Trajectory Parameters
    auto trajectoriesBranch = state.getChildWithName (id::TRAJECTORIES);
    jassert (trajectoriesBranch.getType() == id::TRAJECTORIES);
    addParameter (parameters.currentTrajectory = new tp::ChoiceParameter ("Current Trajectory", 
        getChoices (state.getChildWithName (id::TRAJECTORIES)), 
        "", 
        getCurrentTrajectoryIndexFromString (trajectoriesBranch.getProperty (id::currentTrajectory).toString())));

    auto modifiersBranch = getCurrentTrajectoryBranch (trajectoriesBranch).getChildWithName (id::MODIFIERS);
    jassert (modifiersBranch.getType() == id::MODIFIERS);
    addParameter (parameters.trajectoryModA = new tp::NormalizedFloatParameter ("Trajectory Mod A", modifiersBranch.getProperty (id::mod_A)));
    addParameter (parameters.trajectoryModB = new tp::NormalizedFloatParameter ("Trajectory Mod B", modifiersBranch.getProperty (id::mod_B)));
    addParameter (parameters.trajectoryModC = new tp::NormalizedFloatParameter ("Trajectory Mod C", modifiersBranch.getProperty (id::mod_C)));
    addParameter (parameters.trajectoryModD = new tp::NormalizedFloatParameter ("Trajectory Mod D", modifiersBranch.getProperty (id::mod_D)));
    
    auto trajectoryVariablesBranch = state.getChildWithName (id::TRAJECTORY_VARIABLES);
    jassert (trajectoryVariablesBranch.getType() == id::TRAJECTORY_VARIABLES);
    addParameter (parameters.trajectorySize = new tp::NormalizedFloatParameter ("Size", trajectoryVariablesBranch.getProperty (id::size)));
    addParameter (parameters.trajectoryRotation = new tp::RangedFloatParameter ("Rotation", 
                                                                                {0.0f, juce::MathConstants<float>::twoPi}, 
                                                                                trajectoryVariablesBranch.getProperty (id::rotation)));
    addParameter (parameters.trajectoryTranslationX = new tp::RangedFloatParameter ("Translation X", 
                                                                                    {-1.0f, 1.0f}, 
                                                                                    (trajectoryVariablesBranch.getProperty (id::translation_x))));
    addParameter (parameters.trajectoryTranslationY = new tp::RangedFloatParameter ("Translation Y", 
                                                                                    {-1.0f, 1.0f},
                                                                                    (trajectoryVariablesBranch.getProperty (id::translation_y))));
    addParameter (parameters.meanderanceScale = new tp::NormalizedFloatParameter ("Meanderance Scale", 
                                                                                  trajectoryVariablesBranch.getProperty (id::meanderanceScale)));
    addParameter (parameters.meanderanceSpeed = new tp::RangedFloatParameter ("Meanderance Speed", 
                                                                              {0.0f, 1.0f}, 
                                                                              trajectoryVariablesBranch.getProperty (id::meanderanceSpeed)));

    
    addParameter (parameters.envelopeSize = new juce::AudioParameterBool ({"envelopeSize", 1}, "Envelope Size", true));
    auto range = juce::NormalisableRange<float> (2.0f, 2000.0f); range.setSkewForCentre (100.0f);
    addParameter (parameters.attack = new tp::RangedFloatParameter ("Attack", 
                                                                    range, 
                                                                    trajectoryVariablesBranch.getProperty (id::attack)));
    range = juce::NormalisableRange<float> (2.0f, 1000.0f); range.setSkewForCentre (50.0f);
    addParameter (parameters.decay = new tp::RangedFloatParameter ("Decay", 
                                                                    range, 
                                                                    trajectoryVariablesBranch.getProperty (id::decay)));
    range = juce::NormalisableRange<float> (-24.0f, 0.0f);
    addParameter (parameters.sustain = new tp::RangedFloatParameter ("sustain", 
                                                                     range, 
                                                                     trajectoryVariablesBranch.getProperty (id::sustain)));
    range = juce::NormalisableRange<float> (10.0f, 4000.0f); range.setSkewForCentre (800.0f);
    addParameter (parameters.release = new tp::RangedFloatParameter ("Release", 
                                                                    range, 
                                                                    trajectoryVariablesBranch.getProperty (id::release)));
    
    
    auto trajectoryFeedbackBranch = trajectoryVariablesBranch.getChildWithName (id::FEEDBACK);
    jassert (trajectoryFeedbackBranch.getType() == id::FEEDBACK);
    range = juce::NormalisableRange<float> (0.0f, 2000.0f); range.setSkewForCentre (250.0f);
    addParameter (parameters.feedbackTime = new tp::RangedFloatParameter ("Feedback Time", 
                                                                          range,
                                                                          (trajectoryFeedbackBranch.getProperty (id::feedbackTime))));
    range = juce::NormalisableRange<float> (0.0f, 0.9999f); range.setSkewForCentre (0.8f);
    addParameter (parameters.feedbackScalar = new tp::RangedFloatParameter ("Feedback", 
                                                                            range,
                                                                            (trajectoryVariablesBranch.getProperty (id::feedbackScalar))));
    addParameter (parameters.feedbackCompression = new tp::RangedFloatParameter ("Feedback Compression", 
                                                                                 {1.0f, 20.0f},
                                                                                 (trajectoryVariablesBranch.getProperty (id::feedbackCompression))));
    addParameter (parameters.feedbackMix = new tp::RangedFloatParameter ("Feedback Mix", 
                                                                         {0.0f, 1.0f},
                                                                         (trajectoryVariablesBranch.getProperty (id::feedbackScalar))));

    //=======================================Terrain Parameters
    auto terrainsBranch = state.getChildWithName (id::TERRAINS);
    jassert (terrainsBranch.getType() == id::TERRAINS);
    addParameter (parameters.currentTerrain = new tp::ChoiceParameter ("Current Terrain", 
        getChoices (state.getChildWithName (id::TERRAINS)), 
        "",  
        getCurrentTerrainIndexFromString (terrainsBranch.getProperty (id::currentTerrain).toString())));
    modifiersBranch = terrainsBranch.getChildWithName (id::MODIFIERS);
    addParameter (parameters.terrainModA = new tp::NormalizedFloatParameter ("Terrain Mod A", modifiersBranch.getProperty (id::mod_A)));
    addParameter (parameters.terrainModB = new tp::NormalizedFloatParameter ("Terrain Mod B", modifiersBranch.getProperty (id::mod_B)));
    addParameter (parameters.terrainModC = new tp::NormalizedFloatParameter ("Terrain Mod C", modifiersBranch.getProperty (id::mod_C)));
    addParameter (parameters.terrainModD = new tp::NormalizedFloatParameter ("Terrain Mod D", modifiersBranch.getProperty (id::mod_D)));

    auto terrainVariablesBranch = state.getChildWithName (id::TERRAIN_VARIABLES);
    jassert (terrainVariablesBranch.getType() == id::TERRAIN_VARIABLES);
    range = juce::NormalisableRange<float> (1.0f, 16.0f); range.setSkewForCentre (4.0f);
    addParameter (parameters.terrainSaturation = new tp::RangedFloatParameter ("Terrain Saturation", 
                                                                               range,
                                                                               (terrainVariablesBranch.getProperty (id::feedbackScalar))));

    auto controlsBranch = state.getChildWithName (id::CONTROLS);
    addParameter (parameters.filterResonance = new tp::NormalizedFloatParameter ("Filter Resonance", controlsBranch.getProperty (id::filterResonance)));
    range = juce::NormalisableRange<float> (20.0f, 10000.0f); range.setSkewForCentre (500.0f);
    addParameter (parameters.filterFrequency = new tp::RangedFloatParameter ("Filter Frequency", 
                                                                             range,
                                                                             (controlsBranch.getProperty (id::filterFrequency))));
    addParameter (parameters.filterOnOff = new juce::AudioParameterBool ({"FilterOnOff", 1}, "Filter Bypass", controlsBranch.getProperty (id::filterOnOff)));

    addParameter (parameters.compressorThreshold = new tp::RangedFloatParameter ("Compressor Threshold", 
                                                                                 {-24.0f, 0.0f},
                                                                                 (controlsBranch.getProperty (id::compressionThreshold))));
    addParameter (parameters.compressorRatio = new tp::RangedFloatParameter ("Compressor Ratio", 
                                                                             {1.0f, 12.0f},
                                                                             (controlsBranch.getProperty (id::compressionRatio))));
    addParameter (parameters.outputLevel = new tp::RangedFloatParameter ("Output Level", 
                                                                         {-60.0f, 6.0f}, 
                                                                         controlsBranch.getProperty (id::outputLevel)));
    
    state.addListener (this);

    synthesizer = std::make_unique<tp::WaveTerrainSynthesizer> (parameters);
    outputChain.reset();
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
void MainProcessor::prepareToPlay (double sr, int size) 
{ 
    sampleRate = sr; maxSamplesPerBlock = size;
    
    renderBuffer.setSize (1, maxSamplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = static_cast<juce::uint32> (size);
    spec.numChannels = 2;
    spec.sampleRate = sr;

    auto& dcOffset = outputChain.get<0>();
    dcOffset.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (spec.sampleRate, 20.0);

    auto& ladderFilter = outputChain.get<1>();
    ladderFilter.setEnabled (true);
    ladderFilter.setMode (juce::dsp::LadderFilterMode::LPF24);
    ladderFilter.setDrive (1.0f);

    auto& compressor = outputChain.get<2>();
    compressor.setAttack (20.0f);

    auto& outputLevel = outputChain.get<3>();
    outputLevel.setRampDurationSeconds (0.02);

    outputChain.prepare (spec);
}
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
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    prepareOversampling (buffer.getNumSamples());
    synthesizer->updateTerrain();
    auto overSamplingBlock = overSampler->processSamplesUp (renderBuffer);
    juce::Array<float*> channelPointers = {overSamplingBlock.getChannelPointer(0)};
    juce::AudioBuffer<float> overSamplingBufferReference (channelPointers.getRawDataPointer(), 
                                                          static_cast<int> (overSamplingBlock.getNumChannels()), 
                                                          static_cast<int> (overSamplingBlock.getNumSamples()));

    synthesizer->renderNextBlock (overSamplingBufferReference, midiMessages, 0, overSamplingBufferReference.getNumSamples());
    auto outputBlock = juce::dsp::AudioBlock<float> (renderBuffer);
    overSampler->processSamplesDown (outputBlock);

    auto& ladderFilter = outputChain.get<1>();
    ladderFilter.setCutoffFrequencyHz ((*parameters.filterFrequency));
    ladderFilter.setResonance (*parameters.filterResonance);
    ladderFilter.setEnabled (*parameters.filterOnOff);
    
    auto& compressor = outputChain.get<2>();
    compressor.setThreshold (*parameters.compressorThreshold);
    compressor.setRatio (*parameters.compressorRatio);
    
    auto& outputLevel = outputChain.get<3>();
    outputLevel.setGainDecibels (*parameters.outputLevel);

    juce::dsp::ProcessContextReplacing<float> context (outputBlock);
    outputChain.process (context);

    for (int c = 0; c < buffer.getNumChannels(); c++)
        buffer.copyFrom (c, 0, renderBuffer.getReadPointer (0), buffer.getNumSamples());
    
    renderBuffer.clear();
}
//==============================================================================
bool MainProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* MainProcessor::createEditor() { return new MainEditor (*this); }
//==============================================================================
void MainProcessor::getStateInformation (juce::MemoryBlock& destData) 
{ 
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
    // std::cout << "output\n" << xml->toString() << std::endl;
    // if (destData.getData() == nullptr) return;
    // auto outputStream = juce::MemoryOutputStream (destData.getData(), destData.getSize());
    // state.writeToStream (outputStream);
}
void MainProcessor::setStateInformation (const void* data, int sizeInBytes)
{ 
    juce::ignoreUnused (data, sizeInBytes);
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() == nullptr) return; // make sure we have data
    if (!xml->hasTagName (state.getType())) return; // make sure it's the right data
    state = juce::ValueTree::fromXml (*xml);
    std::cout << "input\n" << xml->toString() << std::endl;
    resetParameterState();
    
    // state.readFromData (data, sizeInBytes);
    // auto inputStream = juce::MemoryInputStream (data, sizeInBytes, false);
    // state.readFromStream (inputStream);
}
void MainProcessor::resetParameterState()
{
    auto trajectoriesBranch = state.getChildWithName (id::TRAJECTORIES);
    setCurrentTrajectoryParamFromString (trajectoriesBranch.getProperty (id::currentTrajectory).toString());
    auto currentTrajectoryBranch = getCurrentTrajectoryBranch (state.getChildWithName (id::TRAJECTORIES));
    auto trajectoryModifiersBranch = currentTrajectoryBranch.getChildWithName (id::MODIFIERS);
    jassert (currentTrajectoryBranch.getType() == id::TRAJECTORY);
    parameters.trajectoryModA->setValueNotifyingHost (trajectoryModifiersBranch.getProperty (id::mod_A));
    parameters.trajectoryModB->setValueNotifyingHost (trajectoryModifiersBranch.getProperty (id::mod_B));
    parameters.trajectoryModC->setValueNotifyingHost (trajectoryModifiersBranch.getProperty (id::mod_C));
    parameters.trajectoryModD->setValueNotifyingHost (trajectoryModifiersBranch.getProperty (id::mod_D));
    
    auto terrainsBranch = state.getChildWithName (id::TERRAINS);
    setCurrentTerrainFromString (terrainsBranch.getProperty (id::currentTerrain).toString());
    auto currentTerrainBranch = getCurrentTerrainBranch (state.getChildWithName (id::TERRAINS));
    auto terrainModifiersBranch = currentTerrainBranch.getChildWithName (id::MODIFIERS);
    parameters.terrainModA->setValueNotifyingHost (terrainModifiersBranch.getProperty (id::mod_A));
    parameters.terrainModB->setValueNotifyingHost (terrainModifiersBranch.getProperty (id::mod_B));
    parameters.terrainModC->setValueNotifyingHost (terrainModifiersBranch.getProperty (id::mod_C));
    parameters.terrainModD->setValueNotifyingHost (terrainModifiersBranch.getProperty (id::mod_D));
            
    auto trajectoryVariablesBranch = state.getChildWithName (id::TRAJECTORY_VARIABLES);
    jassert (trajectoryVariablesBranch.getType() == id::TRAJECTORY_VARIABLES);
    
    parameters.trajectorySize->setValueNotifyingHost (trajectoryVariablesBranch.getProperty (id::size));
    parameters.trajectoryRotation->setValueNotifyingHost (parameters.trajectoryRotation->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::rotation)));
    parameters.trajectoryTranslationX->setValueNotifyingHost (parameters.trajectoryTranslationX->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::translation_x)));
    parameters.trajectoryTranslationY->setValueNotifyingHost (parameters.trajectoryTranslationY->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::translation_y)));
         parameters.meanderanceScale->setValueNotifyingHost (parameters.meanderanceScale->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::meanderanceScale)));
                parameters.meanderanceSpeed->setValueNotifyingHost (parameters.meanderanceSpeed->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::meanderanceSpeed)));
    parameters.envelopeSize->setValueNotifyingHost (static_cast<float> (trajectoryVariablesBranch.getProperty (id::envelopeSize)));
    parameters.attack->setValueNotifyingHost (parameters.attack->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::attack)));
    parameters.decay->setValueNotifyingHost (parameters.decay->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::decay)));
    parameters.sustain->setValueNotifyingHost (parameters.sustain->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::sustain)));
    parameters.release->setValueNotifyingHost (parameters.release->convertTo0to1 (trajectoryVariablesBranch.getProperty (id::release)));

    auto trajectoryFeedbackBranch = trajectoryVariablesBranch.getChildWithName (id::FEEDBACK);
    jassert (trajectoryFeedbackBranch.getType() == id::FEEDBACK);

    parameters.feedbackTime->setValueNotifyingHost (parameters.feedbackTime->convertTo0to1 (trajectoryFeedbackBranch.getProperty (id::feedbackTime)));
    parameters.feedbackScalar->setValueNotifyingHost (parameters.feedbackScalar->convertTo0to1 (trajectoryFeedbackBranch.getProperty (id::feedbackScalar)));
    parameters.feedbackCompression->setValueNotifyingHost (parameters.feedbackCompression->convertTo0to1 (trajectoryFeedbackBranch.getProperty (id::feedbackCompression)));
    parameters.feedbackMix->setValueNotifyingHost (parameters.feedbackMix->convertTo0to1 (trajectoryFeedbackBranch.getProperty (id::feedbackMix)));

    auto terrainVariablesBranch = state.getChildWithName (id::TERRAIN_VARIABLES);
    parameters.terrainSaturation->setValueNotifyingHost (parameters.terrainSaturation->convertTo0to1 (terrainVariablesBranch.getProperty (id::terrainSaturation)));

    auto controlsBranch = state.getChildWithName (id::CONTROLS);
    parameters.filterFrequency->setValueNotifyingHost (parameters.filterFrequency->convertTo0to1 (controlsBranch.getProperty (id::filterFrequency)));
    parameters.filterResonance->setValueNotifyingHost (parameters.filterResonance->convertTo0to1 (controlsBranch.getProperty (id::filterResonance)));
    parameters.filterOnOff->setValueNotifyingHost (static_cast<float> (controlsBranch.getProperty (id::filterOnOff)));
    parameters.compressorThreshold->setValueNotifyingHost (parameters.compressorThreshold->convertTo0to1 (controlsBranch.getProperty (id::compressionThreshold)));
    parameters.compressorRatio->setValueNotifyingHost (parameters.compressorRatio->convertTo0to1 (controlsBranch.getProperty (id::compressionRatio)));
    parameters.outputLevel->setValueNotifyingHost (parameters.outputLevel->convertTo0to1 (controlsBranch.getProperty (id::outputLevel)));
}
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MainProcessor(); }