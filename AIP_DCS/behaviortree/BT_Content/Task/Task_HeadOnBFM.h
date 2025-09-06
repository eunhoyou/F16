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
    class Task_HeadOnBFM : public SyncActionNode
    {
    private:
        // 기존 함수들
        Vector3 CalculateSliceTurn(CPPBlackBoard* BB);
        Vector3 CalculateLevelTurn(CPPBlackBoard* BB);
        Vector3 CalculateVerticalManeuver(CPPBlackBoard* BB);
        Vector3 CalculateLeadTurn(CPPBlackBoard* BB);
        bool ShouldInitiateLeadTurn(CPPBlackBoard* BB);
        bool IsEscapeWindowOpen(CPPBlackBoard* BB);

        // 새로 추가된 동적 계산 함수들
        float CalculateClosingRate(CPPBlackBoard* BB);
        float CalculateEscapeDistance(float mySpeed, float distance);
        float CalculateSliceDistance(float mySpeed, float distance);
        float CalculateLevelTurnDistance(float mySpeed, float distance);
        float CalculateClimbDistance(float mySpeed, float altitude);
        float CalculateLeadDistance(float distance, float timeToMerge);
        float CalculateHeadOnThrottle(float mySpeed, float altitude, float distance);

    public:
        Task_HeadOnBFM(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~Task_HeadOnBFM()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;
    };
}