#pragma once

#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class Task_DefensiveBFM : public SyncActionNode
    {
    private:
        Vector3 CalculateDefensiveTurn(CPPBlackBoard* BB);
        Vector3 CalculateBeamManeuver(CPPBlackBoard* BB);
        Vector3 CalculateJinkManeuver(CPPBlackBoard* BB);
        bool IsUnderAttack(CPPBlackBoard* BB);

    public:
        Task_DefensiveBFM(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~Task_DefensiveBFM()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;
    };
}