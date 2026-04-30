# Project 4: File Systems (파일 시스템)

이전 두 assignment(과제)에서는 file system(파일 시스템)이 내부에서 어떻게 구현되어 있는지 실제로 걱정하지 않고도 file system을 많이 사용했습니다. 마지막 assignment에서는 file system implementation(구현)을 개선합니다. 주로 `filesys` directory(디렉터리)에서 작업하게 됩니다.

project 4는 project 2 또는 project 3 위에 build(빌드)해도 됩니다. 어느 경우든 filesys 제출물에서는 project 2에 필요한 모든 functionality(기능)가 동작해야 합니다. project 3 위에 build한다면 project 3의 모든 functionality도 동작해야 하며, VM(virtual memory, 가상 메모리) 기능을 enable(활성화)하기 위해 `filesys/Make.vars`를 edit(수정)해야 합니다. VM을 disable(비활성화)하면 10% credit(점수)을 감점합니다. page cache(페이지 캐시, extra credit)는 VM functionality를 필요로 한다는 점에 유의하세요.

## Background (배경)

### New Code (새 코드)

아마 여러분에게 새로운 file들이 있습니다. 별도 표시가 없는 한 `filesys` directory에 있습니다.

  - `filesys/fsutil.c`

> kernel command line(커널 명령줄)에서 접근할 수 있는 file system용 simple utility(간단한 유틸리티)입니다.

  - `include/filesys/filesys.h`, `filesys/filesys.c`

> file system에 대한 top-level interface(최상위 인터페이스)입니다. introduction(소개)은 [Using the File System](../2_project2/0_introduction.md)을 참조하세요.

  - `include/filesys/directory.h`, `filesys/directory.c`

> file name(파일 이름)을 inode(파일 메타데이터 객체)로 translate(변환)합니다. directory data structure(디렉터리 자료구조)는 file로 저장됩니다.

  - `include/filesys/inode.h`, `filesys/inode.c`

> disk(디스크) 위 file data(파일 데이터)의 layout(배치)을 나타내는 data structure를 manage(관리)합니다.

  - `include/filesys/fat.h`, `filesys/fat.c`

> FAT filesystem(File Allocation Table 기반 파일 시스템)을 manage합니다.

  - `include/filesys/file.h`, `filesys/file.c`

> file read/write(파일 읽기/쓰기)를 disk sector read/write(디스크 섹터 읽기/쓰기)로 translate합니다.

  - `include/filesys/page_cache.h`, `filesys/page_cache.c`

> vm functionality를 활용하는 page cache implementation입니다. 이 template(템플릿)을 사용하는 것을 강하게 권장합니다. 하지만 직접 code(코드)를 작성할 수도 있습니다. vm flag(플래그)를 끄면 이 template을 사용할 수 없다는 점에 유의하세요.

  - `include/lib/kernel/bitmap.h`, `lib/kernel/bitmap.c`

> disk file에 bitmap(비트맵)을 읽고 쓰는 routine(루틴)과 함께 bitmap data structure를 제공합니다.

우리 file system은 Unix-like interface(유닉스식 인터페이스)를 가지므로 `creat, open, close, read, write, lseek`, `unlink`에 대한 Unix man page(매뉴얼 페이지)를 읽어 보고 싶을 수 있습니다. 우리 file system의 call(호출)은 이들과 비슷하지만 동일하지는 않습니다. file system은 이 call들을 disk operation(디스크 연산)으로 translate합니다.

위 code에는 basic functionality(기본 기능)가 모두 있으므로, 지난 두 project에서 보았듯 file system은 처음부터 usable(사용 가능)합니다. 하지만 제거해야 할 severe limitation(심각한 제한)이 있습니다.

작업 대부분은 `filesys`에서 이루어지지만, 이전 모든 부분과의 interaction(상호작용)에 대비해야 합니다.

#### Testing File System Persistence (파일 시스템 지속성 테스트)

지금까지 각 test(테스트)는 Pintos를 한 번만 invoke(호출)했습니다. 하지만 file system의 중요한 목적은 한 boot(부팅)에서 다음 boot까지 data(데이터)가 접근 가능한 상태로 남아 있게 하는 것입니다. 따라서 file system project에 속한 test는 Pintos를 두 번째로 invoke합니다. 두 번째 run(실행)은 file system 안의 모든 file과 directory를 하나의 file로 합친 뒤, 그 file을 Pintos file system 밖의 host(Unix) file system으로 copy(복사)합니다.

grading script(채점 스크립트)는 두 번째 run에서 copy되어 나온 file의 contents(내용)를 기준으로 file system correctness(정확성)를 check(확인)합니다. 이는 copy되어 나오는 file을 만드는 Pintos user program(사용자 프로그램)인 `tar`를 지원할 수 있을 만큼 file system이 구현되기 전까지, project가 extended file system test(확장 파일 시스템 테스트)를 통과하지 못한다는 뜻입니다. `tar` program은 꽤 demanding(요구가 많음)합니다. extensible file(확장 가능한 파일)과 subdirectory(하위 디렉터리) support(지원)를 모두 요구하므로 작업이 좀 필요합니다. 그 전까지는 extracted file system(추출된 파일 시스템)에 관한 `make check` error(오류)는 무시해도 됩니다.

덧붙이면, 눈치챘을 수도 있듯 file system contents를 copy해 내는 데 사용하는 file format(파일 형식)은 standard Unix "tar" format입니다. Unix tar program을 사용해 이를 examine(검사)할 수 있습니다. test t에 대한 tar file 이름은 `t.tar`입니다.
