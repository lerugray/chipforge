#include <juce_core/juce_core.h>

#include <iostream>

class ConsoleUnitTestRunner final : public juce::UnitTestRunner
{
public:
    void logMessage(const juce::String& message) override
    {
        std::cout << message << '\n';
    }
};

int main()
{
    ConsoleUnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.setPassesAreLogged(true);
    runner.runAllTests();

    if (runner.getNumResults() == 0)
    {
        std::cerr << "No JUCE unit tests were registered.\n";
        return 1;
    }

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* result = runner.getResult(i);
        if (result != nullptr && result->failures > 0)
            return 1;
    }

    return 0;
}
