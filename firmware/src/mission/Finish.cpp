#include "Finish.h"

#include <control/MotionController.h>
#include <control/SlotManager.h>
#include <drivers/LinearActuator.h>
#include <drivers/OLED.h>

Finish::Finish()
  : _motionController(nullptr),
    _slotManager(nullptr),
    _linearActuator(nullptr),
    _oled(nullptr),
    _reportStream(nullptr),
    _ready(false),
    _active(false),
    _completed(false),
    _finishedAtMs(0) {}

bool Finish::begin(MotionController& motionController,
                   SlotManager* slotManager,
                   LinearActuator* linearActuator,
                   OLED* oled,
                   Stream* reportStream) {
  _motionController = &motionController;
  _slotManager = slotManager;
  _linearActuator = linearActuator;
  _oled = oled;
  _reportStream = reportStream;

  _active = false;
  _completed = false;
  _finishedAtMs = 0;
  _ready = _motionController->ready();

  return _ready;
}

void Finish::enter() {
  if (!_ready || _active) return;

  _active = true;
  _completed = false;
  executeFinishActions();
}

void Finish::update() {
  if (!_ready || !_active) return;

  // Finish durumunda hareket cikislari surekli kapali tutulur.
  _motionController->stop();

  if (_linearActuator != nullptr) {
    _linearActuator->stop();
  }
}

void Finish::exit() {
  _active = false;
}

void Finish::reset() {
  _active = false;
  _completed = false;
  _finishedAtMs = 0;
}

bool Finish::ready() const {
  return _ready;
}

bool Finish::isActive() const {
  return _active;
}

bool Finish::isCompleted() const {
  return _completed;
}

uint32_t Finish::finishedAtMs() const {
  return _finishedAtMs;
}

void Finish::executeFinishActions() {
  _motionController->stop();

  if (_linearActuator != nullptr) {
    _linearActuator->stop();
  }

  if (_oled != nullptr && _oled->ready()) {
    String slotText = "";

    if (_slotManager != nullptr) {
      slotText = "Dolu slot: " + String(static_cast<unsigned int>(_slotManager->occupiedCount())) +
                 "/" + String(static_cast<unsigned int>(_slotManager->slotCount()));
    }

    _oled->status("GOREV TAMAMLANDI", slotText, "Robot durduruldu");
  }

  if (_reportStream != nullptr) {
    _reportStream->println(F("MISSION,FINISHED"));
  }

  _finishedAtMs = millis();
  _completed = true;
}
