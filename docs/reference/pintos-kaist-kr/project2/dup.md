## Extend File Descriptor (Extra) (파일 디스크립터 확장)

**linux처럼 stdin, stdout close와 dup2 system call(시스템 콜)을 지원하도록 Pintos를 만드세요.**

현재 Pintos implementation(구현)에서는 `stdin`과 `stdout`의 fd(file descriptor, 열린 파일을 가리키는 정수)를 close(닫기)하는 것이 금지되어 있습니다. 이 extra point requirement(추가 점수 요구사항)에서는 먼저 linux와 같이 user(사용자)가 `stdin`과 `stdout`을 close할 수 있게 해야 합니다. 즉, `stdin`을 close하면 process(프로세스)는 input(입력)을 절대 읽지 않아야 하고, `stdout`을 close하면 process는 아무것도 print(출력)하지 않아야 합니다.

다음으로 `dup2` system call을 구현하세요.

* * *
    
    
    int dup2(int oldfd, int newfd);
    

> `dup2()` system call은 file descriptor `oldfd`의 copy(복사본)를 `newfd`에 지정된 file descriptor number(번호)로 만들고, 성공 시 `newfd`를 반환합니다. file descriptor `newfd`가 이미 open되어 있었다면 재사용되기 전에 조용히 close됩니다.
> 
> 다음 사항에 주의하세요.
> 
>   - `oldfd`가 valid file descriptor(유효한 파일 디스크립터)가 아니면 call은 fail(실패)하고 `-1`을 반환하며, `newfd`는 close되지 않습니다.
> 
>   - `oldfd`가 valid file descriptor이고 `newfd`가 `oldfd`와 같은 값이면 `dup2()`는 아무것도 하지 않고 `newfd`를 반환합니다.
> 
> 이 system call이 성공적으로 return한 뒤에는 old file descriptor와 new file descriptor를 서로 바꿔 사용할 수 있습니다. 이들은 **서로 다른 file descriptor**이지만 **같은 open file description(열린 파일 설명)**을 refer(참조)하므로 *file offset(파일 오프셋)*과 *status flags(상태 플래그)*를 공유합니다. 예를 들어 descriptor 중 하나에서 `seek`를 사용해 file offset을 수정하면 다른 descriptor의 offset도 함께 바뀝니다.

`dup`된 file descriptor는 forking(프로세스 복제) 이후에도 semantic(의미)을 보존해야 한다는 점에 주의하세요.

**점수를 받으려면 EXTRA에 대한 모든 TESTCASE를 통과해야 합니다. 전부 아니면 전무입니다.**
