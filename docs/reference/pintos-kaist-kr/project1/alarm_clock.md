### Alarm Clock (알람 시계)

**`devices/timer.c`에 정의된 `timer_sleep()`을 다시 구현하세요.**

동작하는 구현이 제공되어 있지만, 이 구현은 **busy wait(바쁜 대기)**를 합니다. 즉, 충분한 시간이 지날 때까지 loop(반복문) 안에서 현재 시간을 확인하고 `thread_yield()`를 호출하며 계속 돕니다. busy waiting을 피하도록 다시 구현하세요.

* * *
    
    
    void timer_sleep (int64_t ticks);
    

> 호출한 thread(실행 흐름)의 실행을 시간이 최소 x timer ticks(타이머 틱)만큼 진행될 때까지 중단합니다. system(시스템)이 완전히 idle(유휴) 상태가 아닌 한, thread가 정확히 x ticks 뒤에 깨어날 필요는 없습니다. 올바른 시간만큼 기다린 뒤 ready queue(실행 준비 큐)에 넣으면 됩니다.

`timer_sleep()`은 cursor(커서)를 1초에 한 번 깜박이는 경우처럼 real-time(실시간)으로 동작하는 thread에 유용합니다. `timer_sleep()`의 argument(인자)는 milliseconds(밀리초)나 다른 단위가 아니라 timer ticks로 표현됩니다. 1초에는 `TIMER_FREQ`개의 timer ticks가 있으며, `TIMER_FREQ`는 `devices/timer.h`에 정의된 macro(매크로)입니다. 기본값은 `100`입니다. 이 값을 바꾸는 것은 권장하지 않습니다. 어떤 변경이든 많은 test를 실패하게 만들 가능성이 높기 때문입니다.

각각 milliseconds, microseconds(마이크로초), nanoseconds(나노초) 단위로 특정 시간만큼 sleep(절전 대기)하기 위한 별도 함수 `timer_msleep()`, `timer_usleep()`, `timer_nsleep()`도 존재하지만, 필요한 경우 이 함수들은 자동으로 `timer_sleep()`을 호출합니다. 이 함수들을 수정할 필요는 없습니다. alarm clock 구현은 이후 project에는 필요하지 않지만, project 4에서는 유용할 수 있습니다.
