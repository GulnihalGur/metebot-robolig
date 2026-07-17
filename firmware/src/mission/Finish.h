#pragma once

#include <Arduino.h>

class MotionController;
class SlotManager;
class LinearActuator;
class OLED;

// Gorev tamamlandiginda sistemi guvenli durumda tutar.
class Finish {
public:
  Finish();

  // Zorunlu bagimlilik MotionController'dir.
  // Diger moduller kullanilmiyorsa nullptr birakilabilir.
  bool begin(MotionController& motionController,
             SlotManager* slotManager = nullptr,
             LinearActuator* linearActuator = nullptr,
             OLED* oled = nullptr,
             Stream* reportStream = &Serial);

  // Finish durumuna girerken bir kez cagrilir.
  void enter();

  // Finish aktifken loop icinde cagrilir.
  void update();

  // Finish durumundan cikarken cagrilir.
  void exit();

  // Yeni gorevden once durum bilgisini temizler.
  void reset();

  bool ready() const;
  bool isActive() const;
  bool isCompleted() const;
  uint32_t finishedAtMs() const;

private:
  MotionController* _motionController;
  SlotManager* _slotManager;
  LinearActuator* _linearActuator;
  OLED* _oled;
  Stream* _reportStream;

  bool _ready;
  bool _active;
  bool _completed;
  uint32_t _finishedAtMs;

  void executeFinishActions();
};
