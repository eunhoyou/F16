#include "Task_OffensiveBFM.h"
#include <algorithm>
#include <cmath>

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
        float mySpeed = (*BB)->MySpeed_MS;
        float targetSpeed = (*BB)->TargetSpeed_MS;
        float myAltitude = static_cast<float>(std::abs((*BB)->MyLocation_Cartesian.Z));

        std::cout << "[Task_OffensiveBFM] Distance: " << distance 
                  << "m, Speed: " << mySpeed << "m/s, Altitude: " << myAltitude << "m" << std::endl;

        // 엔트리 윈도우 확인 및 진입
        if (!IsInTurnCircle(BB.value()))
        {
            // 적기의 턴 서클 바깥에 있을 때 - 엔트리 윈도우로 이동
            (*BB)->VP_Cartesian = CalculateEntryWindow(BB.value());
        }
        else
        {
            // 적기의 턴 서클 안에 있을 때
            float engagementRange = CalculateEngagementRange(mySpeed, myAltitude);
            
            if (distance > engagementRange)
            {
                // 교전 거리 밖에서는 래그 추적
                (*BB)->VP_Cartesian = CalculateLagPursuit(BB.value());
            }
            else
            {
                // 교전 거리 내에서는 리드 추적 (기총 사격)
                (*BB)->VP_Cartesian = CalculateLeadPursuit(BB.value());
            }
        }

        // 동적 스로틀 조절 (고도와 속도 고려)
        float optimalThrottle = CalculateOptimalThrottle(mySpeed, myAltitude, distance);
        (*BB)->Throttle = optimalThrottle;

        std::cout << "[Task_OffensiveBFM] Throttle set to: " << optimalThrottle << std::endl;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_OffensiveBFM::CalculateEntryWindow(CPPBlackBoard* BB)
    {
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float distance = BB->Distance;
        float targetSpeed = BB->TargetSpeed_MS;

        // 거리와 속도에 따른 동적 엔트리 거리 계산
        float entryDistance = CalculateEntryDistance(distance, targetSpeed);
        
        // 적기 후방으로 이동 (동적 거리 적용)
        Vector3 entryPoint = targetLocation - targetForward * entryDistance;

        std::cout << "[CalculateEntryWindow] Entry distance: " << entryDistance << "m" << std::endl;
        return entryPoint;
    }

    Vector3 Task_OffensiveBFM::CalculateLagPursuit(CPPBlackBoard* BB)
    {
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float distance = BB->Distance;
        float targetSpeed = BB->TargetSpeed_MS;
        float mySpeed = BB->MySpeed_MS;

        // 속도 차이에 따른 동적 래그 거리 계산
        float lagDistance = CalculateLagDistance(distance, mySpeed, targetSpeed);
        
        // 적기 진행 방향 뒤쪽을 추적 (동적 거리)
        Vector3 lagPoint = targetLocation - targetForward * lagDistance;

        std::cout << "[CalculateLagPursuit] Lag distance: " << lagDistance << "m" << std::endl;
        return lagPoint;
    }

    Vector3 Task_OffensiveBFM::CalculateLeadPursuit(CPPBlackBoard* BB)
    {
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        Vector3 myLocation = BB->MyLocation_Cartesian;
        float targetSpeed = BB->TargetSpeed_MS;
        float distance = BB->Distance;
        float mySpeed = BB->MySpeed_MS;

        // 동적 TOF(Time of Flight) 계산
        float tof = CalculateTimeOfFlight(distance, mySpeed, targetSpeed);

        // 리드 포인트 계산 (예측 사격)
        Vector3 leadPoint = targetLocation + targetForward * targetSpeed * tof;

        std::cout << "[CalculateLeadPursuit] TOF: " << tof << "s, Lead distance: " 
                  << (targetSpeed * tof) << "m" << std::endl;
        return leadPoint;
    }

    float Task_OffensiveBFM::CalculateEntryDistance(float currentDistance, float targetSpeed)
    {
        // 기본 거리 + 속도에 비례한 추가 거리
        float baseDistance = 1000.0f;  // 기본 1km
        float speedFactor = targetSpeed / 100.0f;  // 속도 계수 (100m/s 기준)
        
        return std::max(baseDistance * speedFactor, 500.0f);  // 최소 500m
    }

    float Task_OffensiveBFM::CalculateLagDistance(float distance, float mySpeed, float targetSpeed)
    {
        // 거리의 10-20% + 속도 차이 고려
        float baseRatio = 0.15f;  // 기본 15%
        float speedRatio = (mySpeed > targetSpeed) ? 0.1f : 0.2f;  // 속도 우위시 가까이
        
        float lagDistance = distance * (baseRatio + speedRatio);
        return std::max(200.0f, std::min(lagDistance, 800.0f)); // 200m-800m 제한
    }

    float Task_OffensiveBFM::CalculateTimeOfFlight(float distance, float mySpeed, float targetSpeed)
    {
        // 간단한 TOF 계산 (거리 / 접근 속도)
        float closingSpeed = std::max(mySpeed - targetSpeed * 0.5f, mySpeed * 0.5f);
        return distance / closingSpeed;
    }

    float Task_OffensiveBFM::CalculateEngagementRange(float mySpeed, float altitude)
    {
        // 기본 교전 거리 914.4m에서 속도와 고도에 따라 조정
        float baseRange = 914.4f;
        float speedFactor = mySpeed / 250.0f;  // 250m/s 기준
        float altitudeFactor = std::max(0.8f, std::min(altitude / 5000.0f, 1.2f)); // 고도 보정
        
        return baseRange * speedFactor * altitudeFactor;
    }

    float Task_OffensiveBFM::CalculateOptimalThrottle(float mySpeed, float altitude, float distance)
    {
        // 코너 속도 계산 (고도에 따라 조정)
        float cornerSpeed = 130.0f + (altitude / 10000.0f) * 20.0f;  // 고고도에서 더 높은 코너속도
        
        if (mySpeed < cornerSpeed - 10.0f)
        {
            return 1.0f;  // 최대 추력
        }
        else if (mySpeed > cornerSpeed + 10.0f)
        {
            return 0.6f;  // 추력 감소
        }
        else
        {
            // 거리에 따른 미세 조정
            float baseThrottle = 0.85f;
            if (distance < 500.0f)
            {
                baseThrottle = 0.9f;  // 근거리에서 약간 더 높은 추력
            }
            return baseThrottle;
        }
    }

    bool Task_OffensiveBFM::IsInTurnCircle(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float targetSpeed = BB->TargetSpeed_MS;
        float myAltitude = std::abs(BB->MyLocation_Cartesian.Z);

        // 동적 턴 서클 반경 계산 (고도와 속도 고려)
        float gLoad = 7.0f + (myAltitude / 10000.0f) * 2.0f;  // 고고도에서 더 높은 G
        float estimatedTurnRadius = (targetSpeed * targetSpeed) / (9.81f * gLoad);

        // 턴 서클 판단 기준을 동적으로 조정
        float turnCircleThreshold = estimatedTurnRadius * 2.5f;
        
        std::cout << "[IsInTurnCircle] Turn radius: " << estimatedTurnRadius 
                  << "m, Threshold: " << turnCircleThreshold << "m, Distance: " << distance << "m" << std::endl;

        return distance < turnCircleThreshold;
    }
}