#include "daisy_pod.h"
#include "daisysp.h"
//
#include "daisy_midi.h"

using namespace daisy;
using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BLOCK_SIZE 128

DaisyPod hw;
DaisyMidi daisy_midi;
Metro timer_second;
Color red, white;

bool led_state = true;
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  // every second print out a MIDI message
  if (timer_second.Process()) {
    hw.led1.SetColor(led_state ? red : white);
    hw.led1.Update();
    led_state = !led_state;
  }
  while (hw.midi.HasEvents()) {
    daisy_midi.handlerHWMidiEvent(hw.midi.PopEvent());
  }
}

int main(void) {
  hw.Init();

  // initialize midi
  daisy_midi.Init();
  daisy_midi.SetNoteOnCallback(
      [](uint8_t channel, uint8_t note, uint8_t velocity) {
        daisy_midi.sysex_printf("NoteOn: %d %d %d", channel, note, velocity);
      });
  daisy_midi.SetNoteOffCallback(
      [](uint8_t channel, uint8_t note, uint8_t velocity) {
        daisy_midi.sysex_printf("NoteOff: %d %d %d", channel, note, velocity);
      });
  daisy_midi.SetMidiTimingCallback(
      []() { daisy_midi.sysex_printf("midi timing2"); });
  daisy_midi.SetSysExCallback([](const uint8_t* data, size_t size) {
    if (size == 3 && data[0] == 'a' && data[1] == 'b' && data[2] == 'c') {
      daisy_midi.sysex_printf("got abc");
    } else {
      daisy_midi.sysex_printf("%s (%d)", size, data);
    }
  });

  // initialize led
  red.Init(Color::PresetColor::RED);
  white.Init(Color::PresetColor::WHITE);

  // initialize metro
  timer_second.Init(1.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);

  // start audio
  hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  hw.StartAdc();
  hw.StartAudio(AudioCallback);
  while (1) {
  }
}
