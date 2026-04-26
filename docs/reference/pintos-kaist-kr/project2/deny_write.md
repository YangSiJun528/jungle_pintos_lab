### Deny Write on Executables (실행 파일 쓰기 금지)

**executable(실행 파일)에 대한 write(쓰기)를 막으세요.**

executable로 사용 중인 file(파일)에 write하지 못하도록 code(코드)를 추가하세요. 많은 OS(운영체제)가 이렇게 합니다. process(프로세스)가 disk(디스크)에서 변경되는 중인 code를 실행하려고 하면 예측할 수 없는 결과가 생기기 때문입니다. 이는 project 3에서 virtual memory(가상 메모리)가 구현된 뒤 특히 중요하지만, 지금 적용해도 해가 되지 않습니다.

open file(열린 파일)에 대한 write를 막기 위해 `file_deny_write()`를 사용할 수 있습니다. 그 file에 대해 `file_allow_write()`를 호출하면 write가 다시 가능해집니다(다른 opener(열어 둔 주체)가 write를 deny하고 있지 않다면). file을 close(닫기)해도 write가 다시 가능해집니다. 따라서 process의 executable에 대한 write를 deny하려면, process가 계속 실행 중인 동안 그 file을 open 상태로 유지해야 합니다.
