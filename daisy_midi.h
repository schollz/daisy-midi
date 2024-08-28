#include "daisy_pod.h"

using namespace daisy;

#define MIDI_RT_CLOCK 0xF8
#define MIDI_RT_START 0xFA
#define MIDI_RT_CONTINUE 0xFB
#define MIDI_RT_STOP 0xFC
#define MIDI_RT_ACTIVESENSE 0xFE
#define MIDI_RT_RESET 0xFF
#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90
#define MIDI_AFTERTOUCH 0xA0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0
#define MIDI_CHANNEL_PRESSURE 0xD0
#define MIDI_PITCH_BEND 0xE0
#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_END 0xF7
#define MIDI_TIMING 0xF8

class DaisyMidi {
 public:
  DaisyMidi() {}
  void Init() {
    // initialize midi
    MidiUsbTransport::Config midiusb_out_config;
    midiusb_out.Init(midiusb_out_config);
    midiusb_out.StartRx(
        [](uint8_t* data, size_t size, void* context) {
          static_cast<DaisyMidi*>(context)->handlerUSBMidiEvent(data, size);
        },
        this);
  }

  void send_sysex(char* str) {
    // build sysex message from str
    uint8_t sysex_message[128];
    sysex_message[0] = 0xF0;  // sysex start
    for (size_t i = 0; i < strlen(str); i++) {
      sysex_message[i + 1] = str[i];
    }
    sysex_message[strlen(str) + 1] = 0xF7;  // sysex end

    midiusb_out.Tx(sysex_message, strlen(str) + 2);
  }

  void handlerHWMidiEvent(MidiEvent ev) {}

 private:
  MidiUsbTransport midiusb_out;
  char midi_buffer[128];
  size_t midi_buffer_index = 0;
  bool in_sysex_message = false;

  void handlerUSBMidiEvent(uint8_t* data, size_t size) {
    // Iterate through the incoming MIDI data
    if (!in_sysex_message) {
      char str[128];
      sprintf(str, "MIDI message:");
      // print all data
      for (size_t i = 0; i < size; i++) {
        sprintf(str + strlen(str), "%02X", data[i]);
      }
      switch (data[0]) {
        case MIDI_RT_CLOCK:
        case MIDI_NOTE_ON:
        case MIDI_NOTE_OFF:
          send_sysex(str);
          return;
        default:
          break;
      }
    }
    // SYSEX
    for (size_t i = 0; i < size; ++i) {
      uint8_t byte = data[i];

      if (in_sysex_message) {
        // We are in the middle of a SysEx message
        midi_buffer[midi_buffer_index++] = byte;

        if (byte == MIDI_SYSEX_END ||
            midi_buffer_index >= sizeof(midi_buffer)) {
          // end of sysex message
          in_sysex_message = false;
          midi_buffer_index = 0;
          send_sysex(midi_buffer);
        }
      } else if (byte == MIDI_SYSEX_START) {
        // Start of a SysEx message
        in_sysex_message = true;
        midi_buffer_index = 0;
        midi_buffer[midi_buffer_index++] = byte;
      }
    }
  }
};
