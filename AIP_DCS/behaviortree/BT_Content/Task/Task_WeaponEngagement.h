#pragma once
#include "../../behaviortree_cpp_v3/action_node.h"
#include "../../behaviortree_cpp_v3/bt_factory.h"
#include "../../../Geometry/Vector3.h"
#include "../Functions.h"
#include "../BlackBoard/CPPBlackBoard.h"

using namespace BT;

namespace Action
{
    class Task_WeaponEngagement : public SyncActionNode
    {
    private:
        Vector3 CalculateMissileEngagement(CPPBlackBoard* BB);
        Vector3 CalculateGunEngagement(CPPBlackBoard* BB);
        bool IsInMissileRange(CPPBlackBoard* BB);
        bool IsInGunRange(CPPBlackBoard* BB);
        bool HasValidGunSolution(CPPBlackBoard* BB);

    public:
        Task_WeaponEngagement(const std::string& name, const NodeConfiguration& config) : SyncActionNode(name, config)
        {
        }

        ~Task_WeaponEngagement()
        {
        }

        static PortsList providedPorts();
        NodeStatus tick() override;
    };
}