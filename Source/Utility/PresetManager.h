#pragma once 

#include <juce_audio_processors/juce_audio_processors.h>

class PresetManager
{
public:
    PresetManager (juce::AudioProcessor* ap, juce::ValueTree tree)
      : audioProcessor (ap), 
        state (tree)
    {

    }

    void savePreset (juce::String presetName)
    {
        auto xml = state.createXml();
        auto file = getPresetFolder().getChildFile (presetName + ".xml");
        if (!file.existsAsFile()) file.setCreationTime (juce::Time::getCurrentTime());

        xml->writeTo (file);
    }
    void loadPreset (juce::String presetName)
    {

    }
    void deletePreset (juce::String presetName)
    {

    }
    void renamePreset (juce::String oldName, juce::String newName)
    {
        
    }
    juce::Array<juce::String> getPresetNames()
    {
        juce::Array<juce::String> l;
        auto files = getPresetFolder().findChildFiles (juce::File::TypesOfFileToFind::findFiles, false, "*.xml");
        for (auto f : files) { l.add (f.getFileNameWithoutExtension()); }
            
        return l;
    }

private:
    juce::AudioProcessor* audioProcessor = nullptr;
    juce::ValueTree state;

    juce::File getPresetFolder()
    {
	    auto presetFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
	
#ifdef JUCE_MAC
	    presetFolder = presetFolder.getChildFile("Audio").getChildFile("Presets");
#endif
	
	    presetFolder = presetFolder.getChildFile("Aaron Anderson").getChildFile("Terrain"); // "Imogen" is the name of my plugin
	    auto result = presetFolder.createDirectory();

	    return presetFolder;
    }
};