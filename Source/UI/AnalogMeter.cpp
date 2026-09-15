#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;
constexpr float startAngle = 225.0f * pi / 180.0f;
constexpr float endAngle = -45.0f * pi / 180.0f;

juce::Point<float> polarPoint(juce::Point<float> centre, float radius, float angle)
{
    return { centre.x + std::cos(angle) * radius, centre.y + std::sin(angle) * radius };
}

float dbToAngle(float db)
{
    return juce::jmap(juce::jlimit(-20.0f, 3.0f, db), -20.0f, 3.0f, startAngle, endAngle);
}
}

AnalogMeter::AnalogMeter(juce::String channelLabel) : channel(std::move(channelLabel)) {}

void AnalogMeter::setLevelDb(float db) noexcept
{
    const auto newLevel = juce::jlimit(minDb, maxDb, db);
    if (std::abs(newLevel - levelDb) > 0.001f)
    {
        levelDb = newLevel;
        repaint();
    }
}

void AnalogMeter::setPeakDb(float db) noexcept
{
    const auto newPeak = juce::jlimit(minDb, maxDb, db);
    if (std::abs(newPeak - peakDb) > 0.001f)
    {
        peakDb = newPeak;
        repaint();
    }
}

void AnalogMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    g.setColour(juce::Colour(0xff0b0d0e));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colour(0xff5a5650));
    g.drawRoundedRectangle(bounds, 10.0f, 1.5f);

    for (const auto point : { bounds.getTopLeft(), bounds.getTopRight(),
                              bounds.getBottomLeft(), bounds.getBottomRight() })
    {
        auto screw = juce::Rectangle<float>(point.x - 5.0f, point.y - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xff25282a));
        g.fillEllipse(screw);
        g.setColour(juce::Colour(0xff8d8a80));
        g.drawEllipse(screw, 1.0f);
        g.drawLine(screw.getX() + 2.5f, screw.getCentreY(), screw.getRight() - 2.5f, screw.getCentreY(), 1.0f);
    }

    auto inner = bounds.reduced(14.0f, 12.0f);
    const auto centre = juce::Point<float>(inner.getCentreX(), inner.getBottom() - 5.0f);
    const auto radius = juce::jmin(inner.getWidth() * 0.49f, inner.getHeight() * 0.92f);
    const auto face = juce::Rectangle<float>(centre.x - radius, centre.y - radius,
                                             radius * 2.0f, radius * 1.09f);

    g.setColour(juce::Colour(0xffe8ddc3));
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff8c806c));
    g.drawEllipse(face, 1.2f);

    for (int tick = 0; tick <= 46; ++tick)
    {
        const auto db = juce::jmap(static_cast<float>(tick), 0.0f, 46.0f, minDb, maxDb);
        const auto angle = dbToAngle(db);
        const auto major = tick % 10 == 0 || tick == 46;
        const auto outer = polarPoint(centre, radius * 0.84f, angle);
        const auto innerTick = polarPoint(centre, radius * (major ? 0.68f : 0.74f), angle);
        g.setColour(db >= 0.0f ? juce::Colour(0xffa62b1e) : juce::Colour(0xff171817));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y, major ? 1.8f : 0.8f);
    }

    constexpr float labels[] { -20.0f, -10.0f, -5.0f, 0.0f, 3.0f };
    for (const auto db : labels)
    {
        const auto point = polarPoint(centre, radius * 0.58f, dbToAngle(db));
        auto text = juce::String(db, 0);
        if (db > 0.0f) text = "+" + text;
        g.setColour(db >= 0.0f ? juce::Colour(0xffa62b1e) : juce::Colour(0xff171817));
        g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold")));
        g.drawText(text, juce::Rectangle<float>(point.x - 18.0f, point.y - 8.0f, 36.0f, 16.0f).toNearestInt(),
                   juce::Justification::centred);
    }

    g.setColour(juce::Colour(0xff171817));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(18.0f).withStyle("Bold")));
    g.drawText(channel, face.withX(face.getX() + 16.0f).withY(face.getY() + 14.0f)
                       .withWidth(28.0f).withHeight(24.0f).toNearestInt(), juce::Justification::centred);
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(17.0f).withStyle("Bold")));
    g.drawText("VU", face.withY(face.getY() + face.getHeight() * 0.48f).withHeight(26.0f).toNearestInt(),
               juce::Justification::centred);
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(10.0f)));
    g.drawText("0 VU = -18 dBFS", face.withY(face.getY() + face.getHeight() * 0.63f).withHeight(16.0f).toNearestInt(),
               juce::Justification::centred);

    const auto needleTip = polarPoint(centre, radius * 0.66f, dbToAngle(levelDb));
    const auto needleBase = polarPoint(centre, radius * 0.06f, dbToAngle(levelDb) + pi);
    g.setColour(juce::Colour(0xffb72b1c));
    g.drawLine(needleBase.x, needleBase.y, needleTip.x, needleTip.y, 2.4f);

    const auto peakPoint = polarPoint(centre, radius * 0.88f, dbToAngle(peakDb));
    g.setColour(juce::Colour(0xffd54831));
    g.fillEllipse(juce::Rectangle<float>(peakPoint.x - 2.0f, peakPoint.y - 2.0f, 4.0f, 4.0f));

    g.setColour(juce::Colour(0xff171817));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 9.0f, centre.y - 9.0f, 18.0f, 18.0f));
    g.setColour(juce::Colour(0xffa9a69d));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f));
}
