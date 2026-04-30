# Introduction (소개)

KAIST CS330의 Pintos project에 오신 것을 환영합니다. 이번 학기에는 KAIST CS330 수업에 맞게 수정된 Pintos 버전을 사용합니다.

요구사항이 원본 Pintos와 다소 다르므로, **반드시 이 매뉴얼의 지시를 우선 따라야 합니다.** 또한 **본인이 이전에 수행한** Pintos project를 참고할 수는 있지만, **업데이트된 요구사항에 맞게 일부 요구사항을 다시 구현해야 합니다.**

이 변경사항 때문에 강사진은 매뉴얼도 수정했습니다. 강사진은 매뉴얼을 수정된 버전과 동기화하려고 노력했지만, 아직 맞지 않는 부분이 있을 수 있습니다. 그런 부분을 발견하면 강사진에게 알려 주세요.

**반드시 [Legal and Ethical Issues](3_legal_and_ethical_issues.md) 섹션을 읽어야 합니다.**

pintos-kaist는 x86-64 architecture(컴퓨터 명령어 집합 및 실행 구조)를 위한 단순한 operating system framework(운영체제 프레임워크)입니다. 이 project는 Stanford University의 Pintos project에서 fork(기존 저장소를 바탕으로 분기)되었습니다. 앞으로는 pintos-kaist 대신 Pintos라는 이름을 사용하겠습니다. Pintos는 kernel thread(운영체제 내부 실행 흐름), user program(사용자 영역 프로그램)의 로딩과 실행, file system(파일 시스템)을 지원하지만, 이 모든 것을 매우 단순한 방식으로 구현합니다. Pintos project에서 여러분과 project team은 이 세 영역의 지원을 강화하게 됩니다. 또한 virtual memory(가상 메모리) 구현도 추가합니다. 이론적으로 Pintos는 일반 x86-64 PC에서도 실행될 수 있습니다. 하지만 모든 CS330 학생에게 Pintos용 전용 PC를 제공하는 것은 현실적이지 않습니다. 따라서 Pintos project는 system simulator(하드웨어를 흉내 내는 프로그램)에서 실행합니다. simulator는 수정되지 않은 operating system과 software가 그 위에서 실행될 수 있을 만큼 x86-64 CPU와 주변 장치를 정확하게 시뮬레이션하는 프로그램입니다. 수업에서는 **QEMU** simulator를 사용합니다. 이 project들은 어렵습니다. CS330은 시간이 많이 드는 과목으로 알려져 있으며, 그럴 만한 이유가 있습니다. 많은 지원 자료를 제공하는 등 workload(작업량)를 줄이기 위해 가능한 일을 하겠지만, 여전히 수행해야 할 어려운 작업이 많습니다. 여러분의 feedback(의견)을 환영합니다. 과제의 불필요한 overhead(부담)를 줄이고 핵심적인 문제에 집중할 수 있게 하는 제안이 있다면 알려 주세요. 이 장은 Pintos 작업을 시작하는 방법을 설명합니다. 어떤 project를 시작하기 전에 이 장 전체를 읽어야 합니다.
