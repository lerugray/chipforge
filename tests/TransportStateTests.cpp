#include <juce_core/juce_core.h>

#include "Engine/TransportState.h"

class TransportStateTests final : public juce::UnitTest
{
public:
    TransportStateTests()
        : juce::UnitTest("TransportState", "Engine")
    {
    }

    void runTest() override
    {
        beginTest("defaults to stopped at 120 BPM");
        TransportState transport;
        expect(!transport.getIsPlaying());
        expectEquals(transport.getBpm(), 120);

        beginTest("play, stop, and toggle update playback state");
        transport.play();
        expect(transport.getIsPlaying());

        transport.stop();
        expect(!transport.getIsPlaying());

        transport.toggle();
        expect(transport.getIsPlaying());

        transport.toggle();
        expect(!transport.getIsPlaying());

        beginTest("BPM can be changed");
        transport.setBpm(150);
        expectEquals(transport.getBpm(), 150);
    }
};

static TransportStateTests transportStateTests;
