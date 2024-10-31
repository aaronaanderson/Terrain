#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"
#include "../Identifiers.h"
#include "../Utility/PresetManager.h"

namespace ti{

class PresetComponent : public juce::Component
{
public:
    PresetComponent (PresetManager& pm, juce::ValueTree settingsBranch)
      : viewport ("Preset Component Layout")
    {
        viewport.setViewedComponent (new PresetComponentLayout (this, pm, settingsBranch));
        viewport.setScrollBarsShown (false, false);
        addAndMakeVisible (viewport);
    }
    void resized() override 
    {
        auto b = getLocalBounds();
        componentWidth = b.getWidth();
        viewport.setBounds (b);
        juce::Rectangle<int> layoutViewBounds = b.withWidth (componentWidth * 4);
        auto vc = viewport.getViewedComponent();
        vc->setBounds (layoutViewBounds);
    }
    void refreshList() 
    {
        auto pcl = dynamic_cast<PresetComponentLayout*> (viewport.getViewedComponent());
        pcl->refreshList();
    }
    void viewPresetMainComponent() { viewport.setViewPosition ({0, 0}); }
    void viewActionComponent()     { viewport.setViewPosition ({componentWidth,     0}); }
    void viewSaveComponent()       { viewport.setViewPosition ({componentWidth * 2, 0}); }
    void viewRenameComponent()     { viewport.setViewPosition ({componentWidth * 3, 0}); }
private:

    juce::Viewport viewport;
    int componentWidth;
    
