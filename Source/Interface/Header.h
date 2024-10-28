#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "LookAndFeel.h"
#include "PopUpWindow.h"
#include "../Identifiers.h"
#include "../Utility/PresetManager.h"

namespace ti{

class PresetComponent : public juce::Component
{
public:
    PresetComponent (PresetManager& pm)
      : viewport ("Preset Component Layout")
    {
        viewport.setViewedComponent (new PresetComponentLayout (this, pm));
        viewport.setScrollBarsShown (false, false);
        addAndMakeVisible (viewport);
    };
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
                    // TODO: rename preset
                    presetComponent->viewPresetMainComponent();
                };
            addAndMakeVisible (renameButton);
            
            cancelButton.onClick = [&](){ presetComponent->viewPresetMainComponent(); };
            addAndMakeVisible (cancelButton);
        }
        void paint (juce::Graphics& g)
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
        void paint (juce::Graphics& g)
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
    private:
        PresetComponent* presetComponent = nullptr;
        PresetManager& presetManager;
        juce::TextEditor nameEditor;
        juce::TextButton saveButton {"Save"};
        juce::TextButton cancelButton {"Cancel"};
    };
    struct PresetActionComponent : public juce::Component
    {
        PresetActionComponent (PresetComponent* pc, PresetManager& pm)
          : presetComponent (pc), 
            presetManager (pm)
        {
            saveButton.onClick = [&](){ presetComponent->viewSaveComponent(); };
            addAndMakeVisible (saveButton);
            
            renameButton.onClick = [&](){ presetComponent->viewRenameComponent(); };
            addAndMakeVisible (renameButton);

            deleteButton.onClick = [&]()
                {
                    // TODO: delete preset
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
        };
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
        PresetManager& presetManager;
        juce::TextButton saveButton   {"Save"};
        juce::TextButton renameButton {"Rename"};
        juce::TextButton deleteButton {"Delete"};
        juce::TextButton cancelButton {"Cancel"};
    };
    class PresetMainComponent : public juce::Component
    {
    public:
        PresetMainComponent (PresetComponent* pc, PresetManager& pm)
          : presetComponent (pc), 
            presetManager (pm)
        {
            presets.addItemList (presetManager.getPresetNames(), 1);
            addAndMakeVisible (presets);
            presetLabel.setJustificationType (juce::Justification::centred);
            addAndMakeVisible (presetLabel);
            presetActionButton.onClick = [&](){ presetComponent->viewActionComponent(); };
            addAndMakeVisible (presetActionButton);
    
            randomizeAmountSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 20, 20);
            addAndMakeVisible (randomizeAmountSlider);
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
            presets.addItemList (presetManager.getPresetNames(), 1);
        }
    private:
        PresetComponent* presetComponent = nullptr;
        PresetManager&   presetManager;
        juce::ComboBox presets;
        juce::Label presetLabel {"Preset", "Preset"};
        juce::TextButton presetActionButton {"+"};
    
        juce::Slider randomizeAmountSlider;
        juce::TextButton randomizeButton {"Randomize"};
    };  
    class PresetComponentLayout : public juce::Component
    {
    public:
        PresetComponentLayout (PresetComponent* pc, PresetManager& pm)
          : presetMainComponent   (pc, pm), 
            presetActionComponent (pc, pm), 
            presetSaveComponent   (pc, pm),
            presetRenameComponent (pc, pm)
        {
            addAndMakeVisible (presetMainComponent);
            addAndMakeVisible (presetActionComponent);
            addAndMakeVisible (presetSaveComponent);
            addAndMakeVisible (presetRenameComponent);
        };
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

class Header : public juce::Component
{
public:
    Header (PresetManager& pm)
      : presetComponent (pm)
    {
        addAndMakeVisible (presetComponent);
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

        auto presetRect = juce::Rectangle<int>().withHeight(b.getHeight())
                                                .withWidth (oneThird);
        presetComponent.setBounds (presetRect.withCentre (b.getCentre()));
                                                          
    }
private:
    PresetComponent presetComponent;
};
} // end namespace ti