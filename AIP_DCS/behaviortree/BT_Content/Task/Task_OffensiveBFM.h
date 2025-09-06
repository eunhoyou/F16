#pragma once

#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"
#include <algorithm>
#include <cmath>

using namespace BT;

namespace Action
{
    class Task_OffensiveBFM : public SyncActionNode
    {
    private:
        // 기존 함수들
        Vector3 CalculateEntryWindow(CPPBlackBoard* BB);
        Vector3 CalculateLagPursuit(CPPBlackBoard* BB);
        Vector3 CalculateLeadPursuit(CPPBlackBoard* BB);
        bool IsInTurnCircle(CPPBlackBoard* BB);

        // 새로 추가된 동적 계산 함수들
        float CalculateEntryDistance(float currentDistance, float targetSpeed);
        float CalculateLagDistance(float distance, float mySpeed, float targetSpeed);
        float CalculateTimeOfFlight(float distance, float mySpeed, float targetSpeed);
        float CalculateEngagementRange(float mySpeed, float altitude);
        float CalculateOptimalThrottle(float mySpeed, float altitude, float distance);

    public:
        Task_OffensiveBFM(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~Task_OffensiveBFM()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;
    };
}