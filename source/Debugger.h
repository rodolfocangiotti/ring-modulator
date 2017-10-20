#pragma once

typedef unsigned int uint32;

template <typename T>
class Debugger {
public:
  Debugger(char* inputClassName, char* inputFunctionName);
  ~Debugger();
  void printSomething(char* something);
  void printParamVariation(T inputValue, char* inputParameterName, bool isCounting = false);
private:
  char* className;
  char* functionName;
  uint32 callCounter;
  T previousValue;
};
