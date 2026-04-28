---
marp: true
theme: default
paginate: true
size: 16:9
# 설치:
# brew install marp-cli
#
# 서버 실행:
# PORT=5001 marp -s docs/ai
# http://localhost:5001/003_pintos_ASSERT_발표.marp.md
#
# 발표자 노트:
# HTML 화면에서 p 키를 눌러 Presenter View 열기
#
# PDF 생성:
# marp docs/ai/003_pintos_ASSERT_발표.marp.md -o docs/ai/003_pintos_ASSERT_발표.pdf
#
# HTML 생성:
# marp docs/ai/003_pintos_ASSERT_발표.marp.md -o docs/ai/003_pintos_ASSERT_발표.html
#
# 노트만 추출:
# marp docs/ai/003_pintos_ASSERT_발표.marp.md --notes -o docs/ai/003_pintos_ASSERT_발표_notes.txt
---

<!-- _class: lead -->

# Pintos `ASSERT`

## 깨져서는 안 되는 조건을 코드로 남기기

<!--
안녕하세요. 5조 양시준입니다.
이번 발표에서는 ASSERT를 사용해야 하는 이유와, 주의점에 대해서 가볍게 소개하려고 합니다.
-->

---

# 왜 추천하나?

```text
Kernel PANIC at ../../threads/synch.c:237 in lock_acquire():
assertion `!lock_held_by_current_thread (lock)' failed.
Call stack: 0x800426eff 0x8004202fb ...
```

```text
0x0000000800426eff: debug_panic (lib/debug.c:86)
0x00000008004202fb: file_seek (filesys/file.c:405)
0x000000080042dc22: seek (userprog/syscall.c:744)
0x000000080042cf67: syscall_handler (userprog/syscall.c:444)
...
```

깨진 조건과 호출 경로를 바로 보여준다.

<!--
Pintos의 ASSERT는 C에서 기본적으로 제공되는 assert와 달리,
실패하면 단순히 멈추는 게 아니라,
어떤 조건이 깨졌는지와 어느 함수에서 터졌는지 보여줍니다.
call stack과 backtrace를 알려주어 함수와 파일 위치까지 볼 수 있습니다.
-->

---

# 어디에 쓰나?

`pintos/lib/kernel/list.c` - `list_sort()`

```c
void
list_sort (struct list *list, list_less_func *less, void *aux) {
  ASSERT (list != NULL);
  ASSERT (less != NULL);

  ...

  ASSERT (is_sorted (list_begin (list),
                     list_end (list), less, aux));
}
```

선조건과 후조건을 실제 코드에서 검사한다.

<!--
ASSERT는 함수의 선조건과 후조건을 검사하는 데 쓰기 좋습니다.
예를 들어 제가 ready_list 정렬에 사용한 list_sort 내부에서도
list와 비교 함수가 NULL이 아닌지 먼저 확인하고,
정렬이 끝난 뒤 실제로 정렬됐는지 다시 ASSERT로 확인합니다.
-->

---

# 내 코드 예시

```c
void
threads_wakeup (int64_t ticks) {
  ASSERT (intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  ...
}
```

timer interrupt 안에서만 호출된다는 전제를 검증한다.

<!--
제가 ASSERT를 사용한 예시는 threads_wakeup입니다.
이 함수는 timer interrupt에서 깨울 thread를 처리하는 함수라서,
interrupt context 안에서 호출되어야 하고 interrupt도 꺼져 있어야 합니다.
그래서 이 두 ASSERT로 함수가 기대한 문맥에서 호출되는지 확인했습니다.
-->

---

# 팀플에서 좋은 점 1

```c
void
threads_wakeup (int64_t ticks) {
  // interrupt handler에서만 호출되어야 한다.
  // interrupt는 꺼진 상태여야 한다.

  ...
}
```

주석은 의도를 설명하지만, 실행 중에 검증하지는 않는다.

<!--
팀 프로젝트에서는 함수 호출 의도를 팀원에게 전달하는 것도 중요합니다.
주석으로 이 함수가 어떤 문맥에서 호출되어야 하는지 설명할 수는 있습니다.
하지만 주석은 틀리거나 지켜지지 않아도 실행 중에는 아무 일도 일어나지 않습니다.
-->

---

# 팀플에서 좋은 점 2

```c
void
threads_wakeup (int64_t ticks) {
  ASSERT (intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  ...
}
```

`ASSERT`는 의도를 코드로 보여주고, 잘못 호출하면 바로 실패한다.

<!--
반면 ASSERT로 쓰면 의도를 코드로 보여주는 동시에 실제 검증도 수행합니다.
팀원이 이 함수를 잘못된 문맥에서 호출하면 조용히 지나가지 않고 바로 실패합니다.
-->

---

# 주의점

```c
/* 나쁜 예: index++는 side effect다. */
ASSERT (index++ > 0);
```

`ASSERT`가 꺼지면 `index++`도 사라진다.

<br>

```c
/* 좋은 예: 상태 변경과 검증을 분리한다. */
ASSERT (index > 0);
index++;
```

<!--
주의할 점은 ASSERT 안에 상태가 바뀌는 동작을 넣으면 안 된다는 것입니다.
ASSERT는 개발/디버깅 모드에서만 활성화됩니다.
따라서 ASSERT 안에서 index++를 하면,
ASSERT가 적용되지 않는 빌드에서는 index 증가 자체가 사라집니다.
-->

---

<!-- _class: lead -->

# 결론

`ASSERT`를 많이 쓰자.

<!--
이상으로 발표를 마치겠습니다. 감사합니다.
-->