    struct PresetRenameComponent : public juce::Component
    {
        PresetRenameComponent (PresetComponent* pc, PresetManager& pm)
          : presetComponent (pc),
            presetManager (pm)
        {
            addAndMakeVisible (nameEditor);

            renameButton.onClick = [&]()
                {
                    presetManager.renamePreset (presetManager.getCurrentPresetName(), 
                                                nameEditor.getText());
                    presetComponent->refreshList();
                    presetComponent->viewPresetMainComponent();
                };
            addAndMakeVisible (renameButton);
            
            cancelButton.onClick = [&](){ presetComponent->viewPresetMainComponent(); };
            addAndMakeVisible (cancelButton);
        }
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colours::black);
            auto b = getLocalBounds();
            g.drawRect (b, 2);            
        }
        void resized() override 
        {
            auto b = getLocalBounds();
            auto halfBounds = b.getWidth() / 2;

            nameEditor.setBounds (b.removeFromLeft (halfBounds).reduced (10));
            renameButton.setBounds (b.removeFromLeft (halfBounds / 2).reduced (4));
            cancelButton.setBounds (b.reduced (4));
        }
        void setText (juce::String text) { nameEditor.setText (text); }
    private:
        PresetComponent* presetComponent = nullptr;
        PresetManager& presetManager;
        juce::TextEditor nameEditor;
        juce::TextButton renameButton {"Rename"};
        juce::TextButton cancelButton {"Cancel"};        
    };
    struct PresetSaveComponent : public juce::Component
    {
        PresetSaveComponent (PresetComponent* pc, PresetManager& pm)
          : presetComponent (pc),
            presetManager (pm)
        {
            addAndMakeVisible (nameEditor);
            saveButton.onClick = [&]()
                {
                    presetManager.savePreset (nameEditor.getText());
                    presetComponent->refreshList();
                    presetComponent->viewPresetMainComponent();
                };
            addAndMakeVisible (saveButton);
            
            cancelButton.onClick = [&](){ presetComponent->viewPresetMainComponent(); };
            addAndMakeVisible (cancelButton);
        }
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colours::black);
            auto b = getLocalBounds();
            g.drawRect (b, 2);            
        }
        void resized() override 
        {
            auto b = getLocalBounds();
            auto halfBounds = b.getWidth() / 2;

            nameEditor.setBounds (b.removeFromLeft (halfBounds).reduced (10));
            saveButton.setBounds (b.removeFromLeft (halfBounds / 2).reduced (4));
            cancelButton.setBounds (b.reduced (4));
        }
        void setText (juce::String text) { nameEditor.setText (text); }
    private:
        PresetComponent* presetComponent = nullptr;
        PresetManager& presetManager;
        juce::TextEditor nameEditor;
        juce::TextButton saveButton {"Save"};
        juce::TextButton cancelButton {"Cancel"};
    };
    struct PresetMainComponent : public juce::Component
    {
        PresetMainComponent (PresetComponent* pc, PresetManager& pm, juce::ValueTree settingsBranch)
          : presetComponent (pc), 
            presetManager (pm), 
            settings (settingsBranch)
        {
            jassert (settingsBranch.getType() == id::PRESET_SETTINGS);
            refreshList();
            presets.onChange = [&](){ presetManager.loadPreset (presets.getItemText (presets.getSelectedItemIndex())); };
            addAndMakeVisible (presets);
            presetLabel.setJustificationType (juce::Justification::centred);
            addAndMakeVisible (presetLabel);
            presetActionButton.onClick = [&](){ presetComponent->viewActionComponent(); };
            addAndMakeVisible (presetActionButton);
    
            randomizeAmountSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
            randomizeAmountSlider.setRange ({0.0, 1.0}, 0.0);
            randomizeAmountSlider.setValue (settings.getProperty (id::presetRandomizationScale), 
                                            juce::dontSendNotification);
            randomizeAmountSlider.onValueChange = [&]() {settings.setProperty (id::presetRandomizationScale,
                                                                               randomizeAmountSlider.getValue(), 
                                                                               nullptr); };
            addAndMakeVisible (randomizeAmountSlider);
            randomizeButton.onClick = [&]() { presetManager.randomize(); };
            addAndMakeVisible (randomizeButton);
        }  
        void paint (juce::Graphics& g) override 
        {
            g.setColour (juce::Colours::black);
            auto b = getLocalBounds();
            g.drawRect (b, 2);
        }
        void resized() override
        {
            auto b = getLocalBounds().reduced (2);
            auto twoThirds = b.getWidth() * 2 / 3;
    
            auto p = b.removeFromLeft (twoThirds);
            presetLabel.setBounds (p.removeFromTop (p.getHeight() / 2));
            auto sevenEighths = p.getWidth() * 7 / 8;
            presets.setBounds (p.removeFromLeft (sevenEighths));
            presetActionButton.setBounds (p);
    
            randomizeButton.setBounds (b.removeFromTop (b.getHeight() / 2));
            randomizeAmountSlider.setBounds (b);
        }
        void refreshList()
        {
            presets.clear();
            auto names = presetManager.getPresetNames();
            presets.addItemList (names, 1);
            int currentPresetIndex = 0;
            for (int i = 0; i < names.size(); i++)
            {
                if (names[i] == presetManager.getCurrentPresetName())
                {
                    currentPresetIndex = i;
                    break;
                }    
            }
            presets.setSelectedItemIndex (currentPresetIndex, juce::NotificationType::dontSendNotification);
        }
        juce::String getCurrentText() { return presets.getText(); }
    private:
        PresetComponent* presetComponent = nullptr;
        PresetManager&   presetManager;
        juce::ValueTree settings;
        juce::ComboBox presets;
        juce::Label presetLabel {"Preset", "Preset"};
        juce::TextButton presetActionButton {"+"};
    
        juce::Slider randomizeAmountSlider;
        juce::TextButton randomizeButton {"Randomize"};
    };  
    struct PresetActionComponent : public juce::Component
    {
        PresetActionComponent (PresetComponent* pc, 
                               PresetSaveComponent* psc,
                               PresetRenameComponent* prc, 
                               PresetMainComponent* pmc, 
                               PresetManager& pm)
          : presetComponent (pc), 
            presetSaveComponent (psc),
            presetRenameComponent (prc),
            presetMainComponent (pmc),
            presetManager (pm)
        {
            saveButton.onClick = [&]()
            { 
                presetSaveComponent->setText (presetMainComponent->getCurrentText());
                presetComponent->viewSaveComponent(); 
            };
            addAndMakeVisible (saveButton);
            
            renameButton.onClick = [&]()
            { 
                presetRenameComponent->setText (presetMainComponent->getCurrentText());
                presetComponent->viewRenameComponent(); 
            };
            addAndMakeVisible (renameButton);

            deleteButton.onClick = [&]()
                {
                    presetManager.deletePreset (presetManager.getCurrentPresetName());
                    presetComponent->refreshList();
                    presetComponent->viewPresetMainComponent();
                };
            addAndMakeVisible (deleteButton);

            cancelButton.onClick = [&](){ presetComponent->viewPresetMainComponent(); };
            addAndMakeVisible (cancelButton);
        }
        void paint(juce::Graphics& g) override
        {
            g.setColour (juce::Colours::black);
            auto b = getLocalBounds();
            g.drawRect (b, 2);
        }
        void resized() override 
        {
            auto b = getLocalBounds();
            auto oneFourth = b.getWidth() / 4;
    
            saveButton.setBounds (b.removeFromLeft (oneFourth).reduced (4));
            renameButton.setBounds (b.removeFromLeft (oneFourth).reduced (4));
            deleteButton.setBounds (b.removeFromLeft (oneFourth).reduced (4));
            cancelButton.setBounds (b.removeFromLeft (oneFourth).reduced (4));
        }
    private:
        PresetComponent* presetComponent = nullptr;
        PresetSaveComponent* presetSaveComponent = nullptr;
        PresetRenameComponent* presetRenameComponent = nullptr;
        PresetMainComponent* presetMainComponent = nullptr;
        PresetManager& presetManager;
        juce::TextButton saveButton   {"Save"};
        juce::TextButton renameButton {"Rename"};
        juce::TextButton deleteButton {"Delete"};
        juce::TextButton cancelButton {"Cancel"};
    };
    struct PresetComponentLayout : public juce::Component
    {
        PresetComponentLayout (PresetComponent* pc, 
                               PresetManager& pm, 
                               juce::ValueTree settingsBranch)
          : presetMainComponent   (pc, pm, settingsBranch), 
            presetActionComponent (pc, &presetSaveComponent, &presetRenameComponent, &presetMainComponent, pm), 
            presetSaveComponent   (pc, pm),
            presetRenameComponent (pc, pm)
        {
            addAndMakeVisible (presetMainComponent);
            addAndMakeVisible (presetActionComponent);
            addAndMakeVisible (presetSaveComponent);
            addAndMakeVisible (presetRenameComponent);
        }
        void resized() override
        {
            auto b = getLocalBounds();
            auto oneFourth = b.getWidth() / 4;
    
            presetMainComponent.setBounds   (b.removeFromLeft (oneFourth));
            presetActionComponent.setBounds (b.removeFromLeft (oneFourth));
            presetSaveComponent.setBounds   (b.removeFromLeft (oneFourth));
            presetRenameComponent.setBounds (b);
        }
        void refreshList() { presetMainComponent.refreshList(); }
    private:
        PresetMainComponent   presetMainComponent;
        PresetActionComponent presetActionComponent;
        PresetSaveComponent   presetSaveComponent;
        PresetRenameComponent presetRenameComponent;
    };
};

