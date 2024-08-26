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
MidiUsbTransport midiusb_out;
Color red, white;

void handlerMidiEvent(MidiEvent ev) {
  if (ev.type == NoteOn) {
    hw.led2.SetColor(red);
    hw.led2.Update();
  } else if (ev.type == NoteOff) {
    hw.led2.SetColor(white);
    hw.led2.Update();
  }
}

void sendSysexString(char* str) {
  // build sysex message from str
  uint8_t sysex_message[128];
  sysex_message[0] = 0xF0;  // sysex start
  for (size_t i = 0; i < strlen(str); i++) {
    sysex_message[i + 1] = str[i];
  }
  sysex_message[strlen(str) + 1] = 0xF7;  // sysex end

  midiusb_out.Tx(sysex_message, strlen(str) + 2);
}

bool led_state = true;
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  // every second print out a MIDI message
  if (timer_second.Process()) {
    // uint8_t msg[3] = {0x90, 0x40, 0x7F};
    // hw.midi.SendMessage(msg, 3);
    // midiusb.SendMessage(msg, 3);
    char str[128];
    sprintf(str, "hi");
    sendSysexString(str);
    hw.led1.SetColor(led_state ? red : white);
    hw.led1.Update();
    led_state = !led_state;
  }
  // while (hw.midi.HasEvents()) {
  //   handlerMidiEvent(hw.midi.PopEvent());
  // }
  // while (midiusb.HasEvents()) {
  //   handlerMidiEvent(midiusb.PopEvent());
  // }
}

// void StartRx(MidiRxParseCallback callback, void* context)

// create callback function for MidiRxParseCallback
void handlerUSBMidiEvent(uint8_t* data, size_t size, void* context) {
  char str[128];
  sprintf(str, "USB MIDI: ");
  for (size_t i = 0; i < size; i++) {
    sprintf(str + strlen(str), "%02X ", data[i]);
  }
  sendSysexString(str);
}

int main(void) {
  hw.Init();

  // initialize midi
  MidiUsbHandler::Config midiusb_config;
  midiusb.Init(midiusb_config);
  midiusb.StartReceive();
  MidiUsbTransport::Config midiusb_out_config;
  midiusb_out.Init(midiusb_out_config);
  midiusb_out.StartRx(handlerUSBMidiEvent, nullptr);

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
