#include "MainProcessor.h"
#include "MainEditor.h"

#include "Utility/DefaultTreeGenerator.h"
#include "Utility/VersionType.h"

//==============================================================================
MainProcessor::MainProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState (*this, &undoManager, id::TERRAIN_SYNTH, createParameterLayout()),
       parameters (valueTreeState)
{
    valueTreeState.state.addChild (SettingsTree::create(), -1, nullptr);
    presetManager = std::make_unique<PresetManager> (this, valueTreeState.state);
    synthesizer = std::make_unique<tp::WaveTerrainSynthesizer> (parameters, valueTreeState.state.getChildWithName (id::PRESET_SETTINGS));
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
    allocateMaxSamplesPerBlock (maxSamplesPerBlock);

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
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
void MainProcessor::setStateInformation (const void* data, int sizeInBytes)
{ 
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (valueTreeState.state.getType()))
        {
            auto version = Version (id::version.toString());
            if (version.major < 1) return;
            if (version.minor < 1) return;

            auto newState = juce::ValueTree::fromXml (*xmlState);
            auto verifiedSettingsBranch = verifiedSettings (newState.getChildWithName (id::PRESET_SETTINGS));
            
            for (int i =  newState.getNumChildren() - 1; i >= 0; i--)
            {
                if (newState.getChild (i).getType() == id::PRESET_SETTINGS)
                {
                    // std::cout << i << std::endl;
                    // std::cout << newState.getChild(i).toXmlString() << std::endl;
                    newState.removeChild (i, nullptr);
                }
            }

            newState.addChild (verifiedSettingsBranch, -1, nullptr);
            valueTreeState.replaceState (newState);
            presetManager->setState (valueTreeState.state);
            synthesizer->setState (valueTreeState.state.getChildWithName (id::PRESET_SETTINGS));
        }
    }
}
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MainProcessor(); }
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MainProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range;
    //======================================Trajectory Parameters
    layout.add (std::make_unique<tp::ChoiceParameter> ("Current Trajectory", 
                                                        juce::StringArray {"Ellipse", 
                                                            "Superellipse", 
                                                            "Limacon", 
                                                            "Butterfly", 
                                                            "Scarabaeus", 
                                                            "Squarcle", 
                                                            "Bicorn", 
                                                            "Cornoid", 
                                                            "Epitrochoid 3", "Epitrochoid 5", "Epitrochoid 7", 
                                                            "Hypocycloid 3", "Hypocycloid 5", "Hypocycloid 7", 
                                                            "Gear Curve 3", "Gear Curve 5", "Gear Curve 7"}, 
                                                        "", 
                                                        0));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Trajectory Mod A", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Trajectory Mod B", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Trajectory Mod C", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Trajectory Mod D", 0.5f));
    
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Size", 0.5f));
    range = {0.0f, juce::MathConstants<float>::twoPi};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Rotation", 
                                                            range, 
                                                            0.0f));
    range = {-1.0f, 1.0f};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Translation X", 
                                                            range, 
                                                            0.0f));
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Translation Y", 
                                                            range,
                                                            0.0f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Meanderance Scale", 
                                                                0.3f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Meanderance Speed", 
                                                                0.0f));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID {"EnvelopeSize", 1}, "Envelope Size", true));
    range = juce::NormalisableRange<float> (2.0f, 5000.0f); range.setSkewForCentre (500.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Attack", 
                                                            range, 
                                                            200.0f));
    range = juce::NormalisableRange<float> (2.0f, 5000.0f); range.setSkewForCentre (500.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Decay", 
                                                            range, 
                                                            80.0f));
    range = juce::NormalisableRange<float> (-48.0f, 0.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Sustain", 
                                                            range, 
                                                            -6.0f));
    range = juce::NormalisableRange<float> (2.0f, 5000.0f); range.setSkewForCentre (500.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Release", 
                                                            range, 
                                                            800.0f));
    //=======Feedback
    range = juce::NormalisableRange<float> (0.0f, 2000.0f); range.setSkewForCentre (250.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Feedback Time", 
                                                            range,
                                                            200.0f));
    range = juce::NormalisableRange<float> (0.0f, 0.9999f); range.setSkewForCentre (0.8f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Feedback", 
                                                            range,
                                                            0.8f));
    range = {1.0f, 20.0f};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Feedback Compression", 
                                                            range,
                                                            10.0f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Feedback Mix",
                                                                0.0f));

    //=======================================Terrain Parameters
    layout.add (std::make_unique<tp::ChoiceParameter> ("Current Terrain", 
                                                       juce::StringArray {"Sinusoidal", 
                                                                          "System 1", 
                                                                          "System 2", 
                                                                          "System 3", 
                                                                          "System 9", 
                                                                          "System 11",
                                                                          "System 12", 
                                                                          "System 14", 
                                                                          "System 15"}, 
                                                       "",  
                                                       0));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Terrain Mod A", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Terrain Mod B", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Terrain Mod C", 0.5f));
    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Terrain Mod D", 0.5f));

    range = juce::NormalisableRange<float> (1.0f, 16.0f); range.setSkewForCentre (4.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Terrain Saturation", 
                                                            range,
                                                            1.0f));

    layout.add (std::make_unique<tp::NormalizedFloatParameter> ("Filter Resonance", 0.5f));
    range = juce::NormalisableRange<float> (20.0f, 10000.0f); range.setSkewForCentre (500.0f);
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Filter Frequency", 
                                                            range,
                                                            800.0f));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID {"FilterOnOff", 1}, "Filter Bypass", false));

    range = {-24.0f, 0.0f};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Compressor Threshold", 
                                                            range,
                                                            -4.0f));
    range = {1.0f, 12.0f};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Compressor Ratio", 
                                                            range,
                                                            1.0f));
    range = {-60.0f, 6.0f};
    layout.add (std::make_unique<tp::RangedFloatParameter> ("Output Level", 
                                                            range, 
                                                            0.0f));

    return layout;
} 
void MainProcessor::allocateMaxSamplesPerBlock (int maxSamples)
{
    auto settingsTree = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS);
    auto overSamplingFactor = static_cast<int> (settingsTree.getProperty (id::oversampling));
    synthesizer->allocate (maxSamples * static_cast<int> (std::pow (2, overSamplingFactor)));
    overSampler = std::make_unique<juce::dsp::Oversampling<float>> (1, 
                                                                    overSamplingFactor, 
                                                                    juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    overSampler->initProcessing (static_cast<size_t> (maxSamples));
}
void MainProcessor::prepareOversampling (int bufferSize)
{
    auto presetsTree = valueTreeState.state.getChildWithName (id::PRESET_SETTINGS);
    auto overSamplingFactor = static_cast<int> (presetsTree.getProperty (id::oversampling));
    
    if (overSamplingFactor == storedFactor && bufferSize == storedBufferSize) return;
    //=============Very bad to allocate here, threaded solution instead of blocking should come in the future
    //===The situation only arises if oversampling factor has changed
    if (overSamplingFactor != storedFactor)
    {
        synthesizer->allocate (maxSamplesPerBlock * static_cast<int> (std::pow (2, overSamplingFactor)));
        overSampler = std::make_unique<juce::dsp::Oversampling<float>> (1, 
                                                                        overSamplingFactor, 
                                                                        juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
        overSampler->initProcessing (static_cast<size_t> (maxSamplesPerBlock));
        
        synthesizer->prepareToPlay (sampleRate * std::pow (2, overSamplingFactor), 
                                    bufferSize * static_cast<int> (std::pow (2, overSamplingFactor)));
        renderBuffer.setSize (1, bufferSize, false, false, true); // Don't re-allocate; maxBufferSize is set in prepareToPlay
        renderBuffer.clear();
        
        storedFactor = overSamplingFactor;
        storedBufferSize = bufferSize;
    }
    
    if (bufferSize != storedBufferSize)
    {
        synthesizer->prepareToPlay (sampleRate * std::pow (2, overSamplingFactor), 
                                    bufferSize * static_cast<int> (std::pow (2, overSamplingFactor)));
        renderBuffer.setSize (1, bufferSize, false, false, true); // Don't re-allocate; maxBufferSize is set in prepareToPlay
        renderBuffer.clear();
        storedBufferSize = bufferSize;
    }
}
juce::ValueTree MainProcessor::verifiedSettings (juce::ValueTree settings)
{  
    if (settings == juce::ValueTree()) return SettingsTree::create();
    if (!settings.hasProperty (id::noteOnOrContinuous))
        settings.setProperty (id::noteOnOrContinuous, SettingsTree::DefaultSettings::noteOnOrContinuous, nullptr);
    if (!settings.hasProperty (id::pitchBendRange))
        settings.setProperty (id::pitchBendRange, SettingsTree::DefaultSettings::pitchBendRange, nullptr);
    if (!settings.hasProperty (id::presetRandomizationScale))
        settings.setProperty (id::presetRandomizationScale, SettingsTree::DefaultSettings::presetRandomizationScale, nullptr);

    return settings;
}