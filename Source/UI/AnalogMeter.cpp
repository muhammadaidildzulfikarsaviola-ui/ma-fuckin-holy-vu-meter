#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

// Hardware-style VU scale: 0 VU is slightly right of vertical.
struct ScalePoint { float db; float angleDegrees; };

constexpr ScalePoint scale[] =
{
    { -20.0f, 222.0f }, { -10.0f, 246.0f }, { -7.0f, 255.0f },
    {  -5.0f, 263.0f }, {  -3.0f, 269.0f }, {  -2.0f, 273.0f },
    {  -1.0f, 277.0f }, {   0.0f, 282.0f }, {   1.0f, 293.0f },
    {   2.0f, 304.0f }, {   3.0f, 315.0f }
};

float dbToAngle(float db)
{
    const auto clampedDb = juce::jlimit(-20.0f, 3.0f, db);
    for (size_t i = 1; i < std::size(scale); ++i)
    {
        if (clampedDb <= scale[i].db)
            return juce::jmap(clampedDb, scale[i - 1].db, scale[i].db,
                              scale[i - 1].angleDegrees * pi / 180.0f,
                              scale[i].angleDegrees * pi / 180.0f);
    }
    return scale[std::size(scale) - 1].angleDegrees * pi / 180.0f;
}

juce::Point<float> polarPoint(juce::Point<float> centre, float radius, float angle)
{
    return { centre.x + std::cos(angle) * radius,
             centre.y + std::sin(angle) * radius };
}
}

AnalogMeter::AnalogMeter(juce::String channelLabel,
                         juce::String meterLabel,
                         bool showPeakHoldNeedle)
    : channel(std::move(channelLabel)),
      meterType(std::move(meterLabel)),
      showPeakHold(showPeakHoldNeedle)
{
}

void AnalogMeter::setLevelsDb(float mainDb, float peakHoldDb) noexcept
{
    const auto newMain = juce::jlimit(minDb, maxDb, mainDb);
    const auto newPeak = juce::jlimit(minDb, maxDb, peakHoldDb);
    if (std::abs(newMain - mainLevelDb) > 0.001f
        || std::abs(newPeak - peakHoldLevelDb) > 0.001f)
    {
        mainLevelDb = newMain;
        peakHoldLevelDb = newPeak;
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
        g.drawLine(screw.getX() + 2.5f, screw.getCentreY(),
                   screw.getRight() - 2.5f, screw.getCentreY(), 1.0f);
    }

    auto inner = bounds.reduced(14.0f, 12.0f);
    const auto centre = juce::Point<float>(inner.getCentreX(), inner.getBottom() + 2.0f);
    const auto radius = juce::jmin(inner.getWidth() * 0.49f, inner.getHeight() * 1.02f);
    const auto face = juce::Rectangle<float>(centre.x - radius, centre.y - radius,
                                             radius * 2.0f, radius * 1.10f);

    g.setColour(juce::Colour(0xffe8ddc3));
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff8c806c));
    g.drawEllipse(face, 1.2f);

    g.saveState();
    g.reduceClipRegion(face.toNearestInt());

    for (float db = -20.0f; db <= 3.001f; db += 0.5f)
    {
        const auto angle = dbToAngle(db);
        const auto isMajor = std::abs(db - std::round(db)) < 0.001f;
        const auto outer = polarPoint(centre, radius * 0.84f, angle);
        const auto innerTick = polarPoint(centre, radius * (isMajor ? 0.66f : 0.75f), angle);
        g.setColour(db >= 0.0f ? juce::Colour(0xffb52c20) : juce::Colour(0xff171817));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y, isMajor ? 1.7f : 0.75f);
    }

    const float labelValues[] = { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, 0.0f, 3.0f };
    g.setColour(juce::Colour(0xff171817));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold")));
    for (const auto db : labelValues)
    {
        const auto point = polarPoint(centre, radius * 0.57f, dbToAngle(db));
        auto labelBounds = juce::Rectangle<float>(point.x - 18.0f, point.y - 8.0f, 36.0f, 16.0f);
        const auto text = db > 0.0f ? "+" + juce::String(static_cast<int>(db)) : juce::String(static_cast<int>(db));
        g.drawText(text, labelBounds.toNearestInt(), juce::Justification::centred);
    }

    // Program percentage scale below the -20..0 VU range.
    const float percentageDb[] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f };
    const int percentageValues[] = { 20, 40, 60, 80, 100 };
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(9.0f)));
    for (size_t i = 0; i < std::size(percentageDb); ++i)
    {
        const auto point = polarPoint(centre, radius * 0.48f, dbToAngle(percentageDb[i]));
        auto labelBounds = juce::Rectangle<float>(point.x - 17.0f, point.y + 3.0f, 34.0f, 13.0f);
        g.drawText(juce::String(percentageValues[i]) + "%",
                   labelBounds.toNearestInt(), juce::Justification::centred);
    }

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(17.0f).withStyle("Bold")));
    g.drawText(meterType,
               face.withY(face.getY() + face.getHeight() * 0.39f).withHeight(24.0f).toNearestInt(),
               juce::Justification::centred);

    g.setFont(juce::Font(juce::FontOptions{}.withHeight(13.0f).withStyle("Bold")));
    g.drawText(channel,
               face.withX(face.getX() + 14.0f).withY(face.getY() + 12.0f)
                   .withWidth(70.0f).withHeight(20.0f).toNearestInt(),
               juce::Justification::centred);

    // Main needle. Top panels use this as the true-peak needle.
    const auto mainAngle = dbToAngle(mainLevelDb);
    const auto mainTip = polarPoint(centre, radius * 0.70f, mainAngle);
    const auto mainBase = polarPoint(centre, radius * 0.055f, mainAngle + pi);
    g.setColour(showPeakHold ? juce::Colour(0xff171817) : juce::Colour(0xffb72b1c));
    g.drawLine(mainBase.x, mainBase.y, mainTip.x, mainTip.y, 2.2f);

    // Bottom VU panels get a second red peak-hold needle.
    if (showPeakHold)
    {
        const auto peakAngle = dbToAngle(peakHoldLevelDb);
        const auto peakTip = polarPoint(centre, radius * 0.73f, peakAngle);
        const auto peakBase = polarPoint(centre, radius * 0.055f, peakAngle + pi);
        g.setColour(juce::Colour(0xffb72b1c));
        g.drawLine(peakBase.x, peakBase.y, peakTip.x, peakTip.y, 2.2f);
    }

    g.setColour(juce::Colour(0xff171817));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 9.0f, centre.y - 9.0f, 18.0f, 18.0f));
    g.setColour(juce::Colour(0xffa9a69d));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f));
    g.restoreState();
}
