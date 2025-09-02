import numpy as np
import os
import sys
from DogFightEnvWrapper import DogFightWrapper
import CppBT

if __name__ == "__main__":
    print("현재 작업 디렉토리:", os.getcwd())
    print("KAURML.xml 존재:", os.path.exists("KAURML.xml"))
    print("디렉토리 내용:")
    env_config = {
        'max_engage_time': 300,
        'min_altitude': 300,
        # [N, E, D, Roll, Pitch, Initial Heading(deg), Speed(m/s)]
        'ownship': [1000.0, 0.0, -7000.0, 0.0, 0.0, 0.0, 300.0],
        'target': [6000.0, 0.0, -7000.0, 0.0, 0.0, 180.0, 300.0],
    }

    # DLL 파일 확인
    dll_files = ["AIP_DCS.dll", "AIP_DCS_base.dll"]
    for dll in dll_files:
        if os.path.exists(dll):
            print(f"{dll} 존재: True")
        else:
            print(f"{dll} 존재: False")

    try:
        print("=== AIP 객체 생성 시도 ===")
        AIP_ownship = CppBT.AIPilot("AIP_DCS.dll")
        print("AIP_ownship 생성 성공")
        
        AIP_target = CppBT.AIPilot("AIP_DCS_base.dll")  
        print("AIP_target 생성 성공")
        
    except Exception as e:
        print(f"AIP 객체 생성 실패: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

    print("=== 환경 생성 시도 ===")
    try:
        engageEnv = DogFightWrapper(env_config, AIP_ownship, AIP_target)
    except Exception as e:
        print(f"환경 생성 실패: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

    for item in os.listdir('.'):
        print(f"  - {item}")
    env_config = {
        'max_engage_time': 300,
        'min_altitude': 300,
        # [N, E, D, Roll, Pitch, Initial Heading(deg), Speed(m/s)]
        'ownship': [1000.0, 0.0, -7000.0, 0.0, 0.0, 0.0, 300.0],
        'target': [6000.0, 0.0, -7000.0, 0.0, 0.0, 180.0, 300.0],
    }
    AIP_ownship = CppBT.AIPilot("AIP_DCS_ownship.dll")
    AIP_target = CppBT.AIPilot("AIP_DCS_base.dll")
    engageEnv = DogFightWrapper(env_config, AIP_ownship, AIP_target)
        
    engageEnv.reset()
    obs = None
    done = False
    reward = 0.0
    
    while not done:
        
        obs, reward, done, info = engageEnv.step()
        
    if done:
        engageEnv.make_tacviewLog()