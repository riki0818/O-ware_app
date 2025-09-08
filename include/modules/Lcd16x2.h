#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "rgb_lcd.h"

template <class ModuleType>
class Lcd16X2 : public rgb_lcd {
 private:
  ModuleType& _module;
  int currentRow = 0;
  const int nrows = 2;
  const int ncols = 16;

 public:
  Lcd16X2(ModuleType& module);
  void init();
  bool isConnected();
  void clear();
  template <typename T>
  void append(T text, bool alignRight = false);
  template <typename T>
  void newLine(int line, T text, bool alignRight = false);
};

template <class ModuleType>
Lcd16X2<ModuleType>::Lcd16X2(ModuleType& module) : _module(module) {}

template <class ModuleType>
void Lcd16X2<ModuleType>::init() {
  Wire.begin();
  if (isConnected()) {
    begin(ncols, nrows);
    clear();
  }
}

template <class ModuleType>
void Lcd16X2<ModuleType>::clear() {
  if (isConnected()) {
    rgb_lcd::clear();
  }
}

template <class ModuleType>
bool Lcd16X2<ModuleType>::isConnected() {
  byte error;
  Wire.beginTransmission(LCD_ADDRESS);
  error = Wire.endTransmission();
  return error == 0;
}

template <class ModuleType>
template <typename T>
void Lcd16X2<ModuleType>::append(T text, bool alignRight) {
  if (isConnected()) {
    String strText = String(text);
    if (alignRight) {
      setCursor(ncols - strText.length(), currentRow);
      print(strText);
    } else {
      print(String(text));
    }
  }
}

template <class ModuleType>
template <typename T>
void Lcd16X2<ModuleType>::newLine(int row, T text, bool alignRight) {
  currentRow = row;
  if (isConnected()) {
    String strText = String(text);
    if (alignRight) {
      setCursor(ncols - strText.length(), row);
      print(strText);
    } else {
      setCursor(0, row);
      print(strText);
    }
  }
}
