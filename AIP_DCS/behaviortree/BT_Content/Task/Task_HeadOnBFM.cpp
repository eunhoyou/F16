#include "Task_HeadOnBFM.h"

namespace Action
{
    PortsList Task_HeadOnBFM::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus Task_HeadOnBFM::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");
        
        std::cout << "[Task_HeadOnBFM] Starting Head-on BFM..." << std::endl;
            
        float distance = (*BB)->Distance;
        float mySpeed = (*BB)->MySpeed_MS;
        float targetSpeed = (*BB)->TargetSpeed_MS;
        float myAltitude = std::abs((*BB)->MyLocation_Cartesian.Z);
        float closingRate = CalculateClosingRate(BB.value());

        std::cout << "[Task_HeadOnBFM] Distance: " << distance 
                << ", MySpeed: " << mySpeed << ", TargetSpeed: " << targetSpeed 
                << ", Closing rate: " << closingRate << "m/s" << std::endl;

        // 이탈 창 확인 (동적 계산)
        if (IsEscapeWindowOpen(BB.value()))
        {
            std::cout << "[Task_HeadOnBFM] Escape window open - disengaging" << std::endl;
            Vector3 myLocation = (*BB)->MyLocation_Cartesian;
            Vector3 myForward = (*BB)->MyForwardVector;
            float escapeDistance = CalculateEscapeDistance(mySpeed, distance);
            (*BB)->VP_Cartesian = myLocation + myForward * escapeDistance;
            (*BB)->Throttle = 1.0f;
            return NodeStatus::SUCCESS;
        }

        // 리드 턴 실행 시점 판단 (동적 기준)
        if (ShouldInitiateLeadTurn(BB.value()))
        {
            std::cout << "[Task_HeadOnBFM] Initiating lead turn" << std::endl;
            (*BB)->VP_Cartesian = CalculateLeadTurn(BB.value());
        }
        else
        {
            std::cout << "[Task_HeadOnBFM] Executing crossing maneuver" << std::endl;
            
            // 속도 차이에 따른 기동 선택
            float speedAdvantage = mySpeed - targetSpeed;
            
            if (speedAdvantage > 30.0f)  // 상당한 속도 우위
            {
                std::cout << "[Task_HeadOnBFM] Speed advantage - Slice turn" << std::endl;
                (*BB)->VP_Cartesian = CalculateSliceTurn(BB.value());
            }
            else if (speedAdvantage < -30.0f)  // 상당한 속도 열세
            {
                std::cout << "[Task_HeadOnBFM] Speed disadvantage - Vertical maneuver" << std::endl;
                (*BB)->VP_Cartesian = CalculateVerticalManeuver(BB.value());
            }
            else  // 비슷한 속도
            {
                std::cout << "[Task_HeadOnBFM] Similar speeds - Level turn" << std::endl;
                (*BB)->VP_Cartesian = CalculateLevelTurn(BB.value());
            }
        }

        // 동적 스로틀 계산
        float optimalThrottle = CalculateHeadOnThrottle(mySpeed, myAltitude, distance);
        (*BB)->Throttle = optimalThrottle;

