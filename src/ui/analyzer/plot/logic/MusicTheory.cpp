#include "MusicTheory.h"

#include <array>
#include <cmath>

juce::String MusicTheory::getNearestNoteName(float frequencyHz) const {
    static constexpr std::array<const char *, 12> noteNames{
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    // MIDI note 69 is A4 = 440 Hz
    // Every octave is a doubling of frequency, so equal temperament uses log2
    // We round to the nearest semitone so the tooltip shows the closest note name
    const auto midiNote = static_cast<int>(std::round(69.0 + 12.0 * std::log2(frequencyHz / 440.0f)));
    // Wrap any MIDI note number back into the 12 pitch classes C..B
    const auto wrappedIndex = ((midiNote % 12) + 12) % 12;
    // MIDI octave numbering puts C4 at note 60, so octave = floor(note / 12) - 1
    const auto octave = (midiNote / 12) - 1;
    return juce::String(noteNames[static_cast<size_t>(wrappedIndex)]) + juce::String(octave);
}
