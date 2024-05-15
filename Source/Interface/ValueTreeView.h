#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>

class ValueTreeView : public juce::Component, 
                      private juce::ValueTree::Listener 
{
public:
    ValueTreeView (juce::ValueTree s)  :
        state (s)
    {
        state.addListener(this);
        textBox.setReadOnly(true);
        textBox.setMultiLine (true);
        textBox.setScrollbarsShown (true);
        // textBox.setScrollbarThickness (8);
        updateText();
        addAndMakeVisible(&textBox);
    }

    void resized() override
    {
        textBox.setBounds(getLocalBounds());
    }

    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override
    {juce::ignoreUnused (treeWhosePropertyHasChanged, property); updateText();}                                        

    void valueTreeChildAdded (juce::ValueTree& parentTree,
                              juce::ValueTree& childWhichHasBeenAdded) override
    {juce::ignoreUnused (parentTree, childWhichHasBeenAdded); updateText();}                                   

    void valueTreeChildRemoved (juce::ValueTree& parentTree,
                                juce::ValueTree& childWhichHasBeenRemoved,
                                int indexFromWhichChildWasRemoved) override
    {juce::ignoreUnused (parentTree, childWhichHasBeenRemoved, indexFromWhichChildWasRemoved); updateText();}                                   

    void valueTreeChildOrderChanged (juce::ValueTree& parentTreeWhoseChildrenHaveMoved,
                                     int oldIndex, int newIndex) override
    {juce::ignoreUnused (parentTreeWhoseChildrenHaveMoved, oldIndex, newIndex); updateText();}                                        

    void valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged) override
    {juce::ignoreUnused (treeWhoseParentHasChanged); updateText();}

    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override
    {juce::ignoreUnused (treeWhichHasBeenChanged); updateText();}

private:
    juce::ValueTree state;
    juce::TextEditor textBox;
    void updateText()
    {
        textBox.setText(state.toXmlString());
    }
};

class ValueTreeViewWindow : public juce::DocumentWindow
{
public:
    ValueTreeViewWindow (juce::ValueTree state) 
      : juce::DocumentWindow ("Value Tree View", 
                              juce::Colours::black, 
                              juce::DocumentWindow::allButtons ), 
        valueTreeView (state) 
    {
        setContentNonOwned (&valueTreeView, true);
    }

private:
    ValueTreeView valueTreeView;
};