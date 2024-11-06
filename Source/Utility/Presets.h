#pragma once

#include <BinaryData.h>

struct PresetSaver
{
    static void saveBinaryToFile (const void* data, int sizeInBytes, juce::File presetsFolder, const juce::String name)
    {
        juce::String s = juce::String::createStringFromData (data, sizeInBytes);
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (s));
        auto file = presetsFolder.getChildFile (name + ".xml");
        if (!file.existsAsFile()) file.setCreationTime (juce::Time::getCurrentTime());
        xml->writeTo (file);
    }
    static void movePresetsToDisk()
    {
        auto presetFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
#ifdef JUCE_MAC
    	presetFolder = presetFolder.getChildFile("Audio").getChildFile("Presets");
#endif
    	presetFolder = presetFolder.getChildFile("Aaron Anderson").getChildFile("Terrain"); // "Imogen" is the name of my plugin
        auto result = presetFolder.createDirectory();
        
        saveBinaryToFile (BinaryData::Basic_xml,     BinaryData::Basic_xmlSize,     presetFolder, "Basic");
        saveBinaryToFile (BinaryData::ButterFly_xml, BinaryData::ButterFly_xmlSize, presetFolder, "ButterFly");
        saveBinaryToFile (BinaryData::Echo_xml,      BinaryData::Echo_xmlSize,      presetFolder, "Echo");
        saveBinaryToFile (BinaryData::Fire_xml,      BinaryData::Fire_xmlSize,      presetFolder, "Fire");
        saveBinaryToFile (BinaryData::Space_xml,     BinaryData::Space_xmlSize,     presetFolder, "Space");
    }
};