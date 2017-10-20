//-----------------------------------------------------------------------------
// Oscillator.cpp
// The Oscillator class implements a sinusoidal oscillator.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#include <cmath>
#include "Oscillator.h"

typedef int int32;
typedef unsigned int uint32;

Oscillator::Oscillator(uint32 inputBufferLength, uint32 inputSampleRate):
phase(0.0), phaseIncrement(0.0),
intPhaseZero(0), intPhaseOne(0), fractionalPhase(0.0) {
  buffer = new double[inputBufferLength + 1];
  for (uint32 i = 0; i < inputBufferLength; i++) {
    buffer[i] = sin(2.0 * M_PI * i / inputBufferLength);
  }
  buffer[inputBufferLength] = 0.0;
  // GUARD POINT...
  bufferLength = inputBufferLength;
  sampleRate = inputSampleRate;
}

Oscillator::~Oscillator() {
  delete[] buffer;
}

double Oscillator::oneSampleProcessor(double inputFrequency) {
  intPhaseZero = (int32) phase;
  intPhaseOne = intPhaseZero + 1;
  fractionalPhase = phase - intPhaseZero;

  output = buffer[intPhaseZero] * (1 - fractionalPhase) + buffer[intPhaseOne] * fractionalPhase;

  phaseIncrement = inputFrequency * bufferLength / sampleRate;
  phase += phaseIncrement;
  if (!(phase < bufferLength)) {
    phase -= bufferLength;
  }

  return output;
}

void Oscillator::resetPhase() {
  phase = 0.0;
}
