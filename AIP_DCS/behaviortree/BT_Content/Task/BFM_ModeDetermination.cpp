#include "BFM_ModeDetermination.h"

namespace Action
{
    PortsList BFM_ModeDetermination::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus BFM_ModeDetermination::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float distance = (*BB)->Distance;
        float aspectAngle = (*BB)->MyAspectAngle_Degree;
        float angleOff = (*BB)->MyAngleOff_Degree;
        
        // 디버깅 로그
        std::cout << "[BFM_DEBUG] Distance: " << distance 
                << ", AspectAngle: " << aspectAngle 
                << ", AngleOff: " << angleOff << std::endl;

        // Head-on BFM 판단 (정면 교전)
        if (aspectAngle > 120.0f && angleOff > 120.0f)
        {
            (*BB)->BFM = HABFM;
            std::cout << "[BFM_DEBUG] HABFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // Defensive BFM 판단 (방어 상황: 적이 나를 향하고 있음)
        if (aspectAngle > 120.0f && angleOff < 60.0f)
        {
            (*BB)->BFM = DBFM;
            std::cout << "[BFM_DEBUG] DBFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // Offensive BFM 판단 (공격 상황: 내가 적을 향하고 있음)
        if (aspectAngle < 60.0f)
        {
            (*BB)->BFM = OBFM;
            std::cout << "[BFM_DEBUG] OBFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // 기본값: 중간 상황은 공격 BFM으로 처리
        (*BB)->BFM = OBFM;
        std::cout << "[BFM_DEBUG] Default OBFM selected" << std::endl;
        return NodeStatus::SUCCESS;
    }
}