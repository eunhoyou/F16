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

        float currentAltitude = (*BB)->MyLocation_Cartesian.Z;

        if (IsAltitudeEmergency(currentAltitude))
        {
            (*BB)->VP_Cartesian = CalculateEmergencyClimb(BB.value());
            (*BB)->Throttle = 1.0f;
            return NodeStatus::SUCCESS;
        }

        if (IsAltitudeCritical(currentAltitude))
        {
            Vector3 currentVP = (*BB)->VP_Cartesian;

            // VP의 고도가 현재보다 낮으면 안전 고도로 상승
            if (currentVP.Z < MIN_SAFE_ALTITUDE)
            {
                currentVP.Z = MIN_SAFE_ALTITUDE;
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
        if (plannedVP.Z < MIN_SAFE_ALTITUDE)
        {
            plannedVP.Z = std::max(plannedVP.Z, MIN_SAFE_ALTITUDE);
            (*BB)->VP_Cartesian = plannedVP;
        }

        return NodeStatus::SUCCESS;
    }

    Vector3 AltitudeSafetyCheck::CalculateEmergencyClimb(CPPBlackBoard* BB)
    {
        Vector3 myLocation = BB->MyLocation_Cartesian;
        Vector3 myForward = BB->MyForwardVector;
        Vector3 myUp = BB->MyUpVector;

        // 긴급 상승: 현재 위치에서 수직으로 상승
        Vector3 emergencyPoint = myLocation + myUp * 500.0f;
        emergencyPoint.Z = std::max(emergencyPoint.Z, MIN_SAFE_ALTITUDE + 500.0f);

        // 약간 전진하며 상승 (실속 방지)
        emergencyPoint = emergencyPoint + myForward * 300.0f;

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