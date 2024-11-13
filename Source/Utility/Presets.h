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
        
        saveBinaryToFile (BinaryData::Basic_xml,      BinaryData::Basic_xmlSize,      presetFolder, "Basic");
        saveBinaryToFile (BinaryData::PatternRun_xml, BinaryData::PatternRun_xmlSize, presetFolder, "Pattern Run");
        saveBinaryToFile (BinaryData::Plunky_xml,     BinaryData::Plunky_xmlSize,     presetFolder, "Plunky");


        saveBinaryToFile (BinaryData::MPEBasic_xml,     BinaryData::MPEBasic_xmlSize,     presetFolder, "MPE-Basic");
        saveBinaryToFile (BinaryData::MPEButterFly_xml, BinaryData::MPEButterFly_xmlSize, presetFolder, "MPE-ButterFly");
        saveBinaryToFile (BinaryData::MPEEcho_xml,      BinaryData::MPEEcho_xmlSize,      presetFolder, "MPE-Echo");
        saveBinaryToFile (BinaryData::MPEFire_xml,      BinaryData::MPEFire_xmlSize,      presetFolder, "MPE-Fire");
        saveBinaryToFile (BinaryData::MPESpace_xml,     BinaryData::MPESpace_xmlSize,     presetFolder, "MPE-Space");
    }
};