#pragma once

#include <mc_control/fsm/State.h>

struct RandomJointsExample_Initial : mc_control::fsm::State {
  void configure(const mc_rtc::Configuration& config) override;
  
  std::vector<std::string> split(const std::string& s, char delim);
  
  void start(mc_control::fsm::Controller& ctl) override;

  bool run(mc_control::fsm::Controller& ctl) override;

  void teardown(mc_control::fsm::Controller& ctl) override;

 protected:
  Eigen::VectorXd randomJoints_;
  unsigned pauseIter_ = 100;
  unsigned iter_ = 0;
  std::vector<Eigen::VectorXd> csvJoints_;
  bool fromCSV = true;
};
