#include <iostream>
#include <cfloat>
#include "Debugger.h"

typedef int int32;

template <typename T>
Debugger<T>::Debugger(char* inputClassName, char* inputFunctionName): callCounter(0) {
  std::cout.precision(LDBL_DIG + 1);
  std::cout << "New instance of Debugger class..." << std::endl;
  className = inputClassName;
  functionName = inputFunctionName;
}

template <typename T>
Debugger<T>::~Debugger() {
  std::cout << "Destructing Debugger class..." << std::endl;
}

template <typename T>
void Debugger<T>::printSomething(char* something) {
  std::cout << something << std::endl;
}

template <typename T>
void Debugger<T>::printParamVariation(T inputValue, char* inputParameterName, bool isCounting) {
  if (inputValue != previousValue) {
    std::cout << inputParameterName << ": " << std::fixed << inputValue
              << "\t --- Call n. " << callCounter << " of "
              << className << "::" << functionName << std::endl;
    previousValue = inputValue;
  }
  if (isCounting) {
    callCounter += 1;
  }
}

template class Debugger<int32>;
template class Debugger<float>;
template class Debugger<double>;
