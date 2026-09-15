#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

// Classic VU-style scale. 0 VU sits slightly to the right of vertical,
// while the + side is compressed into the red region.
struct ScalePoint
{
    float db;
    float angleDegrees;
};

constexpr ScalePoint scale[] =
{
    { -20.0f, 222.0f },
    { -10.0f, 246.0f },
    {  -7.0f, 255.0f },
    {  -5.0f, 263.0f },
    {  -3.0f, 269.0f },
    {  -2.0f, 273.0f },
    {  -1.0f, 277.0f },
    {   0.0f, 282.0f },
    {   1.0f, 293.0f },
    {   2.0f, 304.0f },
    {   3.0f, 315.0f }
};

float dbToAngle(float db)
{
    const auto clampedDb = juce::jlimit(-20.0f, 3.0f, db);

    for (size_t i = 1; i < std::size(scale); ++i)
    {
        if (clampedDb <= scale[i].db)
        {
            return juce::jmap(clampedDb,
                              scale[i - 1].db, scale[i].db,
                              scale[i - 1].angleDegrees * pi / 180.0f,
                              scale[i].angleDegrees * pi / 180.0f);
        }
    }

    return scale[std::size(scale) - 1].angleDegrees * pi / 180.0f;
}

juce::Point<float> polarPoint(juce::Point<float> centre, float radius, float angle)
{
    return { centre.x + std::cos(angle) * radius,
             centre.y + std::sin(angle) * radius };
}
}

AnalogMeter::AnalogMeter(juce::String channelLabel, juce::String meterLabel)
    : channel(std::move(channelLabel)),
      meterType(std::move(meterLabel))
{
}

void AnalogMeter::setLevelsDb(float vuDb, float peakDb) noexcept
{
    const auto newVu = juce::jlimit(minDb, maxDb, vuDb);
    const auto newPeak = juce::jlimit(minDb, maxDb, peakDb);

    if (std::abs(newVu - vuLevelDb) > 0.001f
        || std::abs(newPeak - peakLevelDb) > 0.001f)
    {
        vuLevelDb = newVu;
        peakLevelDb = newPeak;
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

    // Hardware screws.
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

    // Cream analog face with a low pivot, like a physical panel meter.
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

    // Fine scale ticks.
    constexpr float minorStepDb = 0.5f;
    for (float db = -20.0f; db <= 3.001f; db += minorStepDb)
    {
        const auto angle = dbToAngle(db);
        const auto nearestMajor = std::round(db);
        const auto isMajor = std::abs(db - nearestMajor) < 0.001f;
        const auto outer = polarPoint(centre, radius * 0.84f, angle);
        const auto innerTick = polarPoint(centre,
                                          radius * (isMajor ? 0.66f : 0.75f),
                                          angle);

        g.setColour(db >= 0.0f
                        ? juce::Colour(0xffb52c20)
                        : juce::Colour(0xff171817));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y,
                   isMajor ? 1.7f : 0.75f);
    }

    // Main dB labels, matching a conventional analog VU face.
    const float labelValues[] = { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, 0.0f, 3.0f };
    g.setColour(juce::Colour(0xff171817));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold")));

    for (const auto db : labelValues)
    {
        const auto angle = dbToAngle(db);
        const auto labelPoint = polarPoint(centre, radius * 0.57f, angle);
        auto labelBounds = juce::Rectangle<float>(labelPoint.x - 18.0f,
                                                   labelPoint.y - 8.0f,
                                                   36.0f, 16.0f);
        const auto text = db > 0.0f ? "+" + juce::String(static_cast<int>(db))
                                    : juce::String(static_cast<int>(db));
        g.drawText(text, labelBounds.toNearestInt(), juce::Justification::centred);
    }

    // Program-scale percentage under the -20..0 VU range:
    // -20 dB = 20%, -15 dB = 40%, -10 dB = 60%, -5 dB = 80%, 0 dB = 100%.
    const float percentageDb[] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f };
    const int percentageValues[] = { 20, 40, 60, 80, 100 };
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(9.0f)));

    for (size_t i = 0; i < std::size(percentageDb); ++i)
    {
        const auto angle = dbToAngle(percentageDb[i]);
        const auto labelPoint = polarPoint(centre, radius * 0.48f, angle);
        auto labelBounds = juce::Rectangle<float>(labelPoint.x - 17.0f,
                                                   labelPoint.y + 3.0f,
                                                   34.0f, 13.0f);
        g.drawText(juce::String(percentageValues[i]) + "%",
                   labelBounds.toNearestInt(), juce::Justification::centred);
    }

    // Face branding.
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(17.0f).withStyle("Bold")));
    g.drawText(meterType,
               face.withY(face.getY() + face.getHeight() * 0.39f)
                   .withHeight(24.0f).toNearestInt(),
               juce::Justification::centred);

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(13.0f).withStyle("Bold")));
    g.drawText(channel,
               face.withX(face.getX() + 14.0f)
                   .withY(face.getY() + 12.0f)
                   .withWidth(28.0f).withHeight(20.0f).toNearestInt(),
               juce::Justification::centred);

    // Main VU needle: the classic, slower-moving program-level indicator.
    const auto vuAngle = dbToAngle(vuLevelDb);
    const auto vuTip = polarPoint(centre, radius * 0.70f, vuAngle);
    const auto vuBase = polarPoint(centre, radius * 0.055f, vuAngle + pi);

    g.setColour(juce::Colour(0xff171817));
    g.drawLine(vuBase.x, vuBase.y, vuTip.x, vuTip.y, 2.0f);

    // Peak/hold needle: red, fast peak detector with a slower visual return.
    const auto peakAngle = dbToAngle(peakLevelDb);
    const auto peakTip = polarPoint(centre, radius * 0.73f, peakAngle);
    const auto peakBase = polarPoint(centre, radius * 0.055f, peakAngle + pi);

    g.setColour(juce::Colour(0xffb72b1c));
    g.drawLine(peakBase.x, peakBase.y, peakTip.x, peakTip.y, 2.2f);

    // Shared pivot cap.
    g.setColour(juce::Colour(0xff171817));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 9.0f, centre.y - 9.0f,
                                         18.0f, 18.0f));
    g.setColour(juce::Colour(0xffa9a69d));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 4.0f, centre.y - 4.0f,
                                         8.0f, 8.0f));

    g.restoreState();
}
