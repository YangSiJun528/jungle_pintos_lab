# Pintos filesys 질문과 답변

작성일: 2026-05-20

이 문서는 Pintos 파일 시스템을 공부하면서 나온 질문과 답변을 계속 축적하는 Q&A 문서다. 긴 설명 문서가 아니라, 나중에 다시 볼 수 있도록 질문의 의도와 핵심 답변을 매끄럽게 요약해 남기는 것을 목표로 한다.

## 운영 규칙

- 이 문서에는 Pintos filesys, FAT, inode, directory, path resolution, file growth, symlink, filesys syscall, filesys synchronization처럼 파일 시스템과 직접 관련된 내용만 남긴다.
- VM, thread, scheduler, userprog 같은 다른 프로젝트 내용은 filesys 이해에 직접 필요한 경우에만 짧게 언급한다.
- 답변은 원래 대화보다 압축해서 기록한다.
- 구현 판단은 로컬 reference 중 `docs/reference/pintos-kaist-original/4_project4`와 현재 구현 정리 문서인 `docs/ai/038_Pintos_filesys_구현_정리.md`를 우선한다.
- 초심자용 전체 흐름은 `docs/ai/039_Pintos_filesys_초심자_구현_흐름.md`를 참고한다.

## 질문과 답변

## Q1. sector 단위로 읽고 쓰는 이유는 하드웨어가 그렇게 지원하기 때문인가요? Pintos의 디스크 모델은 무엇이고, 드라이버 기능까지 수행하나요?

네. filesys가 sector 단위로 읽고 쓰는 가장 직접적인 이유는 Pintos가 다루는 디스크 장치 인터페이스가 sector 단위 I/O를 제공하기 때문이다. 현재 저장소의 `DISK_SECTOR_SIZE`는 512바이트이고, `disk_read()`와 `disk_write()`는 정확히 한 sector를 읽거나 쓰는 인터페이스다.

Pintos 안에서 디스크는 ATA/IDE 디스크처럼 보인다. 실제 실행은 QEMU/Bochs 같은 가상 머신 위에서 이루어지고, host의 `filesys.dsk` 같은 파일이 가상 디스크로 연결된다. 하지만 Pintos kernel 입장에서는 단순 파일이 아니라 legacy ATA 채널에 붙은 디스크 장치로 보이며, `devices/disk.c`가 이 장치를 조작한다.

계층은 다음처럼 나뉜다.

```text
filesys/file.c, inode.c
  - 파일 offset과 길이를 보고 필요한 disk sector를 결정한다.
  - disk_read(), disk_write()를 호출한다.

devices/disk.c
  - ATA/IDE 디스크 드라이버 역할을 한다.
  - I/O port에 ATA register 값을 쓰고 읽는다.
  - READ SECTOR, WRITE SECTOR command를 발행한다.
  - 디스크 interrupt를 기다린다.
  - data port를 통해 512바이트 sector를 전송한다.

가상 머신의 ATA 디스크
  - host의 disk image 파일을 실제 저장소처럼 제공한다.
```

따라서 Pintos의 filesys 과제에서 구현하는 것은 디스크 드라이버가 아니라 그 위의 파일 시스템이다. 드라이버 수준 기능은 이미 `devices/disk.c`가 수행한다. filesys는 sector 번호를 계산하고 `disk_read()`/`disk_write()`를 호출할 뿐, ATA port나 interrupt를 직접 다루지 않는다.

현재 구현에서 중요한 연결은 다음이다.

- `filesys_init()`은 `disk_get(0, 1)`로 파일 시스템용 디스크를 얻는다.
- Pintos의 디스크 번호 convention에서 `0:1`은 file system disk다.
- `inode.c`는 byte offset을 sector 번호로 바꾼 뒤 `disk_read()` 또는 `disk_write()`를 호출한다.
- `disk.c`는 내부적으로 channel lock, ATA command, interrupt wait, PIO data transfer를 처리한다.

정리하면, sector는 filesys가 마음대로 정한 단위가 아니라 디스크 장치 인터페이스가 제공하는 block I/O 단위다. Pintos에서는 `devices/disk.c`가 간단한 ATA/IDE 디스크 드라이버 역할을 하고, filesys는 그 위에서 파일과 디렉터리 의미를 구현한다.
