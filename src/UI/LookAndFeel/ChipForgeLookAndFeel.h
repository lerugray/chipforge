#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// ChipForgeLookAndFeel
//
// SNES "American hardware" color scheme. Dark navy-purple base, vivid purple
// accents, teal highlights. Pixel font for all text.
//==============================================================================
class ChipForgeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // SNES color palette
    static constexpr uint32_t ColourBackground    = 0xFF1A1A2E;  // deep navy-purple
    static constexpr uint32_t ColourPanel         = 0xFF16213E;  // slightly lighter navy
    static constexpr uint32_t ColourActive        = 0xFF533483;  // SNES purple accent
    static constexpr uint32_t ColourTextPrimary   = 0xFFE0E0E0;  // soft white
    static constexpr uint32_t ColourTextSecondary = 0xFF8888AA;  // muted lavender-gray
    static constexpr uint32_t ColourHighlight     = 0xFF7B2FF7;  // vivid purple (power LED)
    static constexpr uint32_t ColourNoteOn        = 0xFF00D4AA;  // retro teal-green
    static constexpr uint32_t ColourWarning       = 0xFFFF6B6B;  // soft red

    ChipForgeLookAndFeel();

    // Font
    juce::Font getDefaultFont() const;

    // LookAndFeel overrides
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics&, juce::Label&) override;

    juce::Font getLabelFont(juce::Label&) override;

    void fillTextEditorBackground(juce::Graphics&, int width, int height,
                                  juce::TextEditor&) override;

    int getDefaultScrollbarWidth() override { return 8; }

private:
    juce::Font pixelFont;
    juce::Font pixelFontSmall;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChipForgeLookAndFeel)
};
