//-----------------------------------------------------------------------------
// OnePole.h
// The OnePole class implements a simple one-pole filter.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#pragma once

class OnePole {
public:
  OnePole(double aOneCoefficient, double bZeroCoefficient);
  double oneSampleProcessor(double inputSample);
private:
  double yT;
  double yTMinusOne;
  double aOne;
  double bZero;
};
