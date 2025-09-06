#include "Task_DefensiveBFM.h"

namespace Action
{
    PortsList Task_DefensiveBFM::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus Task_DefensiveBFM::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float distance = (*BB)->Distance;
        float los = (*BB)->Los_Degree;
        float mySpeed = (*BB)->MySpeed_MS;
        float targetSpeed = (*BB)->TargetSpeed_MS;
        float myAltitude = static_cast<float>(std::abs((*BB)->MyLocation_Cartesian.Z));
        Vector3 calculated_vp;

        std::cout << "[Task_DefensiveBFM] Distance: " << distance 
                  << "m, LOS: " << los << "°, Speed: " << mySpeed 
                  << "m/s, Altitude: " << myAltitude << "m" << std::endl;

        // WEZ 위험도 평가
        float wezThreat = CalculateWEZThreat(BB.value());
        float generalThreat = CalculateGeneralThreat(BB.value());
        
        std::cout << "[Task_DefensiveBFM] WEZ threat: " << wezThreat 
                  << ", General threat: " << generalThreat << std::endl;

        // WEZ 위험도에 따른 방어 기동 선택
        if (wezThreat > 0.9f)  // WEZ 내 극도로 위험
        {
            std::cout << "[Task_DefensiveBFM] CRITICAL: In enemy WEZ - Emergency notch!" << std::endl;
            calculated_vp = CalculateEmergencyNotch(BB.value());
        }
        else if (wezThreat > 0.5f)  // WEZ 접근 위험
        {
            std::cout << "[Task_DefensiveBFM] WARNING: Approaching WEZ - Defensive beam!" << std::endl;
            calculated_vp = CalculateDefensiveBeam(BB.value());
        }
        else if (generalThreat > 0.7f)  // 일반적인 높은 위협
        {
            std::cout << "[Task_DefensiveBFM] High general threat - Aggressive defensive turn!" << std::endl;
            calculated_vp = CalculateAggressiveDefensiveTurn(BB.value());
        }
        else if (IsInMissileEnvelope(BB.value()))  // 미사일 위협권
        {
            std::cout << "[Task_DefensiveBFM] In missile envelope - Beam maneuver!" << std::endl;
            calculated_vp = CalculateBeamManeuver(BB.value());
        }
        else
        {
            std::cout << "[Task_DefensiveBFM] Standard defensive turn!" << std::endl;
            calculated_vp = CalculateDefensiveTurn(BB.value());
        }

        (*BB)->VP_Cartesian = calculated_vp;

        // 위험도에 따른 동적 스로틀 계산
        float optimalThrottle = CalculateDefensiveThrottle(mySpeed, myAltitude, wezThreat, generalThreat);
        (*BB)->Throttle = optimalThrottle;

        std::cout << "[Task_DefensiveBFM] Throttle set to: " << optimalThrottle << std::endl;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_DefensiveBFM::CalculateEmergencyNotch(CPPBlackBoard* BB)
    {
        // WEZ 내에서 긴급 회피: 90도 방향으로 최대 기동
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;
        Vector3 myForward = BB->MyForwardVector;
        float mySpeed = BB->MySpeed_MS;

        // 적기에 대해 정확히 90도 방향으로 기동 (Notch)
        Vector3 toTarget = targetLocation - myLocation;
        Vector3 notchDirection = myRight;
        
        // 오른쪽 또는 왼쪽 중 더 안전한 방향 선택
        if (toTarget.dot(myRight) < 0)
        {
            notchDirection = myRight * -1.0f;
        }

        // 최대 기동 거리 (속도에 비례하여 강력한 기동)
        float notchDistance = mySpeed * 6.0f;  // 6초간 최대 측방 기동
        
        Vector3 notchPoint = myLocation + notchDirection * notchDistance;
        
        // 약간의 고도 변경으로 3차원 기동
        notchPoint.Z = myLocation.Z - 200.0f;  // 200m 상승

        std::cout << "[CalculateEmergencyNotch] Notch distance: " << notchDistance << "m" << std::endl;

        return notchPoint;
    }

    Vector3 Task_DefensiveBFM::CalculateDefensiveBeam(CPPBlackBoard* BB)
    {
        // WEZ 접근 시 방어적 빔 기동
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;
        float distance = BB->Distance;
        float mySpeed = BB->MySpeed_MS;

        // WEZ 거리를 고려한 빔 거리 계산
        float safeDistance = WEZ_MAX_RANGE * 1.5f;  // WEZ 최대 거리의 1.5배
        float beamDistance = std::max(safeDistance - distance + 500.0f, mySpeed * 4.0f);

        Vector3 beamDirection = myRight;
        Vector3 beamPoint = myLocation + beamDirection * beamDistance;

        std::cout << "[CalculateDefensiveBeam] Beam distance: " << beamDistance 
                  << "m (WEZ safe distance: " << safeDistance << "m)" << std::endl;

        return beamPoint;
    }

