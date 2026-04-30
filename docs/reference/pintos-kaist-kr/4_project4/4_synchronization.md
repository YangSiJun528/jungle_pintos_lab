### Synchronization (동기화)

제공된 file system(파일 시스템)은 external synchronization(외부 동기화)을 요구합니다. 즉 caller(호출자)가 한 번에 하나의 thread(실행 흐름)만 file system code(코드) 안에서 실행되도록 보장해야 합니다. 제출물은 external synchronization을 요구하지 않는 더 fine-grained synchronization strategy(세밀한 동기화 전략)를 채택해야 합니다. 가능한 한 independent entity(독립적인 대상)에 대한 operation(연산)은 서로 independent해야 하므로, 서로 기다릴 필요가 없어야 합니다.

서로 다른 cache block(캐시 블록)에 대한 operation은 independent해야 합니다. 특히 어떤 block에 I/O(입출력)가 필요할 때, I/O가 필요하지 않은 다른 block에 대한 operation은 그 I/O가 완료되기를 기다리지 않고 진행되어야 합니다.

여러 process(프로세스)가 하나의 file(파일)에 동시에 접근할 수 있어야 합니다. 하나의 file에 대한 여러 read(읽기)는 서로 기다리지 않고 완료될 수 있어야 합니다. file에 write(쓰기)하더라도 file을 extend(확장)하지 않는 경우에는 여러 process가 하나의 file에 동시에 write할 수도 있어야 합니다. 한 process가 file을 write하는 동안 다른 process가 그 file을 read하면, write가 전혀 완료되지 않았거나, 모두 완료되었거나, 일부만 완료된 것으로 보이는 것이 허용됩니다. 하지만 `write` system call(시스템 콜)이 caller에게 return(반환)한 뒤에는 이후 모든 reader(읽는 쪽)가 change(변경)를 봐야 합니다. 마찬가지로 두 process가 같은 file의 같은 part(부분)에 동시에 write하면 data(데이터)가 interleave(섞임)될 수 있습니다.

반면 file을 extend하고 새 section(구간)에 data를 write하는 것은 atomic(원자적)이어야 합니다. process A와 B가 모두 어떤 file을 open(열기)하고 있고 둘 다 end-of-file(파일 끝)에 위치해 있다고 합시다. A가 read하고 B가 동시에 file에 write하면, A는 B가 쓴 것의 전부, 일부, 또는 아무것도 읽지 않을 수 있습니다. 하지만 A는 B가 쓴 것이 아닌 data를 읽어서는 안 됩니다. 예를 들어 B의 data가 모두 nonzero byte(0이 아닌 바이트)라면, A가 zero를 보는 것은 허용되지 않습니다.

서로 다른 directory(디렉터리)에 대한 operation은 concurrently(동시에) 수행되어야 합니다. 같은 directory에 대한 operation은 서로 기다릴 수 있습니다.

여러 thread가 공유하는 data만 synchronize할 필요가 있다는 점을 기억하세요. base file system(기본 파일 시스템)에서 `struct file`과 `struct dir`은 하나의 thread에서만 access(접근)됩니다.
