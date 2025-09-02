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
        float aspectAngle = (*BB)->MyAspectAngle_Degree;

        // 무기 사용 우선순위: 미사일 -> 기총
        if (IsInMissileRange(BB.value()))
        {
            (*BB)->VP_Cartesian = CalculateMissileEngagement(BB.value());
        }
        else if (IsInGunRange(BB.value()) && HasValidGunSolution(BB.value()))
        {
            (*BB)->VP_Cartesian = CalculateGunEngagement(BB.value());
        }
        else
        {
            // 무기 사용 불가 - 위치 개선
            Vector3 targetLocation = (*BB)->TargetLocaion_Cartesian;
            (*BB)->VP_Cartesian = targetLocation;
        }

        // 교전 시 최대 스로틀
        (*BB)->Throttle = 1.0f;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_WeaponEngagement::CalculateMissileEngagement(CPPBlackBoard* BB)
    {
        // 미사일 사격을 위한 위치 계산
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float targetSpeed = BB->TargetSpeed_MS;
        float distance = BB->Distance;

        // Pure Pursuit for missile engagement
        // 미사일 비행 시간 예측
        float missileSpeed = 800.0f; // 대략적인 미사일 속도 m/s
        float missileFlightTime = distance / missileSpeed;

        // 표적의 예상 위치
        Vector3 leadPoint = targetLocation + targetForward * targetSpeed * missileFlightTime;

        return leadPoint;
    }

    Vector3 Task_WeaponEngagement::CalculateGunEngagement(CPPBlackBoard* BB)
    {
        // 기총 사격을 위한 Lead Pursuit 계산
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        Vector3 targetRight = BB->TargetRightVector;
        float targetSpeed = BB->TargetSpeed_MS;
        float distance = BB->Distance;

        // 기총 탄환 속도 (대략 1000 m/s)
        float bulletSpeed = 1000.0f;
        float timeOfFlight = distance / bulletSpeed;

        // Lead 계산
        Vector3 leadVector = targetForward * targetSpeed * timeOfFlight;

        // 적기가 선회 중이라면 추가 lead 필요
        Vector3 turnLead = targetRight * targetSpeed * 0.1f; // 간단한 선회 보정

        Vector3 gunAimPoint = targetLocation + leadVector + turnLead;

        return gunAimPoint;
    }

    bool Task_WeaponEngagement::IsInMissileRange(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float aspectAngle = BB->MyAspectAngle_Degree;

        // AIM-9M 사이드와인더 기본 범위
        float rMin = 926.0f;   // 0.5nm
        float rMax = 18520.0f; // 10nm

        // 에스펙트에 따른 사거리 조정
        if (aspectAngle < 60.0f) // 후방 공격
        {
            rMax *= 1.2f; // 후방에서 더 멀리 사격 가능
        }
        else if (aspectAngle > 120.0f) // 정면 공격
        {
            rMax *= 0.8f; // 정면에서는 사거리 감소
        }

        return (distance >= rMin && distance <= rMax);
    }

    bool Task_WeaponEngagement::IsInGunRange(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float aspectAngle = BB->MyAspectAngle_Degree;

        // M61 기관포 유효 사거리
        float gunRange = 762.0f; // 2500 feet

        if (aspectAngle > 120.0f) // 높은 에스펙트
        {
            gunRange *= 1.6f; // 4000 feet까지 확장
        }

        return (distance <= gunRange);
    }

    bool Task_WeaponEngagement::HasValidGunSolution(CPPBlackBoard* BB)
    {
        float angleOff = BB->MyAngleOff_Degree;
        float los = BB->Los_Degree;

        // 기총 사격 조건
        // 1. 앵글 오프가 45도 이내
        // 2. LOS가 적절한 범위 내
        // 3. Lead Pursuit 상태

        return (angleOff <= 45.0f && los <= 30.0f);
    }
}