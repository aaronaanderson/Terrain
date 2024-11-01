#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"
#include "Panel.h"
#include "../Utility/Identifiers.h"
#include "../Utility/PresetManager.h"

namespace ti{

class PresetComponent : public Panel
{
public:
    PresetComponent (PresetManager& pm, juce::ValueTree settingsBranch)
      : Panel ("Presets"),
        viewport ("Preset Component Layout")
    {
        viewport.setViewedComponent (new PresetComponentLayout (this, pm, settingsBranch));
        viewport.setScrollBarsShown (false, false);
        addAndMakeVisible (viewport);
    }
    void resized() override 
    {
        Panel::resized();
        auto b = getAdjustedBounds();
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
        void resized() override
        {
            auto b = getLocalBounds().reduced (2);
            auto twoThirds = b.getWidth() * 2 / 3;
    
            auto p = b.removeFromLeft (twoThirds);
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
class PitchBendComponent : public Panel
{
public:
    PitchBendComponent (juce::ValueTree settingsBranch)
      : Panel ("Pitch Bend (Semitones)"),
        settings (settingsBranch)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
        slider.setDoubleClickReturnValue (true, 2.0);
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
        Panel::resized();
        auto b = getAdjustedBounds();
        slider.setBounds (b);
    }
private:
    juce::ValueTree settings;
    juce::Slider slider;
};
struct ConnectionIndicator : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        int shortestSide = b.getWidth() < b.getHeight() ? b.getWidth() : b.getHeight();
        auto adjustedBounds = juce::Rectangle<int> (shortestSide, shortestSide);
        adjustedBounds = adjustedBounds.withCentre (b.getCentre());

        auto* laf = dynamic_cast<TerrainLookAndFeel*> (&getLookAndFeel());
        g.fillAll (laf->getBackgroundColour());
        
        g.setColour (juce::Colours::black);
        g.fillEllipse (adjustedBounds.toFloat());

        g.setColour (laf->getBaseColour());
        g.fillEllipse (adjustedBounds.toFloat().reduced (2));

        if (connected)
        {
            g.setColour (laf->getAccentColour());
            g.fillEllipse (adjustedBounds.reduced (4).toFloat());
        }
    }
    void setConnected (bool isConnected) 
    { 
        connected = isConnected; 
        repaint();
    }
private:
    bool connected  = true;
};
class MTSComponent : public Panel, 
                     private juce::ValueTree::Listener
{
public:
    MTSComponent (juce::ValueTree settingsBranch, 
                  juce::ValueTree ephemeralBranch)
      : Panel ("MTS-ESP"),
        settings (settingsBranch),
        ephemeralState (ephemeralBranch)
    {
        jassert (settings.getType() == id::PRESET_SETTINGS);
        jassert (ephemeralState.getType() == id::EPHEMERAL_STATE);
        
        ephemeralState.addListener (this);

        connectionIndicator.setConnected (ephemeralState.getProperty (id::tuningSystemConnected));
        addAndMakeVisible (connectionIndicator);
        addAndMakeVisible (connectionStatusLabel);
        
        noteOnOrContinuousLabel.setJustificationType (juce::Justification::left);
        addAndMakeVisible (noteOnOrContinuousLabel);
        noteOnOrContinuous.onStateChange = [&]()
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
        noteOnOrContinuous.onStateChange();
        addAndMakeVisible (noteOnOrContinuous);

        currentTuningSystemLabel.setJustificationType (juce::Justification::centred);  
        addAndMakeVisible (currentTuningSystemLabel);
        currentTuningSystem.setJustificationType (juce::Justification::centred);
        currentTuningSystem.setText (ephemeralState.getProperty (id::tuningSystemName).toString(), 
                                     juce::dontSendNotification);
        addAndMakeVisible (currentTuningSystem);
    }
    ~MTSComponent() override { ephemeralState.removeListener (this); }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        juce::Point<float> scalar = {b.getWidth() / 400.0f, b.getHeight() / 40.0f};
        
        b.removeFromLeft (static_cast<int> (10 * scalar.x));
        connectionIndicator.setBounds     (b.removeFromLeft (static_cast<int> (juce::jmax (20 * scalar.x, 22.0f))).reduced (2));
        connectionStatusLabel.setBounds   (b.removeFromLeft (static_cast<int> (60 * scalar.x)) );
        noteOnOrContinuous.setBounds      (b.removeFromLeft (22));
        noteOnOrContinuousLabel.setBounds (b.removeFromLeft (static_cast<int> (60 * scalar.x)));

        currentTuningSystemLabel.setBounds (b.removeFromTop (b.getHeight() / 2));
        currentTuningSystem.setBounds (b);
    }
private:
    juce::ValueTree settings;
    juce::ValueTree ephemeralState;
    ConnectionIndicator connectionIndicator;
    juce::Label connectionStatusLabel { "connectionStatus", "Connection Status"};
    juce::Label noteOnOrContinuousLabel;
    juce::ToggleButton noteOnOrContinuous;
    juce::Label currentTuningSystemLabel {"CTS", "Current Tuning System"};
    juce::Label currentTuningSystem;

    void valueTreePropertyChanged (juce::ValueTree& tree,
                                   const juce::Identifier& property) override
    {
        if (tree.getType() == id::EPHEMERAL_STATE)
        {
            if (property == id::tuningSystemConnected)
                connectionIndicator.setConnected (tree.getProperty (property));
            else if (property == id::tuningSystemName)
                currentTuningSystem.setText (tree.getProperty (property).toString(), juce::dontSendNotification);
        }
    }
};
class MPEComponent : public Panel
{
public:
    MPEComponent (juce::ValueTree settingsBranch)
      : Panel ("MPE"),
        settings (settingsBranch)
    {
        mpeEnableToggle.onStateChange = [&]()
            {
                settings.setProperty (id::mpeEnabled, 
                                      mpeEnableToggle.getToggleState(), 
                                      nullptr);
            };
        mpeEnableToggle.setToggleState (settings.getProperty (id::mpeEnabled), 
                                        juce::sendNotification);
        mpeEnableToggle.setButtonText ("MPE Enabled");
        addAndMakeVisible (mpeEnableToggle);
    }
    void resized() override
    {
        Panel::resized();
        auto b = getAdjustedBounds();
        juce::Rectangle<int> r = {0, 0, 22, 80};
        mpeEnableToggle.setBounds (r.withCentre (b.getCentre()));
    }
private:
    juce::ValueTree settings;
    juce::ToggleButton mpeEnableToggle;
};
class Header : public juce::Component
{
public:
    Header (PresetManager& pm, 
            juce::ValueTree settingsBranch,
            juce::ValueTree ephemeralState)
      : mtsComponent (settingsBranch, ephemeralState),
        presetComponent (pm, settingsBranch), 
        mpeComponent (settingsBranch),
        pitchBendComponent (settingsBranch)
    {
        addAndMakeVisible (mtsComponent);
        addAndMakeVisible (presetComponent);
        addAndMakeVisible (mpeComponent);
        addAndMakeVisible (pitchBendComponent);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        auto oneThird = b.getWidth() / 3;

        mtsComponent.setBounds (b.removeFromLeft (oneThird));
        presetComponent.setBounds (b.removeFromLeft (oneThird));

        auto remainingThird = oneThird / 3;
        mpeComponent.setBounds (b.removeFromLeft (remainingThird));
        pitchBendComponent.setBounds (b);                                                 
    }
private:
    MTSComponent mtsComponent;
    PresetComponent presetComponent;
    MPEComponent mpeComponent;
    PitchBendComponent pitchBendComponent;
};
} // end namespace ti