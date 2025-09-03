#include "Task_OffensiveBFM.h"

namespace Action
{
    PortsList Task_OffensiveBFM::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus Task_OffensiveBFM::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float distance = (*BB)->Distance;
        float aspectAngle = (*BB)->MyAspectAngle_Degree;

        // 엔트리 윈도우 확인 및 진입
        if (!IsInTurnCircle(BB.value()))
        {
            // 적기의 턴 서클 바깥에 있을 때 - 엔트리 윈도우로 이동
            (*BB)->VP_Cartesian = CalculateEntryWindow(BB.value());
        }
        else
        {
            // 적기의 턴 서클 안에 있을 때
            if (distance > 914.4f) // 3000 feet = 914.4m
            {
                // 3000피트 이상에서는 래그 추적
                (*BB)->VP_Cartesian = CalculateLagPursuit(BB.value());
            }
            else
            {
                // 3000피트 이내에서는 리드 추적
                (*BB)->VP_Cartesian = CalculateLeadPursuit(BB.value());
            }
        }

        // 스로틀 조절 (코너 속도 유지)
        if ((*BB)->MySpeed_MS < 130.0f) // 450 KCAS ? 230 m/s, 코너속도보다 낮으면
        {
            (*BB)->Throttle = 1.0f;
        }
        else if ((*BB)->MySpeed_MS > 140.0f) // 코너속도보다 높으면
        {
            (*BB)->Throttle = 0.7f;
        }
        else
        {
            (*BB)->Throttle = 0.85f;
        }

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_OffensiveBFM::CalculateEntryWindow(CPPBlackBoard* BB)
    {
        // 적기가 선회를 시작한 지점(엔트리 윈도우)으로 이동
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;

        // 적기 후방 30도 지점으로 이동
        Vector3 entryPoint = targetLocation - targetForward * 1852.0f; // 1nm 뒤

        return entryPoint;
    }

    Vector3 Task_OffensiveBFM::CalculateLagPursuit(CPPBlackBoard* BB)
    {
        // 래그 추적 - 적기의 뒤쪽을 향함
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;

        // 적기 진행 방향 뒤쪽을 추적
        Vector3 lagPoint = targetLocation - targetForward * 500.0f;

        return lagPoint;
    }

    Vector3 Task_OffensiveBFM::CalculateLeadPursuit(CPPBlackBoard* BB)
    {
        // 리드 추적 - 적기의 앞쪽을 향함 (기총 사격용)
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float targetSpeed = BB->TargetSpeed_MS;

        // 시간 예측 (Time of Flight)
        float tof = BB->Distance / 1000.0f; // 간단한 TOF 계산

        // 리드 포인트 계산
        Vector3 leadPoint = targetLocation + targetForward * targetSpeed * tof;

        return leadPoint;
    }

    bool Task_OffensiveBFM::IsInTurnCircle(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float targetSpeed = BB->TargetSpeed_MS;

        // 적기의 턴 서클 반경 추정 (간단한 계산)
        float estimatedTurnRadius = (targetSpeed * targetSpeed) / (9.81f * 7.0f); // 7G 가정

        // 거리가 턴 서클 반경의 2배보다 작으면 턴 서클 안에 있다고 판단
        return distance < (estimatedTurnRadius * 2.0f);
    }
}