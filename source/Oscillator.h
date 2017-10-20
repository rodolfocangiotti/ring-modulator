//-----------------------------------------------------------------------------
// Oscillator.h
// The Oscillator class implements a sinusoidal oscillator.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#pragma once

typedef int int32;
typedef unsigned int uint32;
// RIFERIMENTO A TIPI DI DATO DEFINITI IN pluginterfaces/base/ftypes.h

class Oscillator {
public:
  Oscillator(uint32 inputBufferLength, uint32 inputSampleRate);
  ~Oscillator();
  double oneSampleProcessor(double inputFrequency);
  void resetPhase();
private:
  double* buffer;
  uint32 bufferLength;
  uint32 sampleRate;
  double phase;
  double phaseIncrement;
  int32 intPhaseZero;
  int32 intPhaseOne;
  double fractionalPhase;
  double output;
};