    Vector3 Task_DefensiveBFM::CalculateAggressiveDefensiveTurn(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;
        float mySpeed = BB->MySpeed_MS;
        float distance = BB->Distance;

        // 적기 방향으로 적극적인 방어 선회 (BFM 문제 유발)
        Vector3 toTarget = targetLocation - myLocation;
        float dotRight = toTarget.dot(myRight);
        Vector3 turnDirection = (dotRight > 0) ? myRight : myRight * -1.0f;

        // 최대 선회율로 기동 (7G 선회)
        float turnRadius = CalculateTurnRadius(mySpeed, 7.0f);
        float aggressiveDistance = turnRadius * 2.5f;  // 더 공격적인 거리

        Vector3 aggressivePoint = myLocation + turnDirection * aggressiveDistance;
        aggressivePoint.Z = myLocation.Z - 150.0f;  // 150m 상승으로 에너지 보존

        std::cout << "[CalculateAggressiveDefensiveTurn] Aggressive turn distance: " 
                  << aggressiveDistance << "m, Turn radius: " << turnRadius << "m" << std::endl;

        return aggressivePoint;
    }

    Vector3 Task_DefensiveBFM::CalculateDefensiveTurn(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myRight = BB->MyRightVector;
        float mySpeed = BB->MySpeed_MS;
        float distance = BB->Distance;

        // 적기 방향 벡터
        Vector3 toTarget = targetLocation - myLocation;

        // 선회 방향 결정 (적기 쪽으로)
        float dotRight = toTarget.dot(myRight);
        Vector3 turnDirection = (dotRight > 0) ? myRight : myRight * -1.0f;

        // 동적 선회 거리 계산 (속도와 거리에 따라)
        float turnRadius = CalculateTurnRadius(mySpeed, 6.0f);  // 6G 선회
        float defensiveDistance = CalculateDefensiveDistance(distance, mySpeed);

        Vector3 defensivePoint = myLocation + turnDirection * defensiveDistance;
        
        // 고도 유지 (약간의 상승으로 에너지 보존)
        defensivePoint.Z = myLocation.Z - 100.0f;  // 100m 상승

        std::cout << "[CalculateDefensiveTurn] Turn radius: " << turnRadius 
                  << "m, Defensive distance: " << defensiveDistance << "m" << std::endl;

        return defensivePoint;
    }

    Vector3 Task_DefensiveBFM::CalculateBeamManeuver(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;
        float distance = BB->Distance;
        float mySpeed = BB->MySpeed_MS;

        // 미사일을 3/9 라인(빔) 위치에 놓는 기동
        // 거리와 속도에 따른 동적 빔 거리 계산
        float beamDistance = CalculateBeamDistance(distance, mySpeed);
        
        Vector3 beamDirection = myRight;
        Vector3 beamPoint = myLocation + beamDirection * beamDistance;

        std::cout << "[CalculateBeamManeuver] Beam distance: " << beamDistance << "m" << std::endl;

        return beamPoint;
    }

    float Task_DefensiveBFM::CalculateWEZThreat(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float los = BB->Los_Degree;
        
        float wezThreat = 0.0f;

        // WEZ 거리 위험도 (가까울수록 위험)
        if (distance <= WEZ_MAX_RANGE)
        {
            if (distance >= WEZ_MIN_RANGE)
            {
                // WEZ 내부: 거리에 따른 위험도
                float distanceRatio = (WEZ_MAX_RANGE - distance) / (WEZ_MAX_RANGE - WEZ_MIN_RANGE);
                wezThreat += distanceRatio * 0.6f;  // 최대 60% 위험도
            }
            else
            {
                // WEZ 최소 거리보다 가까움: 최대 위험
                wezThreat += 0.8f;
            }
        }
        else
        {
            // WEZ 접근 위험도 (1.5배 거리까지 고려)
            float approachDistance = WEZ_MAX_RANGE * 1.5f;
            if (distance <= approachDistance)
            {
                float approachRatio = (approachDistance - distance) / (approachDistance - WEZ_MAX_RANGE);
                wezThreat += approachRatio * 0.3f;  // 최대 30% 위험도
            }
        }

        // WEZ 각도 위험도 (적이 나를 정확히 조준할수록 위험)
        float losRatio = std::max(0.0f, (WEZ_MAX_ANGLE * 5.0f - std::abs(los)) / (WEZ_MAX_ANGLE * 5.0f));
        wezThreat += losRatio * 0.4f;  // 최대 40% 위험도

        return std::clamp(wezThreat, 0.0f, 1.0f);
    }

