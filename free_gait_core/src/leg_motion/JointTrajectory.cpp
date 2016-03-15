/*
 * JointTrajectory.cpp
 *
 *  Created on: Nov 8, 2015
 *      Author: Péter Fankhauser
 *   Institute: ETH Zurich, Autonomous Systems Lab
 */
#include <free_gait_core/leg_motion/JointTrajectory.hpp>

// STD
#include <thread>

namespace free_gait {

JointTrajectory::JointTrajectory(LimbEnum limb)
    : JointMotionBase(LegMotionBase::Type::JointTrajectory, limb),
      ignoreContact_(false),
      duration_(0.0),
      isComputed_(false)
{
}

JointTrajectory::JointTrajectory(const JointTrajectory& other)
    : JointMotionBase(LegMotionBase::Type::JointTrajectory, other.limb_),
      isComputed_(isComputed_.load()),
      ignoreContact_(other.ignoreContact_),
      controlSetup_(other.controlSetup_),
      times_(other.times_),
      values_(other.values_),
      duration_(other.duration_),
      trajectories_(other.trajectories_)
{
}

JointTrajectory::~JointTrajectory()
{
}

std::unique_ptr<LegMotionBase> JointTrajectory::clone() const
{
  std::unique_ptr<LegMotionBase> pointer(new JointTrajectory(*this));
  return pointer;
}

const ControlSetup JointTrajectory::getControlSetup() const
{
  return controlSetup_;
}

void JointTrajectory::updateStartPosition(const JointPositions& startPosition)
{
  isComputed_ = false;
  auto& values = values_.at(ControlLevel::Position);
  auto& times = times_.at(ControlLevel::Position);
  if (times[0] == 0.0) {
    for (size_t i = 0; i < values.size(); ++i) {
      values[i][0] = startPosition(i);
    }
  } else {
    times.insert(times.begin(), 0.0);
    for (size_t i = 0; i < values.size(); ++i) {
      values[i].insert(values[i].begin(), startPosition(i));
    }
  }
}

void JointTrajectory::updateStartVelocity(const JointVelocities& startVelocity)
{
  throw std::runtime_error("JointTrajectory::updateStartVelocity() not implemented.");
}

void JointTrajectory::updateStartAcceleration(const JointAccelerations& startAcceleration)
{
  throw std::runtime_error("JointTrajectory::updateStartAcceleration() not implemented.");
}

void JointTrajectory::updateStartEfforts(const JointEfforts& startEffort)
{
  isComputed_ = false;
  auto& values = values_.at(ControlLevel::Effort);
  auto& times = times_.at(ControlLevel::Effort);
  if (times[0] == 0.0) {
    for (size_t i = 0; i < values.size(); ++i) {
      values[i][0] = startEffort(i);
    }
  } else {
    times.insert(times.begin(), 0.0);
    for (size_t i = 0; i < values.size(); ++i) {
      values[i].insert(values[i].begin(), startEffort(i));
    }
  }
}

bool JointTrajectory::compute(const State& state, const Step& step, const AdapterBase& adapter)
{
  isComputed_ = false;
  duration_ = 0.0;
  for (const auto& times : times_) {
    if (times.second.back() > duration_) duration_ = times.second.back();
  }

  std::thread thread(&JointTrajectory::fitTrajectories, this);
  thread.detach();
  return true;
}

bool JointTrajectory::isComputed() const
{
  return isComputed_;
}

bool JointTrajectory::requiresMultiThreading() const
{
  return false;
}

double JointTrajectory::getDuration() const
{
  return duration_;
}

const JointPositions JointTrajectory::evaluatePosition(const double time) const
{
  if (!isComputed_) throw std::runtime_error("JointTrajectory::evaluatePosition() cannot be called if trajectory is not computed.");
  const auto& trajectories = trajectories_.at(ControlLevel::Position);
  JointPositions jointPositions;
  jointPositions.toImplementation().resize((unsigned int) trajectories.size());
  for (size_t i = 0; i < trajectories.size(); ++i) {
    jointPositions(i) = trajectories[i].evaluate(time);
  }
  return jointPositions;
}

const JointVelocities JointTrajectory::evaluateVelocity(const double time) const
{
  if (!isComputed_) throw std::runtime_error("JointTrajectory::evaluateVelocity() cannot be called if trajectory is not computed.");
  throw std::runtime_error("JointTrajectory::evaluateVelocity() not implemented.");
}

const JointAccelerations JointTrajectory::evaluateAcceleration(const double time) const
{
  if (!isComputed_) throw std::runtime_error("JointTrajectory::evaluateAcceleration() cannot be called if trajectory is not computed.");
  throw std::runtime_error("JointTrajectory::evaluateAcceleration() not implemented.");
}

const JointEfforts JointTrajectory::evaluateEffort(const double time) const
{
  if (!isComputed_) throw std::runtime_error("JointTrajectory::evaluateEffort() cannot be called if trajectory is not computed.");
  const auto& trajectories = trajectories_.at(ControlLevel::Effort);
  JointEfforts jointEfforts;
  jointEfforts.toImplementation().resize((unsigned int) trajectories.size());
  for (size_t i = 0; i < trajectories.size(); ++i) {
    jointEfforts(i) = trajectories[i].evaluate(time);
  }
  return jointEfforts;
}

bool JointTrajectory::isIgnoreContact() const
{
  return ignoreContact_;
}

bool JointTrajectory::fitTrajectories()
{
  for (auto& trajectories : trajectories_) {
    trajectories.second.resize(values_.at(trajectories.first).size());
    for (size_t i = 0; i < values_.at(trajectories.first).size(); ++i) {
      trajectories.second[i].fitCurve(times_.at(trajectories.first), values_.at(trajectories.first)[i]);
    }
  }

  usleep(5e6);

  isComputed_ = true;
  return true;
}

std::ostream& operator<<(std::ostream& out, const JointTrajectory& jointTrajectory)
{
  if (!jointTrajectory.isComputed()) throw std::runtime_error("JointTrajectory::operator<< cannot be called if trajectory is not computed.");
  out << "Duration: " << jointTrajectory.duration_ << std::endl;
  out << "Ignore contact: " << (jointTrajectory.ignoreContact_ ? "True" : "False") << std::endl;
  for (const auto& times : jointTrajectory.times_) {
    out << "Times (" << times.first << "): ";
    for (const auto& time : times.second) out << time << ", ";
    out << std::endl;
  }
  return out;
}

} /* namespace */
