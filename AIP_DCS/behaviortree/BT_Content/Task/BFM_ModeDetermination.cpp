#include "BFM_ModeDetermination.h"

namespace Action
{
    PortsList BFM_ModeDetermination::providedPorts()
    {
        return {
            InputPort<CPPBlackBoard*>("BB")
        };
    }

    NodeStatus BFM_ModeDetermination::tick()
    {
        Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");

        float distance = (*BB)->Distance;
        float aspectAngle = (*BB)->MyAspectAngle_Degree;
        float angleOff = (*BB)->MyAngleOff_Degree;
        
        std::cout << "[BFM_DEBUG] Distance: " << distance 
                << ", AspectAngle: " << aspectAngle 
                << ", AngleOff: " << angleOff << std::endl;

        // HABFM
        if (aspectAngle > 120.0f && angleOff > 120.0f)
        {
            (*BB)->BFM = HABFM;
            std::cout << "[BFM_DEBUG] HABFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // DBFM
        if (aspectAngle > 120.0f && angleOff < 60.0f)
        {
            (*BB)->BFM = DBFM;
            std::cout << "[BFM_DEBUG] DBFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // OBFM
        if (aspectAngle < 60.0f)
        {
            (*BB)->BFM = OBFM;
            std::cout << "[BFM_DEBUG] OBFM selected" << std::endl;
            return NodeStatus::SUCCESS;
        }

        // Default = DBFM
        (*BB)->BFM = OBFM;
        std::cout << "[BFM_DEBUG] Default OBFM selected" << std::endl;
        return NodeStatus::SUCCESS;
    }
}