        std::cout << "[Task_HeadOnBFM] Throttle set to: " << optimalThrottle << std::endl;
        return NodeStatus::SUCCESS;
    }

    Vector3 Task_HeadOnBFM::CalculateSliceTurn(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myRight = BB->MyRightVector;
        float mySpeed = BB->MySpeed_MS;
        float distance = BB->Distance;

        // 적기 방향으로 선회 방향 결정
        Vector3 toTarget = (targetLocation - myLocation);
        float dotRight = toTarget.dot(myRight);
        Vector3 sliceDirection = (dotRight > 0) ? myRight : myRight * -1.0f;

        // 동적 슬라이스 거리 계산 (속도와 거리에 기반)
        float sliceDistance = CalculateSliceDistance(mySpeed, distance);
        float forwardDistance = mySpeed * 3.0f;  // 3초간 전진 거리

        Vector3 slicePoint = myLocation + sliceDirection * sliceDistance + myForward * forwardDistance;
        
        // 동적 강하각 계산
        float descentRate = std::min(mySpeed * 0.1f, 200.0f);  // 속도의 10%, 최대 200m
        slicePoint.Z = myLocation.Z + descentRate;  // NED에서 하강

        std::cout << "[CalculateSliceTurn] Slice distance: " << sliceDistance 
                  << "m, Descent: " << descentRate << "m" << std::endl;

        return slicePoint;
    }

    Vector3 Task_HeadOnBFM::CalculateLevelTurn(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;
        float mySpeed = BB->MySpeed_MS;
        float distance = BB->Distance;

        Vector3 toTarget = (targetLocation - myLocation);
        float dotRight = toTarget.dot(myRight);
        Vector3 turnDirection = (dotRight > 0) ? myRight : myRight * -1.0f;

        // 동적 선회 거리 계산
        float turnDistance = CalculateLevelTurnDistance(mySpeed, distance);

        Vector3 levelTurnPoint = myLocation + turnDirection * turnDistance;
        levelTurnPoint.Z = myLocation.Z;  // 수평 유지

        std::cout << "[CalculateLevelTurn] Turn distance: " << turnDistance << "m" << std::endl;

        return levelTurnPoint;
    }

    Vector3 Task_HeadOnBFM::CalculateVerticalManeuver(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myUp = BB->MyUpVector;
        float mySpeed = BB->MySpeed_MS;
        float myAltitude = static_cast<float>(std::abs(myLocation.Z));

        // 동적 수직 기동 가능성 판단
        float minVerticalSpeed = 120.0f + (myAltitude / 10000.0f) * 30.0f;  // 고도에 따른 최소 속도
        
        if (mySpeed > minVerticalSpeed && myAltitude > 1500.0f)
        {
            // 수직 상승
            float climbDistance = CalculateClimbDistance(mySpeed, myAltitude);
            float forwardDistance = mySpeed * 2.0f;  // 2초간 전진
            
            Vector3 verticalPoint = myLocation + myUp * climbDistance + myForward * forwardDistance;
            
            std::cout << "[CalculateVerticalManeuver] Climb distance: " << climbDistance << "m" << std::endl;
            return verticalPoint;
        }
        else
        {
            // 속도가 부족하면 수평 선회로 대체
            std::cout << "[CalculateVerticalManeuver] Insufficient speed, falling back to level turn" << std::endl;
            return CalculateLevelTurn(BB);
        }
    }

    Vector3 Task_HeadOnBFM::CalculateLeadTurn(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        Vector3 myRight = BB->MyRightVector;
        float targetSpeed = BB->TargetSpeed_MS;
        float distance = BB->Distance;
        float closingRate = CalculateClosingRate(BB);

        // 동적 합류 시간 계산
        float timeToMerge = (closingRate > 0) ? distance / closingRate : 5.0f;
        timeToMerge = std::max(1.0f, std::min(timeToMerge, 8.0f));  // 1-8초 제한

        // 적기의 예측 위치
        Vector3 predictedTargetPos = targetLocation + targetForward * targetSpeed * timeToMerge;

        // 동적 리드 거리 계산
        float leadDistance = CalculateLeadDistance(distance, timeToMerge);
        Vector3 toPredict = (predictedTargetPos - myLocation);
        Vector3 leadTurnPoint = myLocation + toPredict * leadDistance;

        std::cout << "[CalculateLeadTurn] Time to merge: " << timeToMerge 
                  << "s, Lead distance: " << leadDistance << "m" << std::endl;

        return leadTurnPoint;
    }

    float Task_HeadOnBFM::CalculateClosingRate(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 targetForward = BB->TargetForwardVector;
        float mySpeed = BB->MySpeed_MS;
        float targetSpeed = BB->TargetSpeed_MS;

        // 상대 방향 벡터
        Vector3 toTarget = (targetLocation - myLocation);
        
        // 각자의 속도가 상대방향으로 향하는 성분
        float myClosingComponent = mySpeed * myForward.dot(toTarget);
        float targetClosingComponent = targetSpeed * targetForward.dot(-toTarget);
        
        return myClosingComponent + targetClosingComponent;
    }

    float Task_HeadOnBFM::CalculateEscapeDistance(float mySpeed, float distance)
    {
        // 속도와 현재 거리에 비례한 이탈 거리
        float baseDistance = mySpeed * 10.0f;  // 10초간 이동 거리
        float distanceFactor = std::min(distance / 2000.0f, 2.0f);  // 거리 계수
        
        return baseDistance * distanceFactor;
    }

    float Task_HeadOnBFM::CalculateSliceDistance(float mySpeed, float distance)
    {
        // 속도에 비례한 슬라이스 거리
        float baseDistance = mySpeed * 4.0f;  // 4초간 측방 이동
        float distanceRatio = std::max(0.5f, std::min(distance / 3000.0f, 1.5f));
        
        return baseDistance * distanceRatio;
    }

    float Task_HeadOnBFM::CalculateLevelTurnDistance(float mySpeed, float distance)
    {
        // 선회 반경 기반 계산
        float turnRadius = (mySpeed * mySpeed) / (9.81f * 6.0f);  // 6G 선회
        float turnDistance = turnRadius * 1.5f;  // 반경의 1.5배
        
        // 거리에 따른 조정
        if (distance < 2000.0f)
        {
            turnDistance *= 0.8f;  // 근거리에서는 더 타이트한 선회
        }
        
        return turnDistance;
    }

    float Task_HeadOnBFM::CalculateClimbDistance(float mySpeed, float altitude)
    {
        // 속도 에너지를 고도로 변환
        float energyFactor = (mySpeed * mySpeed) / (2.0f * 9.81f);  // 운동에너지 -> 위치에너지
        float climbDistance = std::min(energyFactor * 0.3f, 800.0f);  // 최대 800m
        
        // 현재 고도가 높을수록 상승량 감소
        if (altitude > 8000.0f)
        {
            climbDistance *= 0.7f;
        }
        
        return climbDistance;
    }

    float Task_HeadOnBFM::CalculateLeadDistance(float distance, float timeToMerge)
    {
        // 거리와 시간에 비례한 리드 거리
        float baseDistance = distance * 0.6f;  // 거리의 60%
        float timeFactor = std::max(0.5f, std::min(timeToMerge / 5.0f, 1.5f)); // 시간 계수
        
        return baseDistance * timeFactor;
    }

    float Task_HeadOnBFM::CalculateHeadOnThrottle(float mySpeed, float altitude, float distance)
    {
        // 고도별 최적 속도 계산
        float optimalSpeed = 140.0f + (altitude / 10000.0f) * 20.0f;  // 고고도에서 더 높은 속도
        
        // 거리별 속도 조정
        if (distance < 1000.0f)  // 근거리: 기동성 우선
        {
            optimalSpeed -= 20.0f;
        }
        else if (distance > 4000.0f)  // 원거리: 속도 우선
        {
            optimalSpeed += 30.0f;
        }
        
        // 속도에 따른 스로틀 조정
        if (mySpeed < optimalSpeed - 15.0f)
        {
            return 1.0f;  // 최대 추력
        }
        else if (mySpeed > optimalSpeed + 15.0f)
        {
            return 0.5f;  // 추력 감소
        }
        else
        {
            return 0.8f;  // 중간 추력
        }
    }

    bool Task_HeadOnBFM::ShouldInitiateLeadTurn(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float los = BB->Los_Degree;
        float closingRate = CalculateClosingRate(BB);

        // 동적 리드턴 시작 기준
        float leadTurnDistance = 2000.0f + (closingRate * 10.0f);  // 접근율에 따른 조정
        leadTurnDistance = std::max(1500.0f, std::min(leadTurnDistance, 4000.0f));

        bool distanceCheck = distance < leadTurnDistance;
        bool angleCheck = std::abs(los) < 45.0f;  // LOS 45도 이내

        std::cout << "[ShouldInitiateLeadTurn] Distance check: " << distanceCheck 
                  << " (" << distance << " < " << leadTurnDistance << "), Angle check: " << angleCheck << std::endl;

        return distanceCheck && angleCheck;
    }

    bool Task_HeadOnBFM::IsEscapeWindowOpen(CPPBlackBoard* BB)
    {
        float distance = BB->Distance;
        float mySpeed = BB->MySpeed_MS;
        float targetSpeed = BB->TargetSpeed_MS;
        float angleOff = BB->MyAngleOff_Degree;
        float aspectAngle = BB->MyAspectAngle_Degree;

        // 동적 이탈 창 판단
        float minEscapeDistance = 1500.0f + (mySpeed * 5.0f);  // 속도에 따른 최소 거리
        float speedAdvantage = mySpeed - targetSpeed;
        float minSpeedAdvantage = 15.0f;  // 최소 속도 우위
        float minAngleOff = 60.0f;  // 최소 각도 차이

        bool distanceOk = distance > minEscapeDistance;
        bool speedOk = speedAdvantage > minSpeedAdvantage;
        bool angleOk = angleOff > minAngleOff;
        bool aspectOk = aspectAngle > 90.0f;  // 적이 측면 이상에 있어야 함

        std::cout << "[IsEscapeWindowOpen] Distance: " << distanceOk 
                  << ", Speed: " << speedOk << " (" << speedAdvantage << "m/s advantage)"
                  << ", Angle: " << angleOk << ", Aspect: " << aspectOk << std::endl;

        return distanceOk && speedOk && angleOk && aspectOk;
    }
}