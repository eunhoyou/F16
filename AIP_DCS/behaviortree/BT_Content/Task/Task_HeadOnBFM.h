#pragma once
#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class Task_HeadOnBFM : public SyncActionNode
    {
    private:
        Vector3 CalculateSliceTurn(CPPBlackBoard* BB);
        Vector3 CalculateLevelTurn(CPPBlackBoard* BB);
        Vector3 CalculateVerticalManeuver(CPPBlackBoard* BB);
        Vector3 CalculateLeadTurn(CPPBlackBoard* BB);
        bool ShouldInitiateLeadTurn(CPPBlackBoard* BB);
        bool IsEscapeWindowOpen(CPPBlackBoard* BB);

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