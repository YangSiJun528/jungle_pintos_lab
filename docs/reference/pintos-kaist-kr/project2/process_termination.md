## Process Termination Message (프로세스 종료 메시지)

**process termination message(프로세스 종료 메시지)를 출력하세요.**

user process(사용자 프로세스)가 `exit`을 호출했든 다른 이유든 terminate(종료)될 때마다, **process의 name(이름)과 exit code(종료 코드)**를 다음 `printf`로 출력한 것처럼 format(형식화)하여 출력하세요.
    
    
    printf ("%s: exit(%d)\n", ...);
    

출력되는 name은 `fork()`에 전달된 full name(전체 이름)이어야 합니다. user process가 아닌 kernel thread(커널 스레드)가 terminate될 때나 halt system call(중지 시스템 콜)이 invoke(호출)될 때는 이 message를 출력하지 마세요. process가 load(적재)에 실패한 경우에는 message 출력이 optional(선택)입니다.

이것 외에는 제공된 Pintos가 이미 출력하지 않는 다른 message를 출력하지 마세요. debugging(디버깅) 중에는 추가 message가 유용할 수 있지만, grading script(채점 스크립트)를 혼란스럽게 하므로 점수가 낮아집니다.
