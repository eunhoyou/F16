#include "Task_Detecting.h"

namespace Action
{
    PortsList Task_Detecting::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus Task_Detecting::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        bool enemyInSight = (*BB)->EnemyInSight;
        float distance = (*BB)->Distance;

        if (!enemyInSight)
        {
            // 적기를 시야에서 찾지 못한 경우 - 수색 패턴
            (*BB)->VP_Cartesian = CalculateSearchPattern(BB.value());
        }
        else if (distance > 18520.0f) // 10nm 이상 BVR 상황
        {
            // 가시거리 밖 요격 - 스턴 컨버전
            (*BB)->VP_Cartesian = CalculateSternConversion(BB.value());
        }
        else
        {
            // 일반 요격 코스
            (*BB)->VP_Cartesian = CalculateInterceptCourse(BB.value());
        }

        // 탐지 시에는 순항 스로틀
        (*BB)->Throttle = 0.8f;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_Detecting::CalculateSearchPattern(CPPBlackBoard* BB)
    {
        // 적기를 찾기 위한 수색 패턴 비행
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myRight = BB->MyRightVector;

        // 시간 기반 S자 수색 패턴
        double time = BB->RunningTime;
        float patternPhase = fmod(time, 20.0); // 20초 주기

        Vector3 searchDirection;
        if (patternPhase < 10.0)
        {
            // 오른쪽으로 수색
            searchDirection = myRight;
        }
        else
        {
            // 왼쪽으로 수색
            searchDirection = myRight * -1.0f;
        }

        Vector3 searchPoint = myLocation + myForward * 3000.0f + searchDirection * 1500.0f;

        return searchPoint;
    }

    Vector3 Task_Detecting::CalculateInterceptCourse(CPPBlackBoard* BB)
    {
        // 일반적인 요격 코스 - 적기로 직진
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        float targetSpeed = BB->TargetSpeed_MS;
        float mySpeed = BB->MySpeed_MS;
        float distance = BB->Distance;

        // 예상 요격 시간 계산
        float interceptTime = distance / (mySpeed + targetSpeed * 0.5f);

        // 적기의 예상 위치
        Vector3 interceptPoint = targetLocation + targetForward * targetSpeed * interceptTime;

        return interceptPoint;
    }

    Vector3 Task_Detecting::CalculateSternConversion(CPPBlackBoard* BB)
    {
        // 스턴 컨버전 요격 - 적기 후방으로 접근
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 targetForward = BB->TargetForwardVector;
        Vector3 targetRight = BB->TargetRightVector;
        float aspectAngle = BB->MyAspectAngle_Degree;

        // 에스펙트 앵글의 반대편으로 옵셋
        Vector3 offsetDirection;
        if (aspectAngle > 180.0f) // 좌측 에스펙트
        {
            offsetDirection = targetRight;
        }
        else // 우측 에스펙트
        {
            offsetDirection = targetRight * -1.0f;
        }

        // 적기 측면으로 이동하여 터닝 룸 확보
        Vector3 sternPoint = targetLocation + offsetDirection * 2000.0f - targetForward * 3000.0f;

        return sternPoint;
    }
}