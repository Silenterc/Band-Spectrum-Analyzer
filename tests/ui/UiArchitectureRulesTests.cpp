#include <catch2/catch_test_macros.hpp>

#include <juce_core/juce_core.h>

namespace {
    void appendFiles(juce::Array<juce::File>& files, const juce::File& directory, const juce::String& pattern) {
        files.addArray(directory.findChildFiles(juce::File::findFiles, true, pattern));
    }

    juce::Array<juce::File> sourceFilesIn(const juce::File& directory) {
        juce::Array<juce::File> files;
        appendFiles(files, directory, "*.h");
        appendFiles(files, directory, "*.cpp");
        return files;
    }
}

TEST_CASE("UI sources do not include plugin headers", "[ui][architecture]") {
    const auto uiDirectory = juce::File(BSA_REPO_ROOT).getChildFile("src").getChildFile("ui");
    REQUIRE(uiDirectory.isDirectory());

    const auto files = sourceFilesIn(uiDirectory);

    for (const auto& file : files) {
        const auto text = file.loadFileAsString();
        INFO(file.getFullPathName().toStdString());
        REQUIRE_FALSE(text.contains("#include \"plugin/"));
        REQUIRE_FALSE(text.contains("#include <plugin/"));
        REQUIRE_FALSE(text.contains("plugin/presets"));
        REQUIRE_FALSE(text.contains("PluginPresets::"));
    }
}

TEST_CASE("UI sources use feature-local state and contract folders", "[ui][architecture]") {
    const auto sourceDirectory = juce::File(BSA_REPO_ROOT).getChildFile("src");
    REQUIRE(sourceDirectory.isDirectory());

    const auto files = sourceFilesIn(sourceDirectory);
    for (const auto& file : files) {
        const auto text = file.loadFileAsString();
        INFO(file.getFullPathName().toStdString());
        REQUIRE_FALSE(text.contains("ui/contracts/"));
        REQUIRE_FALSE(text.contains("ui/state/"));
    }

    REQUIRE_FALSE(sourceDirectory.getChildFile("ui").getChildFile("contracts").exists());
    REQUIRE_FALSE(sourceDirectory.getChildFile("ui").getChildFile("state").exists());
}

TEST_CASE("Shared headers do not depend on UI headers", "[ui][architecture]") {
    const auto sharedDirectory = juce::File(BSA_REPO_ROOT).getChildFile("src").getChildFile("shared");
    REQUIRE(sharedDirectory.isDirectory());

    const auto files = sourceFilesIn(sharedDirectory);
    for (const auto& file : files) {
        const auto text = file.loadFileAsString();
        INFO(file.getFullPathName().toStdString());
        REQUIRE_FALSE(text.contains("#include \"ui/"));
        REQUIRE_FALSE(text.contains("#include <ui/"));
        REQUIRE_FALSE(text.contains("../ui/"));
    }
}

TEST_CASE("Popup content does not dismiss callouts directly", "[ui][architecture]") {
    const auto uiDirectory = juce::File(BSA_REPO_ROOT).getChildFile("src").getChildFile("ui");
    REQUIRE(uiDirectory.isDirectory());

    juce::Array<juce::File> popupDirectories;
    popupDirectories.add(uiDirectory.getChildFile("presets").getChildFile("popups"));
    popupDirectories.add(uiDirectory.getChildFile("analyzer").getChildFile("rack").getChildFile("popups"));

    for (const auto& directory : popupDirectories) {
        REQUIRE(directory.isDirectory());
        for (const auto& file : sourceFilesIn(directory)) {
            const auto text = file.loadFileAsString();
            INFO(file.getFullPathName().toStdString());
            REQUIRE_FALSE(text.contains("findParentComponentOfClass<juce::CallOutBox>"));
            REQUIRE_FALSE(text.contains("CalloutPresenter"));
        }
    }
}

TEST_CASE("Preset header is not owned by analyzer layout", "[ui][architecture]") {
    const auto analyzerDirectory = juce::File(BSA_REPO_ROOT)
                                       .getChildFile("src")
                                       .getChildFile("ui")
                                       .getChildFile("analyzer");
    REQUIRE(analyzerDirectory.isDirectory());

    const auto files = sourceFilesIn(analyzerDirectory);
    for (const auto& file : files) {
        INFO(file.getFullPathName().toStdString());
        REQUIRE_FALSE(file.getFileName().startsWith("PresetHeader"));
    }
}

TEST_CASE("Analyzer plot uses state folder instead of data folder", "[ui][architecture]") {
    const auto plotDirectory = juce::File(BSA_REPO_ROOT)
                                   .getChildFile("src")
                                   .getChildFile("ui")
                                   .getChildFile("analyzer")
                                   .getChildFile("plot");
    REQUIRE(plotDirectory.isDirectory());

    REQUIRE_FALSE(plotDirectory.getChildFile("data").exists());
    REQUIRE(plotDirectory.getChildFile("state").isDirectory());
}

TEST_CASE("Analyzer controls and editor components use view folders", "[ui][architecture]") {
    const auto uiDirectory = juce::File(BSA_REPO_ROOT).getChildFile("src").getChildFile("ui");
    REQUIRE(uiDirectory.isDirectory());

    const auto controlsDirectory = uiDirectory.getChildFile("analyzer").getChildFile("controls");
    REQUIRE(controlsDirectory.isDirectory());
    REQUIRE(controlsDirectory.getChildFile("view").isDirectory());
    REQUIRE(controlsDirectory.findChildFiles(juce::File::findFiles, false, "*.h").isEmpty());
    REQUIRE(controlsDirectory.findChildFiles(juce::File::findFiles, false, "*.cpp").isEmpty());

    const auto editorDirectory = uiDirectory.getChildFile("editor");
    REQUIRE(editorDirectory.isDirectory());
    REQUIRE(editorDirectory.getChildFile("view").isDirectory());
    REQUIRE(editorDirectory.findChildFiles(juce::File::findFiles, false, "*.h").isEmpty());
    REQUIRE(editorDirectory.findChildFiles(juce::File::findFiles, false, "*.cpp").isEmpty());
}

TEST_CASE("Settings feature starts with view-only folder structure", "[ui][architecture]") {
    const auto settingsDirectory = juce::File(BSA_REPO_ROOT)
                                       .getChildFile("src")
                                       .getChildFile("ui")
                                       .getChildFile("settings");
    REQUIRE(settingsDirectory.isDirectory());
    REQUIRE(settingsDirectory.getChildFile("view").isDirectory());
    REQUIRE(settingsDirectory.findChildFiles(juce::File::findFiles, false, "*.h").isEmpty());
    REQUIRE(settingsDirectory.findChildFiles(juce::File::findFiles, false, "*.cpp").isEmpty());
}
