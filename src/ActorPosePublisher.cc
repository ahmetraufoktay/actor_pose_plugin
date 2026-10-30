#include "ActorPosePublisher.hh"
#include <gz/msgs/Utility.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/Actor.hh>
#include <gz/sim/components/Actor.hh>
#include <gz/sim/components/Name.hh>

using namespace custom_plugins;

void ActorPosePublisher::Configure(
    const gz::sim::Entity & /*_entity*/,
    const std::shared_ptr<const sdf::Element> &_sdf,
    gz::sim::EntityComponentManager & /*_ecm*/,
    gz::sim::EventManager & /*_eventMgr*/) {
    std::string topicName = "/actors/poses";

    if (_sdf->HasElement("topic")) {
        topicName = _sdf->Get<std::string>("topic");
    }

    if (_sdf->HasElement("frame_id")) {
        this->frameId = _sdf->Get<std::string>("frame_id");
    }

    this->posesPub = this->node.Advertise<gz::msgs::Pose_V>(topicName);
}

void ActorPosePublisher::PostUpdate(
    const gz::sim::UpdateInfo &_info,
    const gz::sim::EntityComponentManager &_ecm) {
    if (_info.paused)
        return;

    gz::msgs::Pose_V posesMsg;

    auto *header = posesMsg.mutable_header();
    auto *stamp = header->mutable_stamp();
    stamp->set_sec(
        std::chrono::duration_cast<std::chrono::seconds>(_info.simTime)
            .count());
    stamp->set_nsec((_info.simTime % std::chrono::seconds(1)).count());

    auto *data = header->add_data();
    data->set_key("frame_id");
    data->add_value(this->frameId);

    _ecm.Each<gz::sim::components::Actor, gz::sim::components::Name>(
        [&](const gz::sim::Entity &_entity,
            const gz::sim::components::Actor * /*_actorComp*/,
            const gz::sim::components::Name *_nameComp) -> bool {
            gz::sim::Actor actor(_entity);

            gz::math::Pose3d currentWorldPose;

            auto trajPose = actor.TrajectoryPose(_ecm);
            auto originPose = actor.Pose(_ecm);

            if (trajPose.has_value() && originPose.has_value()) {
                currentWorldPose = originPose.value() * trajPose.value();
            } else if (actor.WorldPose(_ecm).has_value()) {
                currentWorldPose = actor.WorldPose(_ecm).value();
            } else {
                return true;
            }

            gz::msgs::Pose *pose = posesMsg.add_pose();
            *pose = gz::msgs::Convert(currentWorldPose);
            pose->set_name(_nameComp->Data());
            pose->set_id(_entity);

            return true;
        });

    this->posesPub.Publish(posesMsg);
}

GZ_ADD_PLUGIN(custom_plugins::ActorPosePublisher, gz::sim::System,
              custom_plugins::ActorPosePublisher::ISystemConfigure,
              custom_plugins::ActorPosePublisher::ISystemPostUpdate)
