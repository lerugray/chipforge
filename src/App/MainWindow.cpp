#include "MainWindow.h"

//==============================================================================
// MainContentComponent
//==============================================================================

MainContentComponent::MainContentComponent(TransportState& transport, AudioEngine& audio)
    : transportState(transport), audioEngine(audio)
{
    addAndMakeVisible(toolbar);

    toolbar.onPlayClicked = [this]
    {
        transportState.play();
        toolbar.setIsPlaying(true);
    };

    toolbar.onStopClicked = [this]
    {
        transportState.stop();
        toolbar.setIsPlaying(false);
    };

    addKeyListener(this);
    setWantsKeyboardFocus(true);

    setSize(1280, 800);
}

MainContentComponent::~MainContentComponent()
{
    removeKeyListener(this);
}

void MainContentComponent::resized()
{
    auto area = getLocalBounds();
    toolbar.setBounds(area.removeFromTop(Toolbar::Height));
    // remaining area = placeholder for TrackerView (Phase 2)
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(ChipForgeLookAndFeel::ColourBackground));

    // Phase 0 placeholder text in the main area
    auto placeholderArea = getLocalBounds().withTrimmedTop(Toolbar::Height);
    g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourTextSecondary));
    g.setFont(16.0f);
    g.drawFittedText("CHIPFORGE v0.1 :: PHASE 0\n\nPress PLAY to hear the test tone.\nTracker view coming in Phase 2.",
                     placeholderArea, juce::Justification::centred, 4);
}

bool MainContentComponent::keyPressed(const juce::KeyPress& key,
                                       juce::Component* /*originatingComponent*/)
{
#if MELATONIN_INSPECTOR
    // Ctrl+Shift+I — toggle Melatonin Inspector
    if (key == juce::KeyPress('i', juce::ModifierKeys::ctrlModifier
                                   | juce::ModifierKeys::shiftModifier, 0))
    {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector>(*this);
            inspector->onClose = [this] { inspector.reset(); };
        }
        inspector->setVisible(!inspector->isVisible());
        return true;
    }
#else
    juce::ignoreUnused(key);
#endif

    return false;
}

//==============================================================================
// MainWindow
//==============================================================================

MainWindow::MainWindow(const juce::String& name,
                       TransportState& transport,
                       AudioEngine& audio)
    : DocumentWindow(name,
                     juce::Colour(ChipForgeLookAndFeel::ColourBackground),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setContentOwned(new MainContentComponent(transport, audio), true);
    setResizable(true, false);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
