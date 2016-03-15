/*
 * BaseTrajectory.cpp
 *
 *  Created on: Mar 11, 2015
 *      Author: Péter Fankhauser
 *   Institute: ETH Zurich, Autonomous Systems Lab
 */

#include <free_gait_core/base_motion/BaseTrajectory.hpp>

namespace free_gait {

BaseTrajectory::BaseTrajectory()
    : BaseMotionBase(BaseMotionBase::Type::Trajectory),
      duration_(0.0),
      isComputed_(false)
{
}

BaseTrajectory::~BaseTrajectory()
{
}

std::unique_ptr<BaseMotionBase> BaseTrajectory::clone() const
{
  std::unique_ptr<BaseMotionBase> pointer(new BaseTrajectory(*this));
  return pointer;
}

const ControlSetup BaseTrajectory::getControlSetup() const
{
  return controlSetup_;
}

void BaseTrajectory::updateStartPose(const Pose& startPose)
{
  isComputed_ = false;
  auto& values = values_.at(ControlLevel::Position);
  auto& times = times_.at(ControlLevel::Position);
  if (times[0] == 0.0) {
    values[0] = startPose;
  } else {
    times.insert(times.begin(), 0.0);
    values.insert(values.begin(), startPose);
  }
}

bool BaseTrajectory::compute(const State& state, const Step& step, const StepQueue& queue,
                             const AdapterBase& adapter)
{
  trajectory_.fitCurve(times_[ControlLevel::Position], values_[ControlLevel::Position]);

  duration_ = 0.0;
  for (const auto& times : times_) {
    if (times.second.back() > duration_) duration_ = times.second.back();
  }

  isComputed_ = true;
  return true;
}

bool BaseTrajectory::isComputed() const
{
  return isComputed_;
}

double BaseTrajectory::getDuration() const
{
  return duration_;
}

const std::string& BaseTrajectory::getFrameId(const ControlLevel& controlLevel) const
{
  return frameIds_.at(controlLevel);
}

Pose BaseTrajectory::evaluatePose(const double time) const
{
  return Pose(trajectory_.evaluate(time));
}

std::ostream& operator<<(std::ostream& out, const BaseTrajectory& baseTrajectory)
{
  out << "Duration: " << baseTrajectory.duration_ << std::endl;
  return out;
}

} /* namespace */
