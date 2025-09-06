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
    class Task_DefensiveBFM : public SyncActionNode
    {
    private:
        // WEZ 상수들 (적기의 공격 사정거리 인식용)
        static constexpr float WEZ_MIN_RANGE = 152.4f;  // 500 피트 = 152.4 미터
        static constexpr float WEZ_MAX_RANGE = 914.4f;  // 3000 피트 = 914.4 미터
        static constexpr float WEZ_MAX_ANGLE = 2.0f;    // ±2도

        // 기존 함수들
        Vector3 CalculateDefensiveTurn(CPPBlackBoard* BB);
        Vector3 CalculateBeamManeuver(CPPBlackBoard* BB);

        // WEZ 인식 기반 방어 함수들
        Vector3 CalculateEmergencyNotch(CPPBlackBoard* BB);
        Vector3 CalculateDefensiveBeam(CPPBlackBoard* BB);
        Vector3 CalculateAggressiveDefensiveTurn(CPPBlackBoard* BB);
        
        // 위협 평가 함수들
        float CalculateWEZThreat(CPPBlackBoard* BB);
        float CalculateGeneralThreat(CPPBlackBoard* BB);
        bool IsInMissileEnvelope(CPPBlackBoard* BB);
        
        // 동적 계산 함수들
        float CalculateDefensiveDistance(float distance, float mySpeed);
        float CalculateBeamDistance(float distance, float mySpeed);
        float CalculateTurnRadius(float speed, float gLoad);
        float CalculateDefensiveThrottle(float mySpeed, float altitude, float wezThreat, float generalThreat);

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