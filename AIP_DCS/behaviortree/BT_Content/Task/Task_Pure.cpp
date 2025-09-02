#include "Task_Pure.h"

PortsList Action::Task_Pure::providedPorts()
{
	return {
		InputPort<CPPBlackBoard*>("BB")
	};
}

NodeStatus Action::Task_Pure::tick()
{
	Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");
	Vector3 TargetLocation = (*BB)->TargetLocaion_Cartesian;
	(*BB)->VP_Cartesian = TargetLocation;


	return NodeStatus::SUCCESS;
}
