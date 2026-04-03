#include "ChipForgeLookAndFeel.h"
#include "BinaryData.h"

ChipForgeLookAndFeel::ChipForgeLookAndFeel()
{
    // Load pixel fonts from embedded binary data
    pixelFont = juce::Font(
        juce::FontOptions(juce::Typeface::createSystemTypefaceFor(
            BinaryData::PressStart2PRegular_ttf,
            BinaryData::PressStart2PRegular_ttfSize)).withHeight(14.0f));

    pixelFontSmall = juce::Font(
        juce::FontOptions(juce::Typeface::createSystemTypefaceFor(
            BinaryData::PixelifySansRegular_ttf,
            BinaryData::PixelifySansRegular_ttfSize)).withHeight(14.0f));

    // Base colour scheme
    setColour(juce::ResizableWindow::backgroundColourId,   juce::Colour(ColourBackground));
    setColour(juce::DocumentWindow::textColourId,          juce::Colour(ColourTextPrimary));

    setColour(juce::TextButton::buttonColourId,            juce::Colour(ColourPanel));
    setColour(juce::TextButton::buttonOnColourId,          juce::Colour(ColourActive));
    setColour(juce::TextButton::textColourOffId,           juce::Colour(ColourTextPrimary));
    setColour(juce::TextButton::textColourOnId,            juce::Colour(ColourTextPrimary));

    setColour(juce::ToggleButton::textColourId,            juce::Colour(ColourTextPrimary));
    setColour(juce::ToggleButton::tickColourId,            juce::Colour(ColourHighlight));

    setColour(juce::Label::textColourId,                   juce::Colour(ColourTextPrimary));
    setColour(juce::Label::backgroundColourId,             juce::Colour(0x00000000));

    setColour(juce::TextEditor::backgroundColourId,        juce::Colour(ColourPanel));
    setColour(juce::TextEditor::textColourId,              juce::Colour(ColourTextPrimary));
    setColour(juce::TextEditor::highlightColourId,         juce::Colour(ColourActive));
    setColour(juce::TextEditor::outlineColourId,           juce::Colour(ColourActive));
    setColour(juce::TextEditor::focusedOutlineColourId,    juce::Colour(ColourHighlight));

    setColour(juce::ComboBox::backgroundColourId,          juce::Colour(ColourPanel));
    setColour(juce::ComboBox::textColourId,                juce::Colour(ColourTextPrimary));
    setColour(juce::ComboBox::outlineColourId,             juce::Colour(ColourActive));
    setColour(juce::ComboBox::arrowColourId,               juce::Colour(ColourTextSecondary));

    setColour(juce::Slider::thumbColourId,                 juce::Colour(ColourHighlight));
    setColour(juce::Slider::trackColourId,                 juce::Colour(ColourActive));
    setColour(juce::Slider::backgroundColourId,            juce::Colour(ColourPanel));

    setColour(juce::ScrollBar::thumbColourId,              juce::Colour(ColourActive));
    setColour(juce::ScrollBar::backgroundColourId,         juce::Colour(ColourPanel));

    setColour(juce::PopupMenu::backgroundColourId,         juce::Colour(ColourPanel));
    setColour(juce::PopupMenu::textColourId,               juce::Colour(ColourTextPrimary));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(ColourActive));
    setColour(juce::PopupMenu::highlightedTextColourId,    juce::Colour(ColourTextPrimary));
}

juce::Font ChipForgeLookAndFeel::getDefaultFont() const
{
    return pixelFontSmall;
}

void ChipForgeLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                 juce::Button& button,
                                                 const juce::Colour& /*backgroundColour*/,
                                                 bool isHighlighted,
                                                 bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    const bool isToggled = button.getToggleState();

    juce::Colour fill;
    if (isDown || isToggled)
        fill = juce::Colour(ColourActive);
    else if (isHighlighted)
        fill = juce::Colour(ColourPanel).brighter(0.1f);
    else
        fill = juce::Colour(ColourPanel);

    g.setColour(fill);
    g.fillRect(bounds);

    // Subtle bevel — SNES menu chrome feel
    const auto borderColour = isDown || isToggled
        ? juce::Colour(ColourHighlight)
        : juce::Colour(ColourActive);

    g.setColour(borderColour);
    g.drawRect(bounds, 1.0f);
}

void ChipForgeLookAndFeel::drawButtonText(juce::Graphics& g,
                                           juce::TextButton& button,
                                           bool /*isHighlighted*/,
                                           bool /*isDown*/)
{
    g.setFont(pixelFontSmall);
    g.setColour(button.findColour(juce::TextButton::textColourOffId));
    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds().reduced(4, 2),
                     juce::Justification::centred, 1);
}

void ChipForgeLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    if (!label.isBeingEdited())
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(getLabelFont(label));
        g.drawFittedText(label.getText(),
                         label.getBorderSize().subtractedFrom(label.getLocalBounds()),
                         label.getJustificationType(),
                         juce::jmax(1, (int) ((float) label.getHeight() / getLabelFont(label).getHeight())));
    }
}

juce::Font ChipForgeLookAndFeel::getLabelFont(juce::Label& /*label*/)
{
    return pixelFontSmall;
}

void ChipForgeLookAndFeel::fillTextEditorBackground(juce::Graphics& g,
                                                      int width, int height,
                                                      juce::TextEditor& /*editor*/)
{
    g.setColour(juce::Colour(ColourPanel));
    g.fillRect(0, 0, width, height);
}
