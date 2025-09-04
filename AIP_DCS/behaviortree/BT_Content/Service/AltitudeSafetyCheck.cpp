#include "AltitudeSafetyCheck.h"

namespace Action
{
    PortsList AltitudeSafetyCheck::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus AltitudeSafetyCheck::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float currentAltitude = -(*BB)->MyLocation_Cartesian.Z;  // Z는 Down이므로 음수 취함

        // 응급 상황 (600m 이하)
        if (IsAltitudeEmergency(currentAltitude))
        {
            (*BB)->VP_Cartesian = CalculateEmergencyClimb(BB.value());
            (*BB)->Throttle = 1.0f;
            return NodeStatus::SUCCESS;
        }

        // 위험 상황 (800m 이하)
        if (IsAltitudeCritical(currentAltitude))
        {
            Vector3 currentVP = (*BB)->VP_Cartesian;

            // VP의 고도가 현재보다 낮으면 안전 고도로 상승
            if (-currentVP.Z < MIN_SAFE_ALTITUDE)  // NED에서 실제 고도로 변환
            {
                currentVP.Z = -MIN_SAFE_ALTITUDE;  // 1000m 고도를 NED로 변환
                (*BB)->VP_Cartesian = currentVP;
            }

            // 추력 증가로 상승 지원
            if ((*BB)->Throttle < 0.9f)
            {
                (*BB)->Throttle = 0.9f;
            }
        }

        // 일반적인 VP 고도 안전성 검사
        Vector3 plannedVP = (*BB)->VP_Cartesian;
        float plannedAltitude = -plannedVP.Z;  // NED를 실제 고도로 변환

        if (plannedAltitude < MIN_SAFE_ALTITUDE)
        {
            plannedVP.Z = -MIN_SAFE_ALTITUDE;  // 1000m를 NED로 변환 (-1000)
            (*BB)->VP_Cartesian = plannedVP;
        }

        return NodeStatus::SUCCESS;
    }

    Vector3 AltitudeSafetyCheck::CalculateEmergencyClimb(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myUp = BB->MyUpVector;

        // *** 핵심: NED 좌표계에서 상승 = Z값 감소 ***
        // 현재 고도에서 500m 상승 + 최소 안전고도 보장
        float targetAltitude = std::max(-myLocation.Z + 500.0f, MIN_SAFE_ALTITUDE + 500.0f);

        Vector3 emergencyPoint = myLocation;
        emergencyPoint.Z = -targetAltitude;  // 실제 고도를 NED로 변환

        // 약간 전진하며 상승 (실속 방지)
        Vector3 myForward = BB->MyForwardVector;
        emergencyPoint.X += myForward.X * 300.0f;
        emergencyPoint.Y += myForward.Y * 300.0f;

        return emergencyPoint;
    }

    bool AltitudeSafetyCheck::IsAltitudeCritical(float currentAltitude)
    {
        return currentAltitude <= CRITICAL_ALTITUDE;
    }

    bool AltitudeSafetyCheck::IsAltitudeEmergency(float currentAltitude)
    {
        return currentAltitude <= EMERGENCY_ALTITUDE;
    }
}