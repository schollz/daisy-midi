#include "daisy_pod.h"
#include "daisysp.h"
using namespace daisy;
using namespace daisysp;
DaisyPod hw;

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BLOCK_SIZE 128
Metro timer_second;

/* <midi> */
#define MIDI_RT_CLOCK 0xF8
#define MIDI_RT_START 0xFA
#define MIDI_RT_CONTINUE 0xFB
#define MIDI_RT_STOP 0xFC
#define MIDI_RT_ACTIVESENSE 0xFE
#define MIDI_RT_RESET 0xFF

MidiUsbHandler midiusb;
/* </midi> */
Color red, white;
bool led_state = true;
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  // every second print out a MIDI message
  if (timer_second.Process()) {
    uint8_t msg[3] = {0x90, 0x40, 0x7F};
    hw.midi.SendMessage(msg, 3);
    midiusb.SendMessage(msg, 3);
    hw.led1.SetColor(led_state ? red : white);
    hw.led1.Update();
    led_state = !led_state;
  }
}

int main(void) {
  hw.Init();

  // initialize midi
  MidiUsbHandler::Config midiusb_config;
  midiusb.Init(midiusb_config);
  midiusb.StartReceive();

  // initialize led
  red.Init(Color::PresetColor::RED);
  white.Init(Color::PresetColor::WHITE);

  // initialize metro
  timer_second.Init(1.0f, AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);

  // start audio
  hw.SetAudioBlockSize(128);
  hw.StartAdc();
  hw.StartAudio(AudioCallback);
  while (1) {
  }
}
