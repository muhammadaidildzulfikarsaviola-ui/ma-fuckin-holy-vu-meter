#include "DigitalMeter.h"

void DigitalMeter::setLevels(float leftPeakDb, float leftRmsDb,
                             float rightPeakDb, float rightRmsDb) noexcept
{
    left = { leftPeakDb, leftRmsDb };
    right = { rightPeakDb, rightRmsDb };
    repaint();
}

float DigitalMeter::normalise(float db) noexcept
{
    return juce::jlimit(0.0f, 1.0f, juce::jmap(db, minDb, maxDb, 0.0f, 1.0f));
}

void DigitalMeter::paint(juce::Graphics& g)
{
    auto panel = getLocalBounds().toFloat().reduced(4.0f);
    g.setColour(juce::Colour(0xff0a0d0f));
    g.fillRoundedRectangle(panel, 8.0f);
    g.setColour(juce::Colour(0xff42484b));
    g.drawRoundedRectangle(panel, 8.0f, 1.0f);

    auto content = panel.reduced(18.0f, 14.0f);
    auto leftArea = content.removeFromLeft(content.getWidth() * 0.5f).reduced(8.0f, 0.0f);
    auto rightArea = content.reduced(8.0f, 0.0f);
    drawChannel(g, leftArea, "L", left);
    drawChannel(g, rightArea, "R", right);
}

void DigitalMeter::drawChannel(juce::Graphics& g, juce::Rectangle<float> area,
                               const juce::String& label, const ChannelLevels& levels) const
{
    g.setColour(juce::Colour(0xffd5d8d5));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(22.0f).withStyle("Bold")));
    g.drawText(label, area.removeFromTop(28.0f).toNearestInt(), juce::Justification::centredLeft);

    const auto drawBar = [&g, &area](const juce::String& name, float db, juce::Colour active)
    {
        auto row = area.removeFromTop(62.0f);
        g.setColour(juce::Colour(0xffc9cecb));
        g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold")));
        g.drawText(name, row.removeFromLeft(42.0f).toNearestInt(), juce::Justification::centredLeft);

        auto valueArea = row.removeFromRight(64.0f);
        auto bar = row.reduced(2.0f, 17.0f);
        constexpr int segments = 27;
        const auto lit = juce::roundToInt(normalise(db) * segments);

        for (int segment = 0; segment < segments; ++segment)
        {
            auto segmentArea = bar.withX(bar.getX() + segment * bar.getWidth() / segments)
                                  .withWidth(bar.getWidth() / segments - 2.0f);
            const auto dbAtSegment = juce::jmap(static_cast<float>(segment + 1), 0.0f,
                                                static_cast<float>(segments), minDb, maxDb);
            auto colour = dbAtSegment >= 0.0f ? juce::Colour(0xffed4b35) : active;
            g.setColour(segment < lit ? colour : juce::Colour(0xff252b2e));
            g.fillRect(segmentArea);
        }

        g.setColour(juce::Colour(0xffd5d8d5));
        g.setFont(juce::Font(juce::FontOptions{}.withHeight(13.0f)));
        g.drawText(juce::String(db, 1) + " dB", valueArea.toNearestInt(),
                   juce::Justification::centredRight);
    };

    drawBar("RMS", levels.rmsDb, juce::Colour(0xff00bfe8));
    drawBar("PEAK", levels.peakDb, juce::Colour(0xff5fd9ee));
}
