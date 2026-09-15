#include "AnalogMeter.h"

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;
    constexpr float startAngle = 225.0f * pi / 180.0f;
    constexpr float endAngle = -45.0f * pi / 180.0f;

    juce::Point<float> polarPoint(juce::Point<float> centre, float radius, float angle)
    {
        return {
            centre.x + std::cos(angle) * radius,
            centre.y + std::sin(angle) * radius
        };
    }
}

void AnalogMeter::setLevelDb(float db) noexcept
{
    const auto newLevel = juce::jlimit(minDb, maxDb, db);

    if (std::abs(newLevel - levelDb) > 0.001f)
    {
        levelDb = newLevel;
        repaint();
    }
}

float AnalogMeter::getLevelDb() const noexcept
{
    return levelDb;
}

void AnalogMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    const auto centre = juce::Point<float>(bounds.getCentreX(), bounds.getBottom() - 8.0f);
    const auto radius = juce::jmin(bounds.getWidth() * 0.46f, bounds.getHeight() * 0.90f);

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(bounds, 18.0f);

    const auto face = juce::Rectangle<float>(
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 1.10f);

    g.setColour(juce::Colour(0xffeee8d5));
    g.fillEllipse(face);

    // Main scale arc.
    juce::Path scaleArc;
    scaleArc.addCentredArc(
        centre.x, centre.y, radius * 0.91f, radius * 0.91f,
        0.0f, startAngle, endAngle, true);

    g.setColour(juce::Colours::darkgrey);
    g.strokePath(scaleArc, juce::PathStrokeType(7.0f));

    // Red overload zone: 0 to +3 dB.
    juce::Path redArc;
    const auto zeroAngle = juce::jmap(0.0f, minDb, maxDb, startAngle, endAngle);
    redArc.addCentredArc(
        centre.x, centre.y, radius * 0.91f, radius * 0.91f,
        0.0f, zeroAngle, endAngle, true);
    g.setColour(juce::Colours::red);
    g.strokePath(redArc, juce::PathStrokeType(7.0f));

    constexpr float ticks[] { -20.0f, -10.0f, -5.0f, 0.0f, 1.0f, 3.0f };

    for (const auto db : ticks)
    {
        const auto angle = juce::jmap(db, minDb, maxDb, startAngle, endAngle);
        const auto outer = polarPoint(centre, radius * 0.84f, angle);
        const auto inner = polarPoint(centre, radius * 0.70f, angle);

        g.setColour(db >= 0.0f ? juce::Colours::red : juce::Colours::black);
        g.drawLine(juce::Line<float>(inner, outer), db == 0.0f ? 3.0f : 2.0f);

        const auto textPoint = polarPoint(centre, radius * 0.60f, angle);
        juce::String text = juce::String(db, 0);
        if (db > 0.0f)
            text = "+" + text;

        g.setFont(juce::Font(juce::FontOptions{}.withHeight(15.0f).withStyle("Bold")));
        g.setColour(db >= 0.0f ? juce::Colours::red : juce::Colours::black);
        g.drawText(
            text,
            juce::Rectangle<float>(textPoint.x - 24.0f, textPoint.y - 10.0f, 48.0f, 20.0f).toNearestInt(),
            juce::Justification::centred);
    }

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(18.0f).withStyle("Bold")));
    g.setColour(juce::Colours::black);
    g.drawText(
        "VU",
        face.withY(face.getY() + face.getHeight() * 0.49f).withHeight(30.0f).toNearestInt(),
        juce::Justification::centred);

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f)));
    g.drawText(
        "dBFS",
        face.withY(face.getY() + face.getHeight() * 0.62f).withHeight(20.0f).toNearestInt(),
        juce::Justification::centred);

    // Needle.
    const auto needleAngle = juce::jmap(levelDb, minDb, maxDb, startAngle, endAngle);
    const auto needleTip = polarPoint(centre, radius * 0.66f, needleAngle);
    const auto needleBase = polarPoint(centre, radius * 0.08f, needleAngle + pi);

    juce::Path needle;
    needle.startNewSubPath(needleBase);
    needle.lineTo(needleTip);

    g.setColour(juce::Colours::red);
    g.strokePath(
        needle,
        juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colours::red);
    g.fillEllipse(juce::Rectangle<float>(centre.x - 9.0f, centre.y - 9.0f, 18.0f, 18.0f));
    g.setColour(juce::Colours::black);
    g.fillEllipse(juce::Rectangle<float>(centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f));
}

void AnalogMeter::resized()
{
}
