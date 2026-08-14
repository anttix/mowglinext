// Copyright 2026 Mowgli Project
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "behaviortree_cpp/bt_factory.h"
#include "mowgli_behavior/bt_context.hpp"
#include "mowgli_behavior/condition_nodes.hpp"
#include "mowgli_behavior/recording_transition.hpp"
#include <gtest/gtest.h>

namespace mowgli_behavior
{
namespace
{

class RclcppEnvironment : public ::testing::Environment
{
public:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr);
    }
  }

  void TearDown() override
  {
    rclcpp::shutdown();
  }
};

::testing::Environment* const rclcpp_env =
    ::testing::AddGlobalTestEnvironment(new RclcppEnvironment());

TEST(RecordingTransition, ClearsOnlyTheCommandBeingCompleted)
{
  const auto now = std::chrono::steady_clock::time_point(std::chrono::seconds(10));
  BTContext context;
  context.current_command = 5;

  completeRecordingCommand(context, 5, now);

  EXPECT_EQ(context.current_command, 0);
  EXPECT_EQ(context.recording_transition_until, now + kRecordingTransitionGrace);
}

TEST(RecordingTransition, PreservesANewerOperatorCommand)
{
  const auto now = std::chrono::steady_clock::time_point(std::chrono::seconds(10));
  BTContext context;
  context.current_command = 2;

  completeRecordingCommand(context, 5, now);

  EXPECT_EQ(context.current_command, 2);
  EXPECT_EQ(context.recording_transition_until, now + kRecordingTransitionGrace);
}

class RecordingTransitionConditionTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    context_ = std::make_shared<BTContext>();
    context_->node = rclcpp::Node::make_shared("test_recording_transition");
    blackboard_ = BT::Blackboard::create();
    blackboard_->set("context", context_);
    factory_.registerNodeType<IsRecordingTransition>("IsRecordingTransition");
    tree_ = factory_.createTreeFromText(
        R"(
          <root BTCPP_format="4">
            <BehaviorTree ID="MainTree">
              <IsRecordingTransition/>
            </BehaviorTree>
          </root>
        )",
        blackboard_);
  }

  std::shared_ptr<BTContext> context_;
  BT::Blackboard::Ptr blackboard_;
  BT::BehaviorTreeFactory factory_;
  BT::Tree tree_;
};

TEST_F(RecordingTransitionConditionTest, SucceedsOnlyBeforeDeadline)
{
  context_->recording_transition_until = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  EXPECT_EQ(tree_.tickOnce(), BT::NodeStatus::SUCCESS);

  context_->recording_transition_until = std::chrono::steady_clock::now() - std::chrono::seconds(1);
  EXPECT_EQ(tree_.tickOnce(), BT::NodeStatus::FAILURE);
}

}  // namespace
}  // namespace mowgli_behavior
