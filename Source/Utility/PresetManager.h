#pragma once 

#include <juce_audio_processors/juce_audio_processors.h>

class PresetManager
{
public:
    PresetManager (juce::AudioProcessor* ap, juce::ValueTree& tree)
      : audioProcessor (ap), 
        state (tree)
    {

    }

    void savePreset (juce::String presetName)
    {
        state.setProperty (id::presetName, presetName, nullptr);
        auto xml = state.createXml();
        auto file = getPresetFolder().getChildFile (presetName + ".xml");
        if (!file.existsAsFile()) file.setCreationTime (juce::Time::getCurrentTime());

        xml->writeTo (file);
    }
    void loadPreset (juce::String presetName)
    {
        auto file = getPresetFolder().getChildFile (presetName + ".xml");
        std::cout << file.getFullPathName() << std::endl;
        if (file.existsAsFile())
        {
            auto xml = juce::XmlDocument::parse (file);
            juce::MemoryBlock block;
            audioProcessor->copyXmlToBinary (*xml.get(), block);
            audioProcessor->setStateInformation (block.getData(), (int)block.getSize());
        }
    }
    void deletePreset (juce::String presetName)
    {
        auto file = getPresetFolder().getChildFile (presetName + ".xml");
        if (file.existsAsFile())
            file.deleteFile();
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
    juce::String getCurrentPresetName(){ return state.getProperty (id::presetName).toString(); }

private:
    juce::AudioProcessor* audioProcessor = nullptr;
    juce::ValueTree& state;

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