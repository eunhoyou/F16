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
        bool enemyInSight = (*BB)->EnemyInSight;

        // 시야에 없는 경우 탐지 모드
        if (!enemyInSight)
        {
            (*BB)->BFM = DETECTING;
            return NodeStatus::SUCCESS;
        }

        // Head-on BFM 판단
        if (aspectAngle > 120.0f && angleOff > 120.0f)
        {
            (*BB)->BFM = HABFM;
            return NodeStatus::SUCCESS;
        }

        // Offensive BFM 판단
        if (aspectAngle < 60.0f)
        {
            (*BB)->BFM = OBFM;
            return NodeStatus::SUCCESS;
        }

        // Defensive BFM 판단
        if (aspectAngle > 120.0f && angleOff < 60.0f)
        {
            (*BB)->BFM = DBFM;
            return NodeStatus::SUCCESS;
        }

        // 기본값은 NONE
        (*BB)->BFM = NONE;
        return NodeStatus::SUCCESS;
    }
}