#include "DECO_BFMCheck.h"

namespace Action
{
	PortsList DECO_BFMCheck::providedPorts()
	{
		return {
			InputPort<CPPBlackBoard*>("BB"),
			InputPort<std::string>("CheckBFM")
		};
	}

	NodeStatus DECO_BFMCheck::tick()
	{
		Optional<CPPBlackBoard*> BB = getInput<CPPBlackBoard*>("BB");
		Optional<std::string> CheckBFM = getInput<std::string>("CheckBFM");

		BFM_Mode CurrentBFM = (*BB)->BFM;
		std::cout << "[DECO_BFMCheck] Current BFM: " << (int)CurrentBFM 
              << ", Checking for: " << CheckBFM.value() << std::endl;
		BFM_Mode InputBFM;

		std::string str = CheckBFM.value();
		if(str == "OBFM")
		{
			InputBFM = OBFM;
		}
		else if (str == "DBFM")
		{
			InputBFM = DBFM;
		}
		else if (str == "HABFM")
		{
			InputBFM = HABFM;
		}
		else if (str == "SCISSORS")
		{
			InputBFM = SCISSORS;
		}
		else if (str == "DETECTING")
		{
			InputBFM = DETECTING;
		}
		else
		{
			//CheckBFM 입력 문자열이 오타난건 아닌지 확인 필요!!!! OBFM,DBFM, HABFM, SCISSORS, DETECTING 가 아님
			InputBFM = NONE;
		}

		if (CurrentBFM == InputBFM)
		{
			std::cout << "[DECO_BFMCheck] BFM match - SUCCESS" << std::endl;
			return NodeStatus::SUCCESS;
		}
		else
		{
			std::cout << "[DECO_BFMCheck] BFM mismatch - FAILURE" << std::endl;
			return NodeStatus::FAILURE;
		}
	}

}