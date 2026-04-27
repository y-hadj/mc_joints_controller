#include "RandomJointsExample_Initial.h"

#include <random>

#include "../RandomJointsExample.h"

void RandomJointsExample_Initial::configure(
    const mc_rtc::Configuration& config) {}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) out.push_back(item);
    return out;
}

void RandomJointsExample_Initial::start(mc_control::fsm::Controller& ctl) {
  // store initial robot configuration to always start from it when switching
  // back to the qp otherwise we might start from an invalid configuration
  auto initialQ = ctl.robot().mbc().q;

  ctl.gui()->addElement(
      this, {},
      mc_rtc::gui::Checkbox(
          "Disable QP",
          [&ctl]() -> bool {
            return ctl.datastore().get<bool>("RandomJointsExample::DisableQP");
          },
          [&ctl, initialQ]() {
            auto& disableqp =
                ctl.datastore().get<bool>("RandomJointsExample::DisableQP");
            ctl.robot().mbc().q = initialQ;
            disableqp = !disableqp;
          }),
      mc_rtc::gui::IntegerInput(
          "Pause Iterations", [this]() { return pauseIter_; },
          [this](int v) { pauseIter_ = v; }));
}

bool RandomJointsExample_Initial::run(mc_control::fsm::Controller& ctl) {
  // pause for N iterations
  if (++iter_ % pauseIter_ == 0) {
    auto& robot = ctl.robot();
    const auto& rjo = robot.refJointOrder();

    if (!fromCSV){
      // mc_rtc::log::info("Choosing Mode: Loading random joint data");
      randomJoints_ = Eigen::VectorXd(rjo.size());
      
      // Modern C++ random engine
      std::random_device rd;
      std::mt19937 gen(rd());

      // Update postureTask (qp)
      auto& postureTask = *ctl.getPostureTask(robot.name());
      static auto posture = postureTask.posture();

      for (size_t i = 0; i < randomJoints_.size(); ++i) {
        auto jointIndexInMbc = robot.jointIndexInMBC(i);
        double qMin = robot.ql()[jointIndexInMbc][0];
        double qMax = robot.qu()[jointIndexInMbc][0];
        std::uniform_real_distribution<double> dist(qMin, qMax);
        randomJoints_[i] = dist(gen);

        // Directly update output joints (no qp)
        if (ctl.datastore().get<bool>("RandomJointsExample::DisableQP")) {
          robot.mbc().q[jointIndexInMbc][0] = randomJoints_[i];
        }

        posture[jointIndexInMbc][0] = randomJoints_[i];
        postureTask.posture(posture);
      }
    }else{
      // mc_rtc::log::info("Choosing Mode: Loading csv joint data");

      // pase data (TODO. find better place for this so that it won't be parsed @ every run iteration)
      std::ifstream csv("data/Cmap_joints.csv"); //supposed format: all qi + flag to indicate last qi of the interp
      std::string header; std::getline(csv, header);
      std::vector<std::string> out_lines, in_lines;
      std::string line;
      while (std::getline(csv, line)) in_lines.push_back(line);

      // for each IK solution
      for (size_t j = 0; j < in_lines.size(); ++j)  
      {
        auto tokens = split(in_lines[j], ',');
        int kf = std::stod(tokens[0]);
        
        // for each qi of the IK sol
        for (size_t i = 0; i < randomJoints_.size(); ++i) { 
          csvJoints_[j][i] = std::stod(tokens[i+1]);

          auto& postureTask = *ctl.getPostureTask(robot.name());
          static auto posture = postureTask.posture();
          
          auto jointIndexInMbc = robot.jointIndexInMBC(i);
          
          if (ctl.datastore().get<bool>("RandomJointsExample::DisableQP")) {
            robot.mbc().q[jointIndexInMbc][0] = csvJoints_[j][i];
          }

          posture[jointIndexInMbc][0] = csvJoints_[j][i];
          postureTask.posture(posture);
        }
      }
    }
  }
  output("OK");
  return false;
}

void RandomJointsExample_Initial::teardown(mc_control::fsm::Controller& ctl) {
  ctl.gui()->removeElements(this);
}

EXPORT_SINGLE_STATE("RandomJointsExample_Initial", RandomJointsExample_Initial)

