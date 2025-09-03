#pragma once
#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class AltitudeSafetyCheck : public SyncActionNode
    {
    private:
        static constexpr double MIN_SAFE_ALTITUDE = 2000.0;  // 안전 마진
        static constexpr double CRITICAL_ALTITUDE = 1500.0;  // 긴급 상승 임계점
        static constexpr double EMERGENCY_ALTITUDE = 1000.0; // 최대 긴급 상승

    public:
        AltitudeSafetyCheck(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~AltitudeSafetyCheck()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;

    private:
        Vector3 CalculateEmergencyClimb(CPPBlackBoard* BB);
        bool IsAltitudeCritical(float currentAltitude);
        bool IsAltitudeEmergency(float currentAltitude);
    };
}