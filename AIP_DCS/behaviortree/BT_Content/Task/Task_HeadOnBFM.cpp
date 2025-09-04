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
            
        float distance = (*BB)->Distance;
        float mySpeed = (*BB)->MySpeed_MS;
        float targetSpeed = (*BB)->TargetSpeed_MS;

        // 이탈 창 확인
        if (IsEscapeWindowOpen(BB.value()) && distance < 3704.0f) // 2nm 이내
        {
            // 전투 중단하고 이탈
            Vector3 myLocation = (*BB)->MyLocation_Cartesian;
            Vector3 myForward = (*BB)->MyForwardVector;
            (*BB)->VP_Cartesian = myLocation + myForward * 5000.0f;
            (*BB)->Throttle = 1.0f;
            return NodeStatus::SUCCESS;
        }

        // 리드 턴 실행 시점 판단
        if (ShouldInitiateLeadTurn(BB.value()))
        {
            (*BB)->VP_Cartesian = CalculateLeadTurn(BB.value());
        }
        else
        {
            // 교차 시 옵션 선택
            if (mySpeed > targetSpeed + 20.0f) // 속도 우위 시
            {
                // 슬라이스 턴 (기수 아래로)
                (*BB)->VP_Cartesian = CalculateSliceTurn(BB.value());
            }
            else if (mySpeed < targetSpeed - 20.0f) // 속도 열세 시
            {
                // 수직 기동
                (*BB)->VP_Cartesian = CalculateVerticalManeuver(BB.value());
            }
            else
            {
                // 수평 선회
                (*BB)->VP_Cartesian = CalculateLevelTurn(BB.value());
            }
        }

        // 코너 속도 유지
        if (mySpeed < 120.0f)
        {
            (*BB)->Throttle = 1.0f;
        }
        else if (mySpeed > 150.0f)
        {
            (*BB)->Throttle = 0.6f;
        }
        else
        {
            (*BB)->Throttle = 0.8f;
        }

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_HeadOnBFM::CalculateSliceTurn(CPPBlackBoard* BB)
    {
        // 슬라이스 턴 - 기수를 10도 정도 아래로 하여 얕은 강하 선회
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myRight = BB->MyRightVector;

        // 적기 방향으로 선회 방향 결정
        Vector3 toTarget = (targetLocation - myLocation);
        float dotRight = toTarget.dot(myRight);

        Vector3 sliceDirection;
        if (dotRight > 0)
        {
            sliceDirection = myRight;
        }
        else
        {
            sliceDirection = myRight * -1.0f;
        }

        // 얕은 강하각도로 슬라이스
        Vector3 slicePoint = myLocation + sliceDirection * 1500.0f + myForward * 2000.0f;
        slicePoint.Z = myLocation.Z - 300.0f; // 약간 하강

        return slicePoint;
    }

    Vector3 Task_HeadOnBFM::CalculateLevelTurn(CPPBlackBoard* BB)
    {
        // 수평 선회 - 적기 시야 유지 가능
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;

        Vector3 toTarget = (targetLocation - myLocation);
        float dotRight = toTarget.dot(myRight);

        Vector3 turnDirection = (dotRight > 0) ? myRight : myRight * -1.0f;

        Vector3 levelTurnPoint = myLocation + turnDirection * 1200.0f;
        levelTurnPoint.Z = myLocation.Z; // 수평 유지

        return levelTurnPoint;
    }

    Vector3 Task_HeadOnBFM::CalculateVerticalManeuver(CPPBlackBoard* BB)
    {
        // 수직 기동 - 속도가 충분할 때만
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myUp = BB->MyUpVector;

        float mySpeed = BB->MySpeed_MS;

        if (mySpeed > 140.0f) // 충분한 속도가 있을 때
        {
            // 수직 상승
            Vector3 verticalPoint = myLocation + myUp * 1000.0f + myForward * 500.0f;
            return verticalPoint;
        }
        else
        {
            // 속도가 부족하면 수평 선회로 대체
            return CalculateLevelTurn(BB);
        }
    }

    Vector3 Task_HeadOnBFM::CalculateLeadTurn(CPPBlackBoard* BB)
    {
        // 리드 턴 - 적기의 3/9 라인을 지나기 전에 앵글 오프 감소
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        Vector3 myRight = BB->MyRightVector;

        // 적기의 미래 위치 예측
        float targetSpeed = BB->TargetSpeed_MS;
        float distance = BB->Distance;
        float closingRate = 200.0f; // 대략적인 접근율
        float timeToMerge = distance / closingRate;

        Vector3 predictedTargetPos = targetLocation + targetForward * targetSpeed * timeToMerge;

        // 예측 위치로 리드 턴
        Vector3 toPredict = (predictedTargetPos - myLocation);
        Vector3 leadTurnPoint = myLocation + toPredict * 1500.0f;

        return leadTurnPoint;
    }

    bool Task_HeadOnBFM::ShouldInitiateLeadTurn(CPPBlackBoard* BB)
    {
        // 적기가 30도 정도 위치에 있고 시선 각속도가 증가할 때 리드 턴 시작
        float distance = BB->Distance;
        float los = BB->Los_Degree;

        // 거리가 가까워지고 LOS가 30도 이내일 때
        return (distance < 5556.0f && los < 30.0f); // 3nm 이내, 30도 이내
    }

    bool Task_HeadOnBFM::IsEscapeWindowOpen(CPPBlackBoard* BB)
    {
        // 이탈 창 열림 판단
        float distance = BB->Distance;
        float mySpeed = BB->MySpeed_MS;
        float targetSpeed = BB->TargetSpeed_MS;
        float angleOff = BB->MyAngleOff_Degree;

        // 거리가 멀고, 속도 우위가 있으며, 높은 앵글 오프일 때
        return (distance > 1852.0f && mySpeed > targetSpeed + 10.0f && angleOff > 90.0f);
    }
}