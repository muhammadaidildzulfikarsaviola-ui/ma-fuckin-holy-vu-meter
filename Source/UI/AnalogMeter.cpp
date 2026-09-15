#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

struct ScalePoint { float db; float angleDegrees; };

// Hardware-inspired sweep: 0 VU sits noticeably to the right of vertical.
constexpr ScalePoint scale[] =
{
    { -20.0f, 218.0f }, { -10.0f, 247.0f }, { -7.0f, 257.0f },
    {  -5.0f, 264.0f }, {  -3.0f, 270.0f }, {  -2.0f, 273.0f },
    {  -1.0f, 276.0f }, {   0.0f, 281.0f }, {   1.0f, 294.0f },
    {   2.0f, 307.0f }, {   3.0f, 320.0f }
};

float dbToAngle(float db)
{
    const auto clampedDb = juce::jlimit(-20.0f, 3.0f, db);
    for (size_t i = 1; i < std::size(scale); ++i)
        if (clampedDb <= scale[i].db)
            return juce::jmap(clampedDb, scale[i - 1].db, scale[i].db,
                              scale[i - 1].angleDegrees * pi / 180.0f,
                              scale[i].angleDegrees * pi / 180.0f);
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
                         bool showSecondaryNeedle,
                         bool secondaryIsMaxValue)
    : channel(std::move(channelLabel)),
      meterType(std::move(meterLabel)),
      showSecondary(showSecondaryNeedle),
      secondaryIsMax(secondaryIsMaxValue)
{
}

void AnalogMeter::setLevelsDb(float mainDb, float secondaryDb) noexcept
{
    const auto newMain = juce::jlimit(minDb, maxDb, mainDb);
    const auto newSecondary = juce::jlimit(minDb, maxDb, secondaryDb);
    if (std::abs(newMain - mainLevelDb) > 0.001f
        || std::abs(newSecondary - secondaryLevelDb) > 0.001f)
    {
        mainLevelDb = newMain;
        secondaryLevelDb = newSecondary;
        repaint();
    }
}

void AnalogMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    g.setColour(juce::Colour(0xff0b0d0e));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(0xff514d46));
    g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

    for (const auto point : { bounds.getTopLeft(), bounds.getTopRight(),
                              bounds.getBottomLeft(), bounds.getBottomRight() })
    {
        auto screw = juce::Rectangle<float>(point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f);
        g.setColour(juce::Colour(0xff202325));
        g.fillEllipse(screw);
        g.setColour(juce::Colour(0xff77736a));
        g.drawEllipse(screw, 1.0f);
    }

    // Rectangular illuminated face, closer to the Waves-style hardware look.
    auto face = bounds.reduced(14.0f, 13.0f).withTrimmedBottom(5.0f);
    const auto faceCentre = face.getCentre();
    juce::ColourGradient lamp(juce::Colour(0xfffff8df), faceCentre.x, faceCentre.y,
                              juce::Colour(0xffc5b486), face.getX(), face.getBottom(), true);
    lamp.addColour(0.38, juce::Colour(0xfff3e7c7));
    lamp.addColour(0.75, juce::Colour(0xffdfcea5));
    g.setGradientFill(lamp);
    g.fillRoundedRectangle(face, 5.0f);
    g.setColour(juce::Colour(0xff766a56));
    g.drawRoundedRectangle(face, 5.0f, 1.1f);

    g.saveState();
    g.reduceClipRegion(face.toNearestInt());

    // Pivot is low, as on a physical moving-coil meter.
    const auto centre = juce::Point<float>(face.getCentreX(), face.getBottom() + 1.0f);
    const auto radius = juce::jmin(face.getWidth() * 0.43f, face.getHeight() * 1.06f);

    // Deliberately sparse: only one tick per dB, with stronger labelled ticks.
    for (int dbInt = -20; dbInt <= 3; ++dbInt)
    {
        const auto db = static_cast<float>(dbInt);
        const auto angle = dbToAngle(db);
        const bool major = dbInt == -20 || dbInt == -10 || dbInt == -7 || dbInt == -5
                           || dbInt == -3 || dbInt == 0 || dbInt == 3;
        const auto outer = polarPoint(centre, radius * 0.79f, angle);
        const auto innerTick = polarPoint(centre, radius * (major ? 0.61f : 0.69f), angle);
        g.setColour(db >= 0.0f ? juce::Colour(0xffb52d22) : juce::Colour(0xff292620));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y, major ? 1.55f : 0.9f);
    }

    const float labelValues[] = { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, 0.0f, 3.0f };
    g.setColour(juce::Colour(0xff25221d));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(10.5f).withStyle("Bold")));
    for (const auto db : labelValues)
    {
        const auto point = polarPoint(centre, radius * 0.53f, dbToAngle(db));
        auto labelBounds = juce::Rectangle<float>(point.x - 16.0f, point.y - 7.0f, 32.0f, 14.0f);
        const auto text = db > 0.0f ? "+" + juce::String(static_cast<int>(db))
                                    : juce::String(static_cast<int>(db));
        g.drawText(text, labelBounds.toNearestInt(), juce::Justification::centred);
    }

    const float percentageDb[] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f };
    const int percentageValues[] = { 20, 40, 60, 80, 100 };
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(8.0f)));
    for (size_t i = 0; i < std::size(percentageDb); ++i)
    {
        const auto point = polarPoint(centre, radius * 0.43f, dbToAngle(percentageDb[i]));
        auto labelBounds = juce::Rectangle<float>(point.x - 14.0f, point.y + 2.0f, 28.0f, 11.0f);
        g.drawText(juce::String(percentageValues[i]) + "%",
                   labelBounds.toNearestInt(), juce::Justification::centred);
    }

    // Small label, high on the face so it never collides with the needles.
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold")));
    g.drawText(meterType,
               face.withY(face.getY() + face.getHeight() * 0.18f)
                   .withHeight(16.0f).toNearestInt(),
               juce::Justification::centred);

    // Main/current needle.
    const auto mainAngle = dbToAngle(mainLevelDb);
    const auto mainTip = polarPoint(centre, radius * 0.68f, mainAngle);
    const auto mainBase = polarPoint(centre, radius * 0.045f, mainAngle + pi);
    g.setColour(secondaryIsMax ? juce::Colour(0xff171716) : juce::Colour(0xff171716));
    g.drawLine(mainBase.x, mainBase.y, mainTip.x, mainTip.y, 1.8f);

    // Secondary: MAX true peak or VU peak hold.
    if (showSecondary)
    {
        const auto secondaryAngle = dbToAngle(secondaryLevelDb);
        const auto secondaryTip = polarPoint(centre, radius * 0.70f, secondaryAngle);
        const auto secondaryBase = polarPoint(centre, radius * 0.045f, secondaryAngle + pi);
        g.setColour(juce::Colour(0xffb72b1c));
        g.drawLine(secondaryBase.x, secondaryBase.y, secondaryTip.x, secondaryTip.y, 1.8f);
    }

    g.setColour(juce::Colour(0xff171716));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 7.0f, centre.y - 7.0f, 14.0f, 14.0f));
    g.setColour(juce::Colour(0xffaaa49a));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 2.7f, centre.y - 2.7f, 5.4f, 5.4f));
    g.restoreState();
}
