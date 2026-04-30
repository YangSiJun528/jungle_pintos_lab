# Grading (채점)

과제는 test result(테스트 결과)를 기준으로 채점합니다.

## Testing (테스트)

test result grade(테스트 결과 점수)는 강사진의 test를 기준으로 합니다. 각 project에는 여러 test가 있으며, 각 test 이름은 `tests`로 시작합니다. 제출물을 전체적으로 test하려면 project의 `build` directory(디렉터리)에서 make check를 실행하세요. 그러면 각 test를 build(빌드)하고 실행한 뒤, 각각에 대해 `pass` 또는 `fail` message를 출력합니다. test가 실패하면 make check는 실패 이유에 대한 세부 정보도 출력합니다. 모든 test를 실행한 뒤에는 test result 요약도 출력합니다. 개별 test를 하나씩 실행할 수도 있습니다. 어떤 test `t`는 출력을 `t.output`에 기록하고, script(스크립트)가 그 출력을 `pass` 또는 `fail`로 채점한 뒤 판정을 `t.result`에 기록합니다. 단일 test를 실행하고 채점하려면 `build` directory에서 `.result` file을 명시적으로 만들면 됩니다. 예를 들어 `make tests/threads/alarm-multiple.result`를 실행합니다. make가 test result가 최신이라고 말하지만 다시 실행하고 싶다면, make clean을 실행하거나 `.output` file을 직접 삭제하세요. 기본적으로 각 test는 실행 중에는 feedback(피드백)을 제공하지 않고 완료 시에만 제공합니다. 원한다면 make command line(명령줄)에 `VERBOSE=1`을 지정하여 각 test의 진행 상황을 볼 수 있습니다. 예를 들면 make check `VERBOSE=1`입니다. 모든 test와 관련 file은 `pintos/src/tests`에 있습니다. 제출물을 test하기 전에, 올바른 test가 사용되도록 그 directory의 내용을 깨끗하고 수정되지 않은 원본 copy로 교체합니다. 따라서 debugging(디버깅)에 도움이 된다면 일부 test를 수정할 수 있지만, 강사진은 원본 test를 실행합니다. 모든 software에는 bug(결함)가 있으므로 강사진의 test 중 일부도 잘못되었을 수 있습니다. test failure(테스트 실패)가 여러분 code의 bug가 아니라 test의 bug라고 생각한다면 지적해 주세요. 검토하고 필요하면 수정하겠습니다. test suite(테스트 모음)를 공개하는 강사진의 관대함을 악용하려고 하지 마세요. 여러분의 code는 강사진이 제공하는 test case(테스트 사례)에 대해서만이 아니라 일반적인 경우에도 올바르게 동작해야 합니다. 예를 들어 실행 중인 test case 이름을 바탕으로 kernel(운영체제 핵심부)의 동작을 명시적으로 바꾸는 것은 허용되지 않습니다. 이런 방식으로 test case를 우회하려는 시도는 점수를 받지 못합니다. 이 부분에서 자신의 solution(해결 방법)이 애매한 영역에 있다고 생각되면 강사진에게 문의하세요.

## Submission (제출)

archive file(압축 제출 파일)을 만드는 command를 제공합니다. `threads`, `userprog`, `vm`, `filesys` directory가 있는 pintos project root directory로 이동하세요. `TEAM=YOUR_TEAM_NUMBER make archive`를 입력합니다. 예를 들어 `TEAM=5 make archive`입니다. 그러면 같은 directory에 `team5.tar.gz`가 생성됩니다. 이 archive file을 제출하세요. 중복 제출은 **하지 마세요**. 한 team에는 한 번의 제출이면 충분합니다.
