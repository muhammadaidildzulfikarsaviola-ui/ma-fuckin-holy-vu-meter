#include "AnalogMeter.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;

struct ScalePoint { float db; float angleDegrees; };

// Compact hardware-style sweep. 0 VU is intentionally right of vertical.
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
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Compact dark metal bezel.
    g.setColour(juce::Colour(0xff111314));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff5b5952));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    auto face = bounds.reduced(6.0f, 5.0f);

    // Warm illuminated cream, with a restrained central lamp glow.
    juce::ColourGradient lamp(juce::Colour(0xfffff7dd), face.getCentreX(), face.getY(),
                              juce::Colour(0xffd3c096), face.getRight(), face.getBottom(), true);
    lamp.addColour(0.32, juce::Colour(0xfff7ebcb));
    lamp.addColour(0.70, juce::Colour(0xffe2d0a7));
    g.setGradientFill(lamp);
    g.fillRoundedRectangle(face, 4.0f);
    g.setColour(juce::Colour(0xff796b55));
    g.drawRoundedRectangle(face, 4.0f, 1.0f);

    g.saveState();
    g.reduceClipRegion(face.toNearestInt());

    // Low pivot gives the classic moving-coil meter silhouette.
    const auto centre = juce::Point<float>(face.getCentreX(), face.getBottom() + 1.0f);
    const auto radius = juce::jmin(face.getWidth() * 0.43f, face.getHeight() * 1.06f);

    // Internal title. Nothing is placed outside the meter face.
    g.setColour(juce::Colour(0xff25231e));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(9.0f).withStyle("Bold")));
    g.drawText(meterType, face.getX() + 10, face.getY() + 7, 82, 12,
               juce::Justification::left);
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(8.0f).withStyle("Bold")));
    g.drawText(channel, face.getRight() - 24, face.getY() + 7, 16, 12,
               juce::Justification::right);

    // Tiny legend, kept away from the scale and needle path.
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(6.5f).withStyle("Bold")));
    if (secondaryIsMax)
    {
        g.setColour(juce::Colour(0xffb52d22));
        g.drawText("CURRENT", face.getCentreX() - 55, face.getY() + 8, 38, 10,
                   juce::Justification::centred);
        g.setColour(juce::Colour(0xff25231e));
        g.drawText("MAX", face.getCentreX() + 18, face.getY() + 8, 25, 10,
                   juce::Justification::centred);
    }
    else
    {
        g.setColour(juce::Colour(0xff25231e));
        g.drawText("VU", face.getCentreX() - 38, face.getY() + 8, 22, 10,
                   juce::Justification::centred);
        g.setColour(juce::Colour(0xffb52d22));
        g.drawText("HOLD", face.getCentreX() + 12, face.getY() + 8, 28, 10,
                   juce::Justification::centred);
    }

    // One clean tick per dB. Selected dB points get longer ticks.
    for (int dbInt = -20; dbInt <= 3; ++dbInt)
    {
        const auto db = static_cast<float>(dbInt);
        const auto angle = dbToAngle(db);
        const bool major = dbInt == -20 || dbInt == -10 || dbInt == -7 || dbInt == -5
                           || dbInt == -3 || dbInt == 0 || dbInt == 3;
        const auto outer = polarPoint(centre, radius * 0.82f, angle);
        const auto innerTick = polarPoint(centre, radius * (major ? 0.62f : 0.70f), angle);
        g.setColour(db >= 0.0f ? juce::Colour(0xffb52d22) : juce::Colour(0xff2a2822));
        g.drawLine(innerTick.x, innerTick.y, outer.x, outer.y, major ? 1.45f : 0.85f);
    }

    // dB numbers are part of the meter face, following the scale.
    const float labelValues[] = { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, -2.0f, -1.0f,
                                  0.0f, 1.0f, 2.0f, 3.0f };
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(8.2f).withStyle("Bold")));
    for (const auto db : labelValues)
    {
        const auto point = polarPoint(centre, radius * 0.56f, dbToAngle(db));
        const auto text = db > 0.0f ? "+" + juce::String(static_cast<int>(db))
                                    : juce::String(static_cast<int>(db));
        g.setColour(db >= 0.0f ? juce::Colour(0xffa52a20) : juce::Colour(0xff282620));
        g.drawText(text,
                   juce::Rectangle<int>(static_cast<int>(point.x - 13.0f),
                                        static_cast<int>(point.y - 6.0f), 26, 12),
                   juce::Justification::centred);
    }

    // Secondary percentage scale, also entirely inside the face.
    const float percentageDb[] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f };
    const int percentageValues[] = { 20, 40, 60, 80, 100 };
    g.setColour(juce::Colour(0xff5e5748));
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(6.7f)));
    for (size_t i = 0; i < std::size(percentageDb); ++i)
    {
        const auto point = polarPoint(centre, radius * 0.44f, dbToAngle(percentageDb[i]));
        g.drawText(juce::String(percentageValues[i]) + "%",
                   juce::Rectangle<int>(static_cast<int>(point.x - 14.0f),
                                        static_cast<int>(point.y - 5.0f), 28, 10),
                   juce::Justification::centred);
    }

    // Main needle: current true peak on top, VU on bottom.
    const auto mainAngle = dbToAngle(mainLevelDb);
    const auto mainTip = polarPoint(centre, radius * 0.70f, mainAngle);
    const auto mainBase = polarPoint(centre, radius * 0.045f, mainAngle + pi);
    g.setColour(secondaryIsMax ? juce::Colour(0xffb52d22) : juce::Colour(0xff24231f));
    g.drawLine(mainBase.x, mainBase.y, mainTip.x, mainTip.y, 1.85f);

    // Secondary needle: max true peak on top, VU peak hold on bottom.
    if (showSecondary)
    {
        const auto secondaryAngle = dbToAngle(secondaryLevelDb);
        const auto secondaryTip = polarPoint(centre, radius * 0.72f, secondaryAngle);
        const auto secondaryBase = polarPoint(centre, radius * 0.045f, secondaryAngle + pi);
        g.setColour(secondaryIsMax ? juce::Colour(0xff24231f) : juce::Colour(0xffb52d22));
        g.drawLine(secondaryBase.x, secondaryBase.y, secondaryTip.x, secondaryTip.y,
                   secondaryIsMax ? 1.55f : 1.7f);
    }

    g.setColour(juce::Colour(0xff24231f));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 6.5f, centre.y - 6.5f, 13.0f, 13.0f));
    g.setColour(juce::Colour(0xffb7aa86));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f));

    g.restoreState();
}
