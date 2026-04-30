# Getting Started (시작하기)

시작하려면 Pintos를 build(빌드)할 수 있는 machine(컴퓨터)에 login(로그인)해야 합니다. 강사진이 제공한 Linux machine을 사용할 수도 있고, 본인의 Solaris 또는 Linux machine을 사용할 수도 있습니다. 하지만 강사진은 제공된 machine에서 여러분의 code를 test하며, 여기의 지시는 그 environment(환경)를 가정합니다. 본인 machine에 Pintos를 설치하고 작업하는 것은 지원할 수 없습니다. 아래 server environment(서버 환경)가 각 학생에게 제공됩니다.

이제 다음을 실행하여 Pintos source code(소스 코드)를 `pintos`라는 directory에 가져올 수 있습니다.

    
    git clone https://github.com/casys-kaist/pintos-kaist
    

하지만 source code는 **project 기간 전에** 변경될 가능성이 있으므로, [repository를 duplicate](https://help.github.com/en/github/creating-cloning-and-archiving-repositories/duplicating-a-repository)하는 것을 권장합니다. template code(템플릿 코드)가 변경되면, 여러분의 code를 template과 [쉽게 synchronize](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/syncing-a-fork)할 수 있습니다. **PUBLIC FORK를 만들지 마세요.** template code에 치명적인 bug가 있는 경우를 제외하면, project 기간 중에는 source code를 변경하지 않습니다.

참고로 Pintos code와 solution은 제공된 Linux machine과 같은 `Ubuntu 16.04.6 LTS`, gcc(`gcc (Ubuntu 7.4.0-1ubuntu1~16.04~ppa1) 7.4.0`), qemu-system-x86_64(`QEMU emulator version 2.5.0 (Debian 1:2.5+dfsg-5ubuntu10.43)`)에서 test했습니다.

source code를 가져온 뒤에는 pintos root directory에서 아래 command로 environment를 설정할 수 있습니다.

    
    $ source ./activate
    

이 줄을 home directory의 `.bashrc`에 추가해 두는 것이 좋습니다. 그렇지 않으면 login할 때마다 매번 입력해야 합니다.

## Source Tree Overview (소스 트리 개요)

안에 무엇이 있는지 살펴봅시다. `pintos/`에서 다음 directory structure(디렉터리 구조)를 볼 수 있어야 합니다.

  - `threads/`: base kernel(기본 커널)의 source code입니다. project 1부터 수정하게 됩니다.
  - `userprog/`: user program loader(사용자 프로그램 로더)의 source code입니다. project 2부터 수정하게 됩니다.
  - `vm/`: 거의 비어 있는 directory입니다. project 3에서 virtual memory(가상 메모리)를 여기에 구현합니다.
  - `filesys/`: 기본 file system(파일 시스템)의 source code입니다. project 2부터 이 file system을 사용하지만, project 4가 되기 전에는 수정하지 않습니다.
  - `devices/`: keyboard, timer, disk 등 I/O device(입출력 장치) interface(연결 계층)의 source code입니다. project 1에서 timer 구현을 수정합니다. 그 외에는 이 code를 바꿀 필요가 없어야 합니다.
  - `lib/`: standard C library(표준 C 라이브러리)의 일부를 구현한 것입니다. 이 directory의 code는 Pintos kernel과, project 2부터는 그 위에서 실행되는 user program 양쪽에 compile(컴파일)됩니다. 이 code를 수정할 필요는 거의 없습니다.
  - `include/lib/kernel/`: Pintos kernel에만 포함되는 C library의 일부입니다. kernel code에서 자유롭게 사용할 수 있는 몇 가지 data type(자료형) 구현도 포함합니다. bitmap, doubly linked list(이중 연결 리스트), hash table(해시 테이블)이 여기에 있습니다. kernel에서는 이 directory의 header(헤더)를 `#include <...>` 표기법으로 include(포함)할 수 있습니다.
  - `include/lib/user/`: Pintos user program에만 포함되는 C library의 일부입니다. user program에서는 이 directory의 header를 `#include <...>` 표기법으로 include할 수 있습니다.
  - `tests/`: 각 project용 test입니다. 제출물을 test하는 데 도움이 된다면 이 code를 수정할 수 있지만, 강사진은 test를 실행하기 전에 원본으로 교체합니다.
  - `examples/`: project 2부터 사용할 example user program(예제 사용자 프로그램)입니다.
  - `include/`: header file(`*.h`)의 source code입니다.

## Building Pintos (Pintos 빌드하기)

다음 단계로 첫 번째 project를 위해 제공된 source code를 build합니다. 먼저 `threads` directory로 `cd`하세요. 그런 다음 `make` command를 실행합니다. 그러면 `threads` 아래에 `build` directory가 생성되고, 그 안에 `Makefile`과 몇 개의 subdirectory(하위 디렉터리)가 채워진 뒤 kernel이 build됩니다.

build가 끝난 뒤 `build` directory에서 흥미로운 file은 다음과 같습니다.

  - `Makefile`: `pintos/src/Makefile.build`의 copy입니다. kernel을 build하는 방법을 설명합니다.
  - `kernel.o`: 전체 kernel의 object file(목적 파일)입니다. 각 kernel source file에서 compile된 object file들을 하나의 object file로 link(링크)한 결과입니다. debug information(디버그 정보)을 포함하므로, 여기에 대해 GDB(디버거, [GDB](../5_appendix/5_debugging_tools.md#GDB) 참조)나 backtrace(호출 스택 추적, [Backtraces](https://casys-kaist.github.io/pintos-kaist/appendix/debuuging_tools.md#Backtraces) 참조)를 실행할 수 있습니다.
  - `kernel.bin`: kernel의 memory image(메모리 이미지), 즉 Pintos kernel을 실행하기 위해 memory에 load되는 정확한 byte(바이트)들입니다. 이는 debug information이 제거된 `kernel.o`일 뿐입니다. 이렇게 하면 많은 space(공간)를 절약하고, kernel loader 설계가 부과하는 512 kB size limit(크기 제한)에 kernel이 부딪히는 것을 막을 수 있습니다.
  - `loader.bin`: kernel loader(커널 로더)의 memory image입니다. assembly language(어셈블리 언어)로 작성된 작은 code 조각으로, disk에서 kernel을 memory로 읽어 들이고 시작합니다. 길이는 정확히 512 bytes이며, 이는 PC BIOS가 정한 크기입니다. `build`의 subdirectory에는 object file(`.o`)과 dependency file(`.d`)이 들어 있으며, 둘 다 compiler(컴파일러)가 생성합니다. dependency file은 다른 source file이나 header file이 변경되었을 때 어떤 source file을 다시 compile해야 하는지 make에 알려 줍니다.

## Running Pintos (Pintos 실행하기)

Pintos를 simulator(시뮬레이터)에서 편리하게 실행하기 위한 program인 pintos를 제공합니다. 가장 단순한 경우 pintos를 pintos argument... 형태로 호출할 수 있습니다. 각 argument(인자)는 Pintos kernel로 전달되어 처리됩니다. 직접 시도해 보세요. 먼저 새로 생성된 `build` directory로 cd하세요. 그런 다음 pintos run alarm-multiple command를 실행합니다. 이 command는 run alarm-multiple 인자를 Pintos kernel에 전달합니다. 여기서 run은 kernel에게 test를 실행하라고 지시하고, alarm-multiple은 실행할 test입니다. Pintos가 boot(부팅)되고 alarm-multiple test program을 실행하며, 여러 화면 분량의 text를 출력합니다.

command line에서 redirect(리다이렉션)하여 serial output(직렬 출력)을 file에 기록할 수 있습니다. 예를 들어 `pintos -- run alarm-multiple > logfile`입니다. pintos program은 qemu 또는 virtual hardware(가상 하드웨어)를 설정하기 위한 여러 option(옵션)을 제공합니다. option을 지정한다면 Pintos kernel에 전달되는 command보다 앞에 두어야 하며, `\--`로 구분해야 합니다. 전체 command는 `pintos option... -- argument....` 형태가 됩니다. 사용할 수 있는 option 목록을 보려면 argument 없이 pintos를 실행하세요. option에는 VM output(가상 머신 출력)을 표시하는 방식을 정하는 것이 포함됩니다. VGA display를 끄려면 `-v`를 사용하고, stdin에서 serial input을 받거나 stdout으로 출력하는 것을 억제하려면 `-s`를 사용합니다. Pintos kernel에는 run 외에도 command와 option이 있습니다. 지금은 그다지 중요하지 않지만, `pintos -h`처럼 `-h`를 사용하면 목록을 볼 수 있습니다.
