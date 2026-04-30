# Development Tools (개발 도구)

code(코드)를 개발하는 동안 유용할 수 있는 도구 몇 가지입니다.

## Tags (태그)

Tags(태그)는 program(프로그램)에 선언된 function(함수)과 global variable(전역 변수)에 대한 index(색인)입니다. Emacs와 vi를 포함한 많은 editor(편집기)가 이를 사용할 수 있습니다. `pintos/`의 `Makefile`은 make `TAGS` command(명령)로 Emacs-style tags를 만들거나, make tags로 vi-style tags를 만듭니다.

Emacs에서는 현재 window(창)에서 tag를 따라가려면 `M-`.를, 새 window에서는 `C-x 4` .를, 새 frame(프레임)에서는 `C-x 5` .를 사용합니다. 이 command 중 하나를 사용할 때 cursor(커서)가 symbol name(심볼 이름)에 있으면 그것이 default target(기본 대상)이 됩니다. tag name에 definition(정의)이 여러 개 있으면 `M-0 M-`.가 다음 것으로 jump(점프)합니다. 마지막 tag를 따라가기 전 위치로 돌아가려면 `M-*`를 사용합니다.

## cscope

`cscope` program도 program에 선언된 function과 variable의 index를 제공합니다. tag facility(태그 기능)에 없는 기능도 있습니다. 특히 어떤 function이 호출되는 program 안의 모든 지점을 찾을 수 있습니다.

`pintos/`의 `Makefile`은 make `cscope`로 invoke(호출)될 때 `cscope` index를 만듭니다. index가 생성되면 shell command line(셸 명령줄)에서 `cscope`를 실행하세요. 일반적으로 command-line argument(명령줄 인자)는 필요 없습니다. 그런 다음 terminal(터미널) 아래쪽 근처에 나열된 search criteria(검색 기준) 중 하나를 arrow key(화살표 키)로 선택하고 identifier(식별자)를 입력한 뒤 `Enter`를 누릅니다. `cscope`는 terminal 위쪽에 match(일치 항목)를 표시합니다. arrow key로 특정 match를 선택할 수 있고, 그 상태에서 `Enter`를 누르면 `cscope`가 default system editor(기본 시스템 편집기) [1](#fn_1)를 invoke하고 cursor를 해당 match 위치로 이동합니다. 새 search를 시작하려면 `Tab`을 입력합니다. `cscope`를 exit(종료)하려면 `Ctrl-d`를 입력합니다.

Emacs와 일부 vi version(버전)은 `cscope`에 대한 자체 interface(인터페이스)를 가집니다. 이 interface 사용법은 `cscope` home page(<http://cscope.sourceforge.net>)를 참조하세요.

> 1. 일반적으로 `vi`입니다. `vi`를 exit하려면 `:q` `Enter`를 입력하세요.[ ↩](#reffn_1)
