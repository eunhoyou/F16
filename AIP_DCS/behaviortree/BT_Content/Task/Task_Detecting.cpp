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


        float distance = (*BB)->Distance;
        
        if ((*BB)->EnemyInSight)
        {
            // 6km 이상 원거리이고 시야에 있을 때만 전략적인 후방 점유(Stern Conversion) 시도
            (*BB)->VP_Cartesian = CalculateSternConversion(BB.value());
        }
        else
        {
            // 그 외 모든 경우 (거리가 가깝거나, 시야에 없거나) 에는 가장 빠른 요격 코스 선택
            (*BB)->VP_Cartesian = CalculateInterceptCourse(BB.value());
        }

        (*BB)->Throttle = 0.8f;

        return NodeStatus::SUCCESS;
    }

    Vector3 Task_Detecting::CalculateInterceptCourse(CPPBlackBoard* BB)
    {
        // 일반적인 요격 코스 - 적기로 직진
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myVelocity = BB->MyForwardVector * BB->MySpeed_MS;
        Vector3 targetVelocity = BB->TargetForwardVector * BB->TargetSpeed_MS;

        Vector3 relativeVelocity = targetVelocity - myVelocity;
        Vector3 relativePosition = targetLocation - myLocation;

        // 1. relativePosition의 크기(magnitude) 계산
        float distance = sqrt(relativePosition.X * relativePosition.X +
                            relativePosition.Y * relativePosition.Y +
                            relativePosition.Z * relativePosition.Z);

        if (distance < 0.1f) // 거리가 매우 가까우면 현재 위치 반환
        {
            return targetLocation;
        }

        // 2. relativePosition의 단위 벡터(normalized vector) 계산
        Vector3 relativePositionNormalized = relativePosition / distance;

        // 3. 접근 속도(closing speed) 계산 (두 단위 벡터의 내적 활용)
        float closingSpeed = -relativeVelocity.dot(relativePositionNormalized);

        if (closingSpeed <= 0) // 멀어지고 있다면 현재 적 위치로 향함
        {
            return targetLocation;
        }

        // 4. 예상 요격 시간 및 지점 계산
        float timeToIntercept = distance / closingSpeed;
        Vector3 interceptPoint = targetLocation + targetVelocity * timeToIntercept;

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
        Vector3 sternPoint = targetLocation + offsetDirection * 200.0f - targetForward * 300.0f;

        return sternPoint;
    }
}