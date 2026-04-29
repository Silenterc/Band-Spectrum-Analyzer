#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "ui/widgets/RasterFilmstrip.h"

TEST_CASE("Raster filmstrip frame mapping clamps and reaches endpoints", "[ui][knob]") {
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(-0.25f, 128) == 0);
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(0.0f, 128) == 0);
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(1.0f, 128) == 127);
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(1.25f, 128) == 127);
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(0.5f, 128) == 63);
}

TEST_CASE("Raster filmstrip validates vertical and horizontal strips", "[ui][knob]") {
    const juce::Image verticalImage(juce::Image::ARGB, 120, 120 * 128, true);
    const Ui::RasterFilmstripSpec verticalSpec{
        .frameCount = 128,
        .frameWidth = 120,
        .frameHeight = 120,
        .orientation = Ui::FilmstripOrientation::vertical
    };
    REQUIRE(Ui::RasterFilmstrip::isValid(verticalImage, verticalSpec));

    const juce::Image horizontalImage(juce::Image::ARGB, 64 * 10, 64, true);
    const Ui::RasterFilmstripSpec horizontalSpec{
        .frameCount = 10,
        .frameWidth = 64,
        .frameHeight = 64,
        .orientation = Ui::FilmstripOrientation::horizontal
    };
    REQUIRE(Ui::RasterFilmstrip::isValid(horizontalImage, horizontalSpec));
    REQUIRE_FALSE(Ui::RasterFilmstrip::isValid(horizontalImage, verticalSpec));
}

TEST_CASE("Raster knob value helpers normalise and snap representative ranges", "[ui][knob]") {
    REQUIRE(Ui::RasterFilmstrip::normaliseValue(0.0f, 0.0f, 5000.0f) == Catch::Approx(0.0f));
    REQUIRE(Ui::RasterFilmstrip::normaliseValue(2500.0f, 0.0f, 5000.0f) == Catch::Approx(0.5f));
    REQUIRE(Ui::RasterFilmstrip::normaliseValue(5000.0f, 0.0f, 5000.0f) == Catch::Approx(1.0f));

    REQUIRE(Ui::RasterFilmstrip::snapValue(754.0f, 0.0f, 5000.0f, 10.0f) == Catch::Approx(750.0f));
    REQUIRE(Ui::RasterFilmstrip::snapValue(755.0f, 0.0f, 5000.0f, 10.0f) == Catch::Approx(760.0f));
    REQUIRE(Ui::RasterFilmstrip::snapValue(-100.0f, 0.0f, 5000.0f, 10.0f) == Catch::Approx(0.0f));
    REQUIRE(Ui::RasterFilmstrip::snapValue(5100.0f, 0.0f, 5000.0f, 10.0f) == Catch::Approx(5000.0f));
}

TEST_CASE("Raster knob parser accepts optional suffix and rejects invalid input", "[ui][knob]") {
    const auto plainValue = Ui::RasterFilmstrip::parseNumericText("750", "ms");
    REQUIRE(plainValue.has_value());
    REQUIRE(*plainValue == Catch::Approx(750.0f));

    const auto suffixedValue = Ui::RasterFilmstrip::parseNumericText("750 ms", "ms");
    REQUIRE(suffixedValue.has_value());
    REQUIRE(*suffixedValue == Catch::Approx(750.0f));

    const auto signedValue = Ui::RasterFilmstrip::parseNumericText("+6 dB", "dB");
    REQUIRE(signedValue.has_value());
    REQUIRE(*signedValue == Catch::Approx(6.0f));

    REQUIRE_FALSE(Ui::RasterFilmstrip::parseNumericText("fast", "ms").has_value());
    REQUIRE_FALSE(Ui::RasterFilmstrip::parseNumericText("12-3", "ms").has_value());
}
