/**
 * @author Ryan Benasutti, WPI
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "okapi/api/chassis/model/skidSteerModel.hpp"
#include <algorithm>
#include <utility>

namespace okapi {
SkidSteerModel::SkidSteerModel(std::shared_ptr<AbstractMotor> ileftSideMotor,
                               std::shared_ptr<AbstractMotor> irightSideMotor,
                               std::shared_ptr<ContinuousRotarySensor> ileftEnc,
                               std::shared_ptr<ContinuousRotarySensor> irightEnc,
                               const double imaxOutput)
  : leftSideMotor(ileftSideMotor),
    rightSideMotor(irightSideMotor),
    leftSensor(ileftEnc),
    rightSensor(irightEnc),
    maxOutput(imaxOutput) {
}

SkidSteerModel::SkidSteerModel(std::shared_ptr<AbstractMotor> ileftSideMotor,
                               std::shared_ptr<AbstractMotor> irightSideMotor,
                               const double imaxOutput)
  : leftSideMotor(ileftSideMotor),
    rightSideMotor(irightSideMotor),
    leftSensor(ileftSideMotor->getEncoder()),
    rightSensor(irightSideMotor->getEncoder()),
    maxOutput(imaxOutput) {
}

void SkidSteerModel::forward(const double ispeed) const {
  const double speed = std::clamp(ispeed, -1.0, 1.0);
  leftSideMotor->moveVelocity(static_cast<int16_t>(speed * maxOutput));
  rightSideMotor->moveVelocity(static_cast<int16_t>(speed * maxOutput));
}

void SkidSteerModel::driveVector(const double iySpeed, const double izRotation) const {
  // This code is taken from WPIlib. All credit goes to them. Link:
  // https://github.com/wpilibsuite/allwpilib/blob/master/wpilibc/src/main/native/cpp/Drive/DifferentialDrive.cpp#L73
  const double ySpeed = std::clamp(iySpeed, -1.0, 1.0);
  const double zRotation = std::clamp(izRotation, -1.0, 1.0);

  double leftOutput = ySpeed + zRotation;
  double rightOutput = ySpeed - zRotation;
  if (const double maxInputMag = std::max<double>(std::abs(leftOutput), std::abs(rightOutput));
      maxInputMag > 1) {
    leftOutput /= maxInputMag;
    rightOutput /= maxInputMag;
  }

  leftSideMotor->moveVelocity(static_cast<int16_t>(leftOutput * maxOutput));
  rightSideMotor->moveVelocity(static_cast<int16_t>(rightOutput * maxOutput));
}

void SkidSteerModel::rotate(const double ispeed) const {
  const double speed = std::clamp(ispeed, -1.0, 1.0);
  leftSideMotor->moveVelocity(static_cast<int16_t>(speed * maxOutput));
  rightSideMotor->moveVelocity(static_cast<int16_t>(-1 * speed * maxOutput));
}

void SkidSteerModel::stop() {
  leftSideMotor->moveVelocity(0);
  rightSideMotor->moveVelocity(0);
}

void SkidSteerModel::tank(const double ileftSpeed,
                          const double irightSpeed,
                          const double ithreshold) const {
  // This code is taken from WPIlib. All credit goes to them. Link:
  // https://github.com/wpilibsuite/allwpilib/blob/master/wpilibc/src/main/native/cpp/Drive/DifferentialDrive.cpp#L73
  double leftSpeed = std::clamp(ileftSpeed, -1.0, 1.0);
  if (std::abs(leftSpeed) < ithreshold) {
    leftSpeed = 0;
  }

  double rightSpeed = std::clamp(irightSpeed, -1.0, 1.0);
  if (std::abs(rightSpeed) < ithreshold) {
    rightSpeed = 0;
  }

  leftSideMotor->moveVoltage(static_cast<int16_t>(leftSpeed * maxOutput));
  rightSideMotor->moveVoltage(static_cast<int16_t>(rightSpeed * maxOutput));
}

void SkidSteerModel::arcade(const double iySpeed,
                            const double izRotation,
                            const double ithreshold) const {
  // This code is taken from WPIlib. All credit goes to them. Link:
  // https://github.com/wpilibsuite/allwpilib/blob/master/wpilibc/src/main/native/cpp/Drive/DifferentialDrive.cpp#L73
  double ySpeed = std::clamp(iySpeed, -1.0, 1.0);
  if (std::abs(ySpeed) < ithreshold) {
    ySpeed = 0;
  }

  double zRotation = std::clamp(izRotation, -1.0, 1.0);
  if (std::abs(zRotation) < ithreshold) {
    zRotation = 0;
  }

  double maxInput = std::copysign(std::max(std::abs(ySpeed), std::abs(zRotation)), ySpeed);
  double leftOutput = 0;
  double rightOutput = 0;

  if (ySpeed >= 0) {
    if (zRotation >= 0) {
      leftOutput = maxInput;
      rightOutput = ySpeed - zRotation;
    } else {
      leftOutput = ySpeed + zRotation;
      rightOutput = maxInput;
    }
  } else {
    if (zRotation >= 0) {
      leftOutput = ySpeed + zRotation;
      rightOutput = maxInput;
    } else {
      leftOutput = maxInput;
      rightOutput = ySpeed - zRotation;
    }
  }

  leftSideMotor->moveVoltage(static_cast<int16_t>(std::clamp(leftOutput, -1.0, 1.0) * maxOutput));
  rightSideMotor->moveVoltage(static_cast<int16_t>(std::clamp(rightOutput, -1.0, 1.0) * maxOutput));
}

void SkidSteerModel::left(const double ispeed) const {
  leftSideMotor->moveVelocity(static_cast<int16_t>(ispeed * maxOutput));
}

void SkidSteerModel::right(const double ispeed) const {
  rightSideMotor->moveVelocity(static_cast<int16_t>(ispeed * maxOutput));
}

std::valarray<std::int32_t> SkidSteerModel::getSensorVals() const {
  return std::valarray<std::int32_t>{static_cast<std::int32_t>(leftSensor->get()),
                                     static_cast<std::int32_t>(rightSensor->get())};
}

void SkidSteerModel::resetSensors() const {
  leftSensor->reset();
  rightSensor->reset();
}

void SkidSteerModel::setBrakeMode(const AbstractMotor::brakeMode mode) const {
  leftSideMotor->setBrakeMode(mode);
  rightSideMotor->setBrakeMode(mode);
}

void SkidSteerModel::setEncoderUnits(const AbstractMotor::encoderUnits units) const {
  leftSideMotor->setEncoderUnits(units);
  rightSideMotor->setEncoderUnits(units);
}

void SkidSteerModel::setGearing(const AbstractMotor::gearset gearset) const {
  leftSideMotor->setGearing(gearset);
  rightSideMotor->setGearing(gearset);
}

void SkidSteerModel::setPosPID(double ikF, double ikP, double ikI, double ikD) const {
  leftSideMotor->setPosPID(ikF, ikP, ikI, ikD);
  rightSideMotor->setPosPID(ikF, ikP, ikI, ikD);
}

void SkidSteerModel::setPosPIDFull(double ikF,
                                   double ikP,
                                   double ikI,
                                   double ikD,
                                   double ifilter,
                                   double ilimit,
                                   double ithreshold,
                                   double iloopSpeed) const {
  leftSideMotor->setPosPIDFull(ikF, ikP, ikI, ikD, ifilter, ilimit, ithreshold, iloopSpeed);
  rightSideMotor->setPosPIDFull(ikF, ikP, ikI, ikD, ifilter, ilimit, ithreshold, iloopSpeed);
}

void SkidSteerModel::setVelPID(double ikF, double ikP, double ikI, double ikD) const {
  leftSideMotor->setVelPID(ikF, ikP, ikI, ikD);
  rightSideMotor->setVelPID(ikF, ikP, ikI, ikD);
}

void SkidSteerModel::setVelPIDFull(double ikF,
                                   double ikP,
                                   double ikI,
                                   double ikD,
                                   double ifilter,
                                   double ilimit,
                                   double ithreshold,
                                   double iloopSpeed) const {
  leftSideMotor->setVelPIDFull(ikF, ikP, ikI, ikD, ifilter, ilimit, ithreshold, iloopSpeed);
  rightSideMotor->setVelPIDFull(ikF, ikP, ikI, ikD, ifilter, ilimit, ithreshold, iloopSpeed);
}

std::shared_ptr<AbstractMotor> SkidSteerModel::getLeftSideMotor() const {
  return leftSideMotor;
}

std::shared_ptr<AbstractMotor> SkidSteerModel::getRightSideMotor() const {
  return rightSideMotor;
}
} // namespace okapi