class PitchBendComponent : public juce::Component
{
public:
    PitchBendComponent (juce::ValueTree settingsBranch)
      : settings (settingsBranch)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        slider.setRange ({0.0, 12.0}, 0.0);
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, 60, 20);
        slider.setValue (settings.getProperty (id::pitchBendRange), juce::dontSendNotification);
        slider.onValueChange = [&]() { settings.setProperty (id::pitchBendRange, 
                                                             slider.getValue(), 
                                                             nullptr); };
        addAndMakeVisible (slider);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (b.getHeight() / 2));
        slider.setBounds (b);
    }
private:
    juce::ValueTree settings;
    juce::Label label {"PitchBend", "Pitch Bend (Semitones)"};
    juce::Slider slider;
};

struct ConnectionIndicator : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.fillAll (laf->getBackgroundColour());
        
        g.setColour (juce::Colours::black);
        g.fillEllipse (getLocalBounds().toFloat());

        g.setColour (laf->getBaseColour());
        g.fillEllipse (getLocalBounds().toFloat().reduced (2));

        if (connected)
        {
            g.setColour (laf->getAccentColour());
            g.fillEllipse (getLocalBounds().reduced (4).toFloat());
        }
    }
    void setConnected (bool isConnected) { connected = isConnected; }
private:
    bool connected  = true;
};
class MTSComponent : public juce::Component
{
public:
    MTSComponent (juce::ValueTree settingsBranch)
      : settings (settingsBranch)
    {
        connectionIndicator.setConnected (settings.getProperty (id::mtsConnection));
        addAndMakeVisible (connectionIndicator);

        addAndMakeVisible (noteOnOrContinuousLabel);
        noteOnOrContinuous.onClick = [&]()
            {
                if (noteOnOrContinuous.getToggleState())
                {
                    settings.setProperty (id::noteOnOrContinuous, true, nullptr);
                    noteOnOrContinuousLabel.setText ("Continuous", juce::dontSendNotification);
                }
                else 
                {
                    settings.setProperty (id::noteOnOrContinuous, false, nullptr);
                    noteOnOrContinuousLabel.setText ("Note On", juce::dontSendNotification);
                }
            };
        noteOnOrContinuous.setToggleState (settings.getProperty (id::noteOnOrContinuous), 
                                           juce::sendNotification);
        addAndMakeVisible (noteOnOrContinuous);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        connectionIndicator.setBounds (b.removeFromLeft (40).reduced (10));
        noteOnOrContinuous.setBounds (b.removeFromLeft (30).reduced (2));
        noteOnOrContinuousLabel.setBounds (b.removeFromLeft (100));
    }
private:
    juce::ValueTree settings;
    ConnectionIndicator connectionIndicator;

    juce::Label noteOnOrContinuousLabel;
    juce::ToggleButton noteOnOrContinuous;
};

class Header : public juce::Component
{
public:
    Header (PresetManager& pm, juce::ValueTree settingsBranch)
      : mtsComponent (settingsBranch),
        presetComponent (pm, settingsBranch), 
        pitchBendComponent (settingsBranch)
    {
        addAndMakeVisible (mtsComponent);
        addAndMakeVisible (presetComponent);
        addAndMakeVisible (pitchBendComponent);
    }
    void paint (juce::Graphics& g) override 
    {
        g.setColour (juce::Colours::black);
        auto b = getLocalBounds();
        g.drawRect (b, 2);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        auto oneThird = b.getWidth() / 3;

        mtsComponent.setBounds (b.removeFromLeft (oneThird));
        presetComponent.setBounds (b.removeFromLeft (oneThird));

        b.removeFromLeft (b.getWidth() / 2);
        pitchBendComponent.setBounds (b);                                                  
    }
private:
    MTSComponent mtsComponent;
    PresetComponent presetComponent;
    PitchBendComponent pitchBendComponent;
};
} // end namespace ti