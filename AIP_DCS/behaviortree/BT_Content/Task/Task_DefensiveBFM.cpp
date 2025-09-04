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
        Vector3 calculated_vp; // 계산된 VP를 담을 변수

        // 미사일 공격 상황 확인
        if (IsUnderMissileAttack(BB.value()))
        {
            // [로그 추가] 어떤 기동이 선택되었는지 확인
            std::cout << "  -> DefensiveBFM: 미사일 회피 기동 선택!" << std::endl;
            calculated_vp = CalculateBeamManeuver(BB.value());
        }
        else if (distance < 914.4f)
        {
            // [로그 추가] 어떤 기동이 선택되었는지 확인
            std::cout << "  -> DefensiveBFM: 기총 방어(징킹) 기동 선택!" << std::endl;
            calculated_vp = CalculateJinkManeuver(BB.value());
        }
        else
        {
            // [로그 추가] 어떤 기동이 선택되었는지 확인
            std::cout << "  -> DefensiveBFM: 기본 방어 선회 선택!" << std::endl;
            calculated_vp = CalculateDefensiveTurn(BB.value());
        }

        // [로그 추가] 블랙보드에 넣기 직전의 VP 값 확인
        std::cout << "[TASK_DEBUG] DefensiveBFM calculated VP: ("
            << calculated_vp.X << ", " << calculated_vp.Y << ", " << calculated_vp.Z << ")" << std::endl;

        (*BB)->VP_Cartesian = calculated_vp;

        // 방어 시 최대 스로틀
        (*BB)->Throttle = 1.0f;

        // 참고: 방어 기동은 여러 프레임에 걸쳐 지속되므로 RUNNING을 반환하는 것이 일반적입니다.
        // 하지만 지금은 디버깅이 우선이므로 SUCCESS도 괜찮습니다.
        return NodeStatus::SUCCESS;
    }

    Vector3 Task_DefensiveBFM::CalculateDefensiveTurn(CPPBlackBoard* BB)
    {
        // 적기에게 BFM 문제 유발 - 양력벡터를 적기에게 놓고 최대 선회
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myRight = BB->MyRightVector;

        // 적기 방향 벡터
        Vector3 toTarget = targetLocation - myLocation;

        // 적기 쪽으로 가장 빠른 선회 방향 결정
        float dotRight = toTarget.dot(myRight);

        Vector3 turnDirection;
        if (dotRight > 0)
        {
            // 오른쪽으로 선회
            turnDirection = myRight;
        }
        else
        {
            // 왼쪽으로 선회
            turnDirection = myRight * -1.0f;
        }

        // 최대 선회율로 적기 쪽으로 선회
        Vector3 defensivePoint = myLocation + turnDirection * 1000.0f;
        defensivePoint.Z = myLocation.Z; // 수평 선회 유지

        return defensivePoint;
    }

    Vector3 Task_DefensiveBFM::CalculateBeamManeuver(CPPBlackBoard* BB)
    {
        // 미사일을 3/9 라인(빔) 위치에 놓는 기동
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 targetLocation = BB->TargetLocaion_Cartesian;
        Vector3 myRight = BB->MyRightVector;

        // 적기에 대해 90도 방향으로 기동
        Vector3 beamDirection = myRight;
        Vector3 beamPoint = myLocation + beamDirection * 2000.0f;

        return beamPoint;
    }

    Vector3 Task_DefensiveBFM::CalculateJinkManeuver(CPPBlackBoard* BB)
    {
        // 기총 방어를 위한 불규칙 기동
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myUp = BB->MyUpVector;
        Vector3 myRight = BB->MyRightVector;

        // 시간 기반 불규칙 기동 패턴
        double time = BB->RunningTime;
        int pattern = ((int)(time * 2.0)) % 4;

        Vector3 jinkDirection;
        switch (pattern)
        {
        case 0: // 위로
            jinkDirection = myUp;
            break;
        case 1: // 오른쪽으로
            jinkDirection = myRight;
            break;
        case 2: // 아래로
            jinkDirection = myUp * -1.0f;
            break;
        case 3: // 왼쪽으로
            jinkDirection = myRight * -1.0f;
            break;
        }

        Vector3 jinkPoint = myLocation + jinkDirection * 500.0f + myForward * 1000.0f;

        return jinkPoint;
    }

    bool Task_DefensiveBFM::IsUnderMissileAttack(CPPBlackBoard* BB)
    {
        // 미사일 공격 판단 로직
        float los = BB->Los_Degree;
        float distance = BB->Distance;

        // 적기가 나를 향하고 있고 공격 사거리 내인지 확인
        return (distance > 152.4f && distance < 914.4f && los < 30.0f);
    }
}