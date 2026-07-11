#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "ui/widgets/RasterFilmstrip.h"
#include "ui/widgets/RasterSliderValueMapping.h"

TEST_CASE("Raster horizontal slider logarithmic mapping clamps to endpoints", "[ui][slider]") {
    REQUIRE(Ui::RasterSliderValueMapping::normaliseValue(10.0f,
                                                         20.0f,
                                                         20000.0f,
                                                         Ui::SliderValueMapping::logarithmic) == Catch::Approx(0.0f));
    REQUIRE(Ui::RasterSliderValueMapping::normaliseValue(20000.0f,
                                                         20.0f,
                                                         20000.0f,
                                                         Ui::SliderValueMapping::logarithmic) == Catch::Approx(1.0f));
    REQUIRE(Ui::RasterSliderValueMapping::normaliseValue(40000.0f,
                                                         20.0f,
                                                         20000.0f,
                                                         Ui::SliderValueMapping::logarithmic) == Catch::Approx(1.0f));
}

TEST_CASE("Raster horizontal slider logarithmic midpoint is geometric", "[ui][slider]") {
    const auto midpointValue = Ui::RasterSliderValueMapping::denormaliseValue(0.5f,
                                                                              20.0f,
                                                                              20000.0f,
                                                                              Ui::SliderValueMapping::logarithmic);
    REQUIRE(midpointValue == Catch::Approx(632.4555f).margin(0.001f));

    const auto midpointNormalised = Ui::RasterSliderValueMapping::normaliseValue(midpointValue,
                                                                                 20.0f,
                                                                                 20000.0f,
                                                                                 Ui::SliderValueMapping::logarithmic);
    REQUIRE(midpointNormalised == Catch::Approx(0.5f));
}

TEST_CASE("Raster horizontal slider frame mapping reaches first and last frames", "[ui][slider]") {
    const auto minNormalised = Ui::RasterSliderValueMapping::normaliseValue(20.0f,
                                                                            20.0f,
                                                                            20000.0f,
                                                                            Ui::SliderValueMapping::logarithmic);
    const auto maxNormalised = Ui::RasterSliderValueMapping::normaliseValue(20000.0f,
                                                                            20.0f,
                                                                            20000.0f,
                                                                            Ui::SliderValueMapping::logarithmic);

    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(minNormalised, 128) == 0);
    REQUIRE(Ui::RasterFilmstrip::frameIndexForNormalisedValue(maxNormalised, 128) == 127);
}

TEST_CASE("Raster horizontal slider asset uses one slider per filmstrip frame", "[ui][slider]") {
    const juce::Image sliderImage(juce::Image::ARGB, 444, 104 * 256, true);
    const Ui::RasterFilmstripSpec sliderSpec{
        .frameCount = 256,
        .frameWidth = 444,
        .frameHeight = 104,
        .orientation = Ui::FilmstripOrientation::vertical
    };

    REQUIRE(Ui::RasterFilmstrip::isValid(sliderImage, sliderSpec));
    REQUIRE(Ui::RasterFilmstrip::getFrameSourceBounds(sliderSpec, 0) == juce::Rectangle<int>(0, 0, 444, 104));
    REQUIRE(Ui::RasterFilmstrip::getFrameSourceBounds(sliderSpec, 255) == juce::Rectangle<int>(0, 104 * 255, 444, 104));
}

TEST_CASE("Raster horizontal slider frequency parser accepts Hz and kHz text", "[ui][slider]") {
    const auto plainValue = Ui::RasterSliderValueMapping::parseFrequencyText("30");
    REQUIRE(plainValue.has_value());
    REQUIRE(*plainValue == Catch::Approx(30.0f));

    const auto hzValue = Ui::RasterSliderValueMapping::parseFrequencyText("30 Hz");
    REQUIRE(hzValue.has_value());
    REQUIRE(*hzValue == Catch::Approx(30.0f));

    const auto khzValue = Ui::RasterSliderValueMapping::parseFrequencyText("1.2 kHz");
    REQUIRE(khzValue.has_value());
    REQUIRE(*khzValue == Catch::Approx(1200.0f));

    REQUIRE_FALSE(Ui::RasterSliderValueMapping::parseFrequencyText("fast").has_value());
}