    float Task_DefensiveBFM::CalculateGeneralThreat(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float los = BB->Los_Degree;
        float mySpeed = BB->MySpeed_MS;
        float targetSpeed = BB->TargetSpeed_MS;
        float aspectAngle = BB->MyAspectAngle_Degree;

        float threatLevel = 0.0f;

        // 일반적인 거리 위협 (2km 이내)
        float distanceThreat = std::max(0.0f, (2000.0f - distance) / 2000.0f);
        
        // LOS 위협 (30도 이내에서 위험)
        float losThreat = std::max(0.0f, (30.0f - std::abs(los)) / 30.0f);
        
        // 속도 열세 위협
        float speedThreat = std::max(0.0f, (targetSpeed - mySpeed) / 100.0f);
        
        // 에스펙트 앵글 위협 (적이 내 후방에 있을수록 위험)
        float aspectThreat = std::max(0.0f, (aspectAngle - 120.0f) / 60.0f);

        threatLevel = (distanceThreat * 0.3f + losThreat * 0.4f + 
                      speedThreat * 0.2f + aspectThreat * 0.1f);

        return std::clamp(threatLevel, 0.0f, 1.0f);
    }

    bool Task_DefensiveBFM::IsInMissileEnvelope(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float los = BB->Los_Degree;
        float targetSpeed = BB->TargetSpeed_MS;

        // 미사일 위협권: WEZ보다 넓은 범위
        float missileMinRange = 500.0f;  // 500m
        float missileMaxRange = 2000.0f + (targetSpeed / 100.0f) * 500.0f;  // 속도에 따른 동적 거리
        float missileMaxAngle = 45.0f;  // ±45도

        bool inMissileRange = (distance >= missileMinRange && distance <= missileMaxRange);
        bool inMissileAngle = (std::abs(los) <= missileMaxAngle);

        std::cout << "[IsInMissileEnvelope] Missile range: " << inMissileRange 
                  << " (" << distance << "m), Missile angle: " << inMissileAngle 
                  << " (" << los << "°)" << std::endl;

        return inMissileRange && inMissileAngle;
    }

    float Task_DefensiveBFM::CalculateDefensiveDistance(float distance, float mySpeed)
    {
        // WEZ를 고려한 방어 거리 계산
        float baseDistance = std::max(distance * 0.5f, 800.0f);  // 최소 800m
        float speedFactor = mySpeed / 200.0f;  // 200m/s 기준
        
        // WEZ 거리보다 멀리 유지하려고 시도
        if (distance < WEZ_MAX_RANGE * 1.2f)
        {
            baseDistance = std::max(baseDistance, WEZ_MAX_RANGE * 0.8f);
        }
        
        return baseDistance * speedFactor;
    }

    float Task_DefensiveBFM::CalculateBeamDistance(float distance, float mySpeed)
    {
        // 거리의 70-90% + 속도 보정
        float baseRatio = 0.8f;
        float speedFactor = mySpeed / 250.0f;
        
        return std::clamp(distance * baseRatio * speedFactor, 1000.0f, 3000.0f);
    }

    float Task_DefensiveBFM::CalculateTurnRadius(float speed, float gLoad)
    {
        return (speed * speed) / (9.81f * gLoad);
    }

    float Task_DefensiveBFM::CalculateDefensiveThrottle(float mySpeed, float altitude, float wezThreat, float generalThreat)
    {
        // WEZ 위험도가 높을수록 최대 추력
        if (wezThreat > 0.7f)
        {
            return 1.0f;  // 최대 추력
        }
        
        if (generalThreat > 0.8f)
        {
            return 1.0f;  // 최대 추력
        }
        
        // 일반적인 방어 상황
        float baseThrottle = 0.9f;
        
        // 속도가 너무 높으면 추력 조절
        float maxDefensiveSpeed = 250.0f + (altitude / 10000.0f) * 50.0f;
        if (mySpeed > maxDefensiveSpeed)
        {
            return 0.7f;
        }
        
        return baseThrottle;
    }
}