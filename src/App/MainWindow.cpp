#include "MainWindow.h"

//==============================================================================
// QWERTY piano layout helpers
//
//  Lower row (octave N):   Z  S  X  D  C  V  G  B  H  N  J  M
//                          C  C# D  D# E  F  F# G  G# A  A# B
//  Upper row (octave N+1): Q  2  W  3  E  R  5  T  6  Y  7  U
//                          C  C# D  D# E  F  F# G  G# A  A# B
//==============================================================================
static int qwertyToSemitone(int kc)
{
    // JUCE key codes use uppercase ASCII for letters (e.g. 'Z' = 90)
    switch (kc)
    {
        case 'Z': return 0;   case 'S': return 1;
        case 'X': return 2;   case 'D': return 3;
        case 'C': return 4;   case 'V': return 5;
        case 'G': return 6;   case 'B': return 7;
        case 'H': return 8;   case 'N': return 9;
        case 'J': return 10;  case 'M': return 11;
        case 'Q': return 12;  case '2': return 13;
        case 'W': return 14;  case '3': return 15;
        case 'E': return 16;  case 'R': return 17;
        case '5': return 18;  case 'T': return 19;
        case '6': return 20;  case 'Y': return 21;
        case '7': return 22;  case 'U': return 23;
        default:  return -1;
    }
}

//==============================================================================
// MainContentComponent
//==============================================================================

MainContentComponent::MainContentComponent(TransportState& transport, AudioEngine& audio)
    : transportState(transport), audioEngine(audio), trackerView(audio.getPattern())
{
    addAndMakeVisible(toolbar);
    addAndMakeVisible(trackerView);

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
    trackerView.setBounds(area);
}

void MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(ChipForgeLookAndFeel::ColourBackground));
}

bool MainContentComponent::keyPressed(const juce::KeyPress& key,
                                       juce::Component* /*originatingComponent*/)
{
    const int kc = key.getKeyCode();

    // Melatonin Inspector toggle (debug builds)
#if MELATONIN_INSPECTOR
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
#endif

    // Octave shift
    if (kc == '+' || kc == '=')
    {
        keyboardOctave = juce::jmin(keyboardOctave + 1, 8);
        repaint();
        return true;
    }
    if (kc == '-')
    {
        keyboardOctave = juce::jmax(keyboardOctave - 1, 0);
        repaint();
        return true;
    }

    // Note keys — ignore key repeat if same key is already held
    const int semitone = qwertyToSemitone(kc);
    if (semitone >= 0)
    {
        if (kc == activeKeyCode)
            return true;  // key repeat, note already playing

        const int octave   = keyboardOctave + (semitone >= 12 ? 1 : 0);
        const int midiNote = (octave + 1) * 12 + (semitone % 12);

        if (midiNote >= 33 && midiNote <= 112)
        {
            audioEngine.noteOn(midiNote);
            activeKeyCode = kc;
        }
        return true;
    }

    return false;
}

bool MainContentComponent::keyStateChanged(bool /*isKeyDown*/,
                                            juce::Component* /*originatingComponent*/)
{
    // If the held key is no longer down, release the note.
    if (activeKeyCode >= 0 && !juce::KeyPress::isKeyCurrentlyDown(activeKeyCode))
    {
        audioEngine.noteOff();
        activeKeyCode = -1;
        return true;
    }
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
