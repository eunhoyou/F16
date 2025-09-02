#pragma once

#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class Task_Detecting : public SyncActionNode
    {
    private:
        Vector3 CalculateSearchPattern(CPPBlackBoard* BB);
        Vector3 CalculateInterceptCourse(CPPBlackBoard* BB);
        Vector3 CalculateSternConversion(CPPBlackBoard* BB);

    public:
        Task_Detecting(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~Task_Detecting()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;
    };
}