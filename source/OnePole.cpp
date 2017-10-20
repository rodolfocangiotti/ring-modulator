//-----------------------------------------------------------------------------
// OnePole.cpp
// The OnePole class implements a simple one-pole filter.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#include "OnePole.h"

OnePole::OnePole(double aOneCoefficient, double bZeroCoefficient): yT(0.0), yTMinusOne(0.0) {
  aOne = aOneCoefficient;
  bZero = bZeroCoefficient;
}

double OnePole::oneSampleProcessor(double inputSample) {
  yT = inputSample * bZero + yTMinusOne * aOne;
  yTMinusOne = yT;
  return yT;
}
