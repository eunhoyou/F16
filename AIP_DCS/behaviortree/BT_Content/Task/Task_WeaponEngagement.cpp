#include "Task_WeaponEngagement.h"

namespace Action
{
    PortsList Task_WeaponEngagement::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus Task_WeaponEngagement::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float distance = (*BB)->Distance;
        float los = (*BB)->Los_Degree;

        // WEZ 조건 확인
        if (IsInWEZ(BB.value()))
        {
            // WEZ 내에서 무기 교전 - 정밀 조준점 계산
            (*BB)->VP_Cartesian = CalculateWeaponAimPoint(BB.value());
        }
        else
        {
            // WEZ 밖에서는 WEZ 진입을 위한 위치 이동
            (*BB)->VP_Cartesian = CalculateWEZEntryPoint(BB.value());
        }

        // 교전 시 최대 스로틀
        (*BB)->Throttle = 1.0f;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_WeaponEngagement::CalculateWeaponAimPoint(CPPBlackBoard* BB)
    {
        // WEZ 내에서 무기 사격을 위한 정밀 조준점 계산
        Vector3 aimPoint = BB->TargetLocaion_Cartesian;

        return aimPoint;
    }

    Vector3 Task_WeaponEngagement::CalculateWEZEntryPoint(CPPBlackBoard* BB)
    {
        // WEZ 진입을 위한 위치 계산
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float distance = BB->Distance;

        // WEZ 진입을 위한 최적 거리 (WEZ 중간 지점)
        float optimalRange = (WEZ_MIN_RANGE + WEZ_MAX_RANGE) * 0.5f; // 약 533m

        // 적기를 향한 방향 벡터
        Vector3 toTarget = targetLocation - myLocation;
        toTarget = toTarget / distance; // 정규화

        Vector3 entryPoint;

        if (distance > WEZ_MAX_RANGE)
        {
            // 너무 멀리 있을 때 - 적기 쪽으로 접근
            entryPoint = targetLocation - toTarget * optimalRange;
        }
        else if (distance < WEZ_MIN_RANGE)
        {
            // 너무 가까이 있을 때 - 적기에서 멀어지면서 WEZ 각도 확보
            entryPoint = myLocation - toTarget * (WEZ_MIN_RANGE - distance + 100.0f);
        }
        else
        {
            // 거리는 적절하지만 각도가 맞지 않을 때 - 각도 조정을 위한 기동
            Vector3 myForward = BB->MyForwardVector;
            Vector3 myRight = BB->MyRightVector;
            
            // LOS가 2도 이내로 들어오도록 조정
            float adjustmentDistance = 200.0f;
            
            if (BB->Los_Degree > 2.0f)
            {
                // LOS 각도가 2도를 초과할 때 각도 조정 기동
                Vector3 adjustDirection = myRight;
                
                // LOS 각도에 따라 조정 방향과 강도 결정
                if (BB->Los_Degree > 0)
                {
                    adjustDirection = myRight * -1.0f; // 왼쪽으로 조정
                }
                
                float adjustmentDistance = BB->Los_Degree * 50.0f; // 각도에 비례하여 조정
                entryPoint = myLocation + adjustDirection * adjustmentDistance + myForward * 200.0f;
            }
            else
            {
                // 각도는 좋으므로 현재 방향으로 전진하여 WEZ 유지
                entryPoint = myLocation + myForward * 100.0f;
            }
        }

        return entryPoint;
    }

    bool Task_WeaponEngagement::IsInWEZ(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float los = BB->Los_Degree;

        // WEZ 거리 조건 확인
        bool rangeValid = (distance >= WEZ_MIN_RANGE && distance <= WEZ_MAX_RANGE);
        
        // WEZ 각도 조건 확인 (±2도)
        bool angleValid = (fabs(los) <= WEZ_MAX_ANGLE);

        // 추가 조건: 적기가 시야에 있어야 함
        bool enemyInSight = BB->EnemyInSight;

        return rangeValid && angleValid && enemyInSight;
    }

    bool Task_WeaponEngagement::HasValidWeaponSolution(CPPBlackBoard* BB)
    {
        // WEZ 조건과 동일
        return IsInWEZ(BB);
    }
}