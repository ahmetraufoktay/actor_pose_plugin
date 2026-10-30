#pragma once

#include <gz/msgs/pose_v.pb.h>
#include <gz/sim/System.hh>
#include <gz/transport/Node.hh>

namespace custom_plugins {
class ActorPosePublisher : public gz::sim::System,
                           public gz::sim::ISystemConfigure,
                           public gz::sim::ISystemPostUpdate {
  public:
    void Configure(const gz::sim::Entity &_entity,
                   const std::shared_ptr<const sdf::Element> &_sdf,
                   gz::sim::EntityComponentManager &_ecm,
                   gz::sim::EventManager &_eventMgr) override;

  public:
    void PostUpdate(const gz::sim::UpdateInfo &_info,
                    const gz::sim::EntityComponentManager &_ecm) override;

  private:
    gz::transport::Node node;

  private:
    gz::transport::Node::Publisher posesPub;

  private:
    std::string frameId{"world"};
};
} // namespace custom_plugins
