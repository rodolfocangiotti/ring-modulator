//-----------------------------------------------------------------------------
// LICENSE
// (c) 2017, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ringModulatorProcessor.h"
#include "ringModulatorIDs.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "OnePole.h"
#include "Oscillator.h"
#include "Ramp.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
ringModulatorProcessor::ringModulatorProcessor(): bypass(true), modulationFrequency(0.0), modulationAmplitude(0.0) {
  setControllerClass(ringModulatorControllerUID);
  frequencyOnePole = new OnePole(0.998, 0.002);
  frequencyRamp = new Ramp(128);
  amplitudeOnePole = new OnePole(0.998, 0.002);
  amplitudeRamp = new Ramp(128);
  oscillator = new Oscillator(2048, processSetup.sampleRate);
}

//-----------------------------------------------------------------------------
ringModulatorProcessor::~ringModulatorProcessor() {
  delete frequencyOnePole;
  delete frequencyRamp;
  delete amplitudeOnePole;
  delete amplitudeRamp;
  delete oscillator;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::initialize(FUnknown* context) {
  tresult result = AudioEffect::initialize(context);
  if (result == kResultTrue) {
    addAudioInput(USTRING("AudioInput"), SpeakerArr::kStereo);
    addAudioOutput(USTRING("AudioOutput"), SpeakerArr::kStereo);
  }
  return result;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) {
  // ISTANZIO UN BUS ARRANGEMENT CONSISTENTE IN UN SOLO INPUT STEREO ED UN OUTPUT STEREO...
  if (numIns == 1 && numOuts == 1 && inputs[0] == SpeakerArr::kStereo && outputs[0] == SpeakerArr::kStereo) {
    return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
  }
  return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::setActive(TBool state) {
  SpeakerArrangement arr;
  // VERIFICO CHE IL CORRENTE BUFFER SIA CONNESSO ALL'OUTPUT...
  if (getBusArrangement(kOutput, 0, arr) != kResultTrue) {
    return kResultFalse;
  }
  // VERIFICO L'EFFETTIVA PRESENZA DI CANALI LEGATI ALL'OUTPUT...
  int32 numChannels = SpeakerArr::getChannelCount(arr);
  if (numChannels == 0) {
    return kResultFalse;
  }

  // SE IL PLUG-IN NON È ATTIVO...
  if (!state) {
    oscillator->resetPhase();
  }

  return AudioEffect::setActive(state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::process(ProcessData& data) {
  // AGGIORNO I PARAMETRI DEL COMPONENTE PROCESSOR FACENDO RIFERIMENTO AL COMPONENTE CONTROLLER...
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(index);
      if (paramQueue) {
        ParamValue value;
        int32 sampleOffset;
        int32 numPoints = paramQueue->getPointCount();
        switch (paramQueue->getParameterId()) {
        case kBypass:
          if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
            bypass = (value > 0.5);
          }
          break;
        case kModulationFrequency:
          if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
            modulationFrequency = value * 1000;
            // LO SCAMBIO DI DATI TRA IL CONTROLLER ED IL PROCESSOR AVVIENE CON VALORI NORMALIZZATI...
          }
          break;
        case kModulationAmplitude:
          if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
            modulationAmplitude = value;
          }
          break;
        }
      }
    }
  }

  if (data.numSamples > 0) {
    SpeakerArrangement arr;
    getBusArrangement(kOutput, 0, arr);
    int32 numChannels = SpeakerArr::getChannelCount(arr);

    for (int32 sample = 0; sample < data.numSamples; sample++) {
      // ATTENUO LA VARIAZIONE DEI VALORI modulationFrequency e modulationAmplitude...
      float rampedModFrequency = (float) frequencyRamp->oneSampleProcessor(modulationFrequency);
      float filteredModFrequency = (float) frequencyOnePole->oneSampleProcessor(rampedModFrequency);
      float rampedModAmplitude = (float) amplitudeRamp->oneSampleProcessor(modulationAmplitude);
      float filteredModAmplitude = (float) amplitudeOnePole->oneSampleProcessor(rampedModAmplitude);
      // PROCESSO IL SAMPLE...
      float sampleValue = (float) oscillator->oneSampleProcessor(filteredModFrequency);
      sampleValue *= filteredModAmplitude;
      for (int32 channel = 0; channel < numChannels; channel++) {
        // ACQUISISCO PUNTATORE AL CANALE DI INPUT E DI OUTPUT...
        float* inputChannel = data.inputs[0].channelBuffers32[channel];
        float* outputChannel = data.outputs[0].channelBuffers32[channel];
        // SCRIVO IL VETTORE DI OUTPUT...
        if (bypass) {
          // IL VETTORE IN OUTPUT È IDENTICO AL VETTORE IN INPUT...
          outputChannel[sample] = inputChannel[sample];
        } else {
          // IL VETTORE OUTPUT È MODULATO IN AMPIEZZA...
          outputChannel[sample] = inputChannel[sample] * sampleValue;
        }
      }
    }
  }
  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::setState(IBStream* state) {
  if (!state) {
    return kResultFalse;
  }

  // called when we load a preset, the model has to be reloaded
  int32 savedBypass = 0;
  if (state->read(&savedBypass, sizeof(int32)) != kResultOk) {
    // could be an old version, continue
  }

  float savedModulationFrequency = 0.0;
  if (state->read(&savedModulationFrequency, sizeof(float)) != kResultOk) {
    return kResultFalse;
  }

  float savedModulationAmplitude = 0.0;
  if (state->read(&savedModulationAmplitude, sizeof(float)) != kResultOk) {
    return kResultFalse;
  }

#if BYTEORDER == kBigEndian
  SWAP_32(savedBypass)
  SWAP_32(savedModulationFrequency)
  SWAP_32(savedModulationAmplitude)
#endif

  bypass = savedBypass > 0;
  modulationFrequency = savedModulationFrequency;
  modulationAmplitude = savedModulationAmplitude;

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ringModulatorProcessor::getState(IBStream* state) {

  // here we need to save the model
  int32 toSaveBypass = bypass ? 1 : 0;
  float toSaveModulationFrequency = modulationFrequency;
  float toSaveModulationAmplitude = modulationAmplitude;

#if BYTEORDER == kBigEndian
  SWAP_32(toSaveBypass)
  SWAP_32(toSaveModulationFrequency)
  SWAP_32(toSaveModulationAmplitude)
#endif

  state->write(&toSaveBypass, sizeof(int32));
  state->write(&toSaveModulationFrequency, sizeof(float));
  state->write(&toSaveModulationAmplitude, sizeof(float));

  return kResultOk;
}

} // namespace Vst
} // namespace Steinberg
