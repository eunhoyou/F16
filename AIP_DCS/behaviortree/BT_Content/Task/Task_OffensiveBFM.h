#pragma once

#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class Task_OffensiveBFM : public SyncActionNode
    {
    private:
        Vector3 CalculateEntryWindow(CPPBlackBoard* BB);
        Vector3 CalculateLagPursuit(CPPBlackBoard* BB);
        Vector3 CalculateLeadPursuit(CPPBlackBoard* BB);
        bool IsInTurnCircle(CPPBlackBoard* BB);

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