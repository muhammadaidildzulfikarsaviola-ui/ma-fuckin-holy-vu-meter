#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

// Classic analog VU sweep: lower-left -> top-center -> upper-right.
constexpr float lowerLeftAngle = 225.0f * pi / 180.0f; // -20 VU
constexpr float zeroVuAngle = 270.0f * pi / 180.0f;    // 0 VU
constexpr float upperRightAngle = 315.0f * pi / 180.0f; // +3 VU

juce::Point<float> polarPoint(juce::Point<float> centre, float radius, float angle)
{
    return { centre.x + std::cos(angle) * radius,
             centre.y + std::sin(angle) * radius };
}

float dbToAngle(float db)
{
    const auto clampedDb = juce::jlimit(-20.0f, 3.0f, db);

    if (clampedDb <= 0.0f)
        return juce::jmap(clampedDb, -20.0f, 0.0f,
                          lowerLeftAngle, zeroVuAngle);

    return juce::jmap(clampedDb, 0.0f, 3.0f,
                      zeroVuAngle, upperRightAngle);
}
}

AnalogMeter::AnalogMeter(juce::String channelLabel, juce::String meterLabel)
    : channel(std::move(channelLabel)),
      meterType(std::move(meterLabel))
{
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

void AnalogMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // Dark outer chassis.
    g.setColour(juce::Colour(0xff0b0d0e));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colour(0xff5a5650));
    g.drawRoundedRectangle(bounds, 10.0f, 1.5f);

    // Simple hardware screws.
    for (const auto point : { bounds.getTopLeft(), bounds.getTopRight(),
                              bounds.getBottomLeft(), bounds.getBottomRight() })
    {
        auto screw = juce::Rectangle<float>(point.x - 5.0f, point.y - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xff25282a));
        g.fillEllipse(screw);
        g.setColour(juce::Colour(0xff8d8a80));
        g.drawEllipse(screw, 1.0f);
        g.drawLine(screw.getX() + 2.5f, screw.getCentreY(),
                   screw.getRight() - 2.5f, screw.getCentreY(), 1.0f);
    }

    // Cream analog face. The pivot sits low, like a real panel meter.
    auto inner = bounds.reduced(14.0f, 12.0f);
    const auto centre = juce::Point<float>(inner.getCentreX(), inner.getBottom() + 2.0f);
    const auto radius = juce::jmin(inner.getWidth() * 0.49f,
                                   inner.getHeight() * 1.02f);
    const auto face = juce::Rectangle<float>(centre.x - radius, centre.y - radius,
                                             radius * 2.0f, radius * 1.10f);

    g.setColour(juce::Colour(0xffe8ddc3));
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff8c806c));
    g.drawEllipse(face, 1.2f);

    g.saveState();
    g.reduceClipRegion(face.toNearestInt());

    // Dense tick marks, with no numeric scale clutter.
    for (int tick = 0; tick <= 46; ++tick)
    {
        const auto db = juce::jmap(static_cast<float>(tick), 0.0f, 46.0f,
                                   minDb, maxDb);
        const auto angle = dbToAngle(db);
        const auto major = tick % 10 == 0 || tick == 46;
        const auto outer = polarPoint(centre, radius * 0.84f, angle);
        const auto innerTick = polarPoint(centre,
                                          radius * (major ? 0.68f : 0.74f),
                                          angle);

        g.setColour(db >= 0.0f
                        ? juce::Colour(0xffa62b1e)
                        : juce::Colour(0xff171817));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y,
                   major ? 1.8f : 0.8f);
    }

    // Minimal face branding. No numbers inside the analog meter.
    g.setColour(juce::Colour(0xff171817));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(17.0f).withStyle("Bold")));
    g.drawText(meterType,
               face.withY(face.getY() + face.getHeight() * 0.36f)
                   .withHeight(24.0f).toNearestInt(),
               juce::Justification::centred);

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(13.0f).withStyle("Bold")));
    g.drawText(channel,
               face.withX(face.getX() + 14.0f)
                   .withY(face.getY() + 12.0f)
                   .withWidth(28.0f).withHeight(20.0f).toNearestInt(),
               juce::Justification::centred);

    // Needle.
    const auto angle = dbToAngle(levelDb);
    const auto needleTip = polarPoint(centre, radius * 0.70f, angle);
    const auto needleBase = polarPoint(centre, radius * 0.055f, angle + pi);

    g.setColour(juce::Colour(0xffb72b1c));
    g.drawLine(needleBase.x, needleBase.y,
               needleTip.x, needleTip.y, 2.4f);

    // Pivot cap.
    g.setColour(juce::Colour(0xff171817));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 9.0f, centre.y - 9.0f,
                                         18.0f, 18.0f));
    g.setColour(juce::Colour(0xffa9a69d));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 4.0f, centre.y - 4.0f,
                                         8.0f, 8.0f));

    g.restoreState();
}
