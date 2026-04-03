#include "Toolbar.h"
#include "UI/LookAndFeel/ChipForgeLookAndFeel.h"

Toolbar::Toolbar()
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(trackerTab);
    addAndMakeVisible(mixerTab);
    addAndMakeVisible(arrangeTab);
    addAndMakeVisible(bpmLabel);

    playButton.onClick = [this] { if (onPlayClicked) onPlayClicked(); };
    stopButton.onClick = [this] { if (onStopClicked) onStopClicked(); };

    // Disable mixer/arrange tabs in Phase 0 (not implemented yet)
    mixerTab.setEnabled(false);
    arrangeTab.setEnabled(false);
    trackerTab.setToggleState(true, juce::dontSendNotification);

    bpmLabel.setText("BPM: 120", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId,
                       juce::Colour(ChipForgeLookAndFeel::ColourTextSecondary));
    bpmLabel.setJustificationType(juce::Justification::centredLeft);
}

void Toolbar::setIsPlaying(bool playing)
{
    playButton.setToggleState(playing, juce::dontSendNotification);
    stopButton.setToggleState(!playing, juce::dontSendNotification);
}

void Toolbar::setBpm(int bpm)
{
    bpmLabel.setText("BPM: " + juce::String(bpm), juce::dontSendNotification);
}

void Toolbar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(ChipForgeLookAndFeel::ColourPanel));

    // Bottom border line
    g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourActive));
    g.drawLine(0.0f, (float) getHeight() - 1.0f,
               (float) getWidth(), (float) getHeight() - 1.0f, 1.0f);
}

void Toolbar::resized()
{
    auto area = getLocalBounds().reduced(4, 4);

    // Left: transport
    playButton.setBounds(area.removeFromLeft(64));
    area.removeFromLeft(4);
    stopButton.setBounds(area.removeFromLeft(64));
    area.removeFromLeft(12);
    bpmLabel.setBounds(area.removeFromLeft(90));

    // Right: view tabs
    area.removeFromRight(4);
    arrangeTab.setBounds(area.removeFromRight(100));
    area.removeFromRight(2);
    mixerTab.setBounds(area.removeFromRight(100));
    area.removeFromRight(2);
    trackerTab.setBounds(area.removeFromRight(100));
}
