### Subdirectories (하위 디렉터리)

**hierarchical name space(계층적 이름 공간)를 구현하세요.** basic file system에서는 모든 file(파일)이 하나의 directory(디렉터리)에 있습니다. directory entry(디렉터리 항목)가 file 또는 다른 directory를 가리킬 수 있도록 이를 수정하세요.

directory도 다른 file과 마찬가지로 original size(원래 크기)를 넘어 expand(확장)할 수 있어야 합니다.

basic file system에는 file name(파일 이름)에 14-character(문자) 제한이 있습니다. 개별 file name component(경로 구성요소)에 대해 이 제한을 유지해도 되고, 선택에 따라 확장해도 됩니다. 하지만 full path name(전체 경로 이름)은 14 character보다 훨씬 길 수 있도록 허용해야 합니다.

**각 process(프로세스)마다 separate current directory(별도의 현재 디렉터리)를 유지하세요.** startup(시작) 시 initial process(초기 프로세스)의 current directory를 root(루트)로 설정합니다. 한 process가 fork system call(시스템 콜)로 다른 process를 시작하면 child process(자식 프로세스)는 parent(부모)의 current directory를 inherit(상속)합니다. 그 이후 두 process의 current directory는 독립적이므로, 어느 한쪽이 자신의 current directory를 변경해도 다른 쪽에는 영향이 없습니다. 이것이 Unix에서 cd command(명령)가 external program(외부 프로그램)이 아니라 shell built-in(셸 내장 명령)인 이유입니다.

caller(호출자)가 file name을 제공하는 모든 기존 system call을 update(갱신)하여 absolute path name(절대 경로) 또는 relative path name(상대 경로)을 사용할 수 있게 하세요. directory separator character(디렉터리 구분 문자)는 forward slash(`/`)입니다. Unix와 같은 의미를 갖는 special file name(특수 파일 이름) `.`과 `..`도 support(지원)해야 합니다.

`open` system call을 update하여 directory도 open(열기)할 수 있게 하세요. 기존 system call 중에서는 close만 directory에 대한 file descriptor(파일 디스크립터)를 accept(허용)하면 됩니다.

`remove` system call을 update하여 regular file(일반 파일)뿐 아니라 empty directory(빈 디렉터리, root 제외)도 delete(삭제)할 수 있게 하세요. directory는 `.`과 `..` 외의 file이나 subdirectory를 포함하지 않을 때에만 delete될 수 있습니다. process가 open한 directory 또는 process의 current working directory(현재 작업 디렉터리)로 사용 중인 directory의 deletion을 허용할지 여부는 직접 결정할 수 있습니다. 허용한다면 삭제된 directory에서 file(`.`과 `..` 포함)을 open하거나 새 file을 create(생성)하려는 시도는 disallow(금지)해야 합니다. 다음 새 system call들을 구현하세요.

* * *
    
    
    bool chdir (const char *dir);
    

> process의 current working directory를 dir로 변경합니다. dir은 relative 또는 absolute일 수 있습니다. 성공하면 true, 실패하면 false를 반환합니다.

* * *
    
    
    bool mkdir (const char *dir);
    

> dir이라는 directory를 create합니다. dir은 relative 또는 absolute일 수 있습니다. 성공하면 true, 실패하면 false를 반환합니다. dir이 이미 존재하거나, dir 안에서 마지막 이름을 제외한 어떤 directory name(디렉터리 이름)이 아직 존재하지 않으면 fail합니다. 즉 `mkdir("/a/b/c")`는 `/a/b`가 이미 존재하고 `/a/b/c`가 존재하지 않을 때에만 성공합니다.

* * *
    
    
    bool readdir (int fd, char *name);
    

> file descriptor fd에서 directory entry를 read합니다. fd는 directory를 나타내야 합니다. 성공하면 null-terminated file name(널 종료 파일 이름)을 name에 저장하고 true를 반환합니다. name에는 `READDIR_MAX_LEN + 1` bytes(바이트)를 위한 room(공간)이 있어야 합니다. directory에 남은 entry가 없으면 false를 반환합니다.
> 
> `readdir`은 .과 ..를 반환해서는 안 됩니다. directory가 open되어 있는 동안 변경된다면 일부 entry가 전혀 읽히지 않거나 여러 번 읽히는 것은 허용됩니다. 그렇지 않다면 각 directory entry는 어떤 순서든 한 번씩 읽혀야 합니다.
> 
> `READDIR_MAX_LEN`은 `lib/user/syscall.h`에 정의되어 있습니다. file system이 basic file system보다 긴 file name을 support한다면 이 값을 기본값 14에서 늘려야 합니다.

* * *
    
    
    bool isdir (int fd);
    

> fd가 directory를 나타내면 true를, ordinary file(일반 파일)을 나타내면 false를 반환합니다.

* * *
    
    
    int inumber (int fd);
    

> fd와 associated(연관)된 inode의 inode number(아이노드 번호)를 반환합니다. fd는 ordinary file 또는 directory를 나타낼 수 있습니다.
> 
> inode number는 file 또는 directory를 persistently(지속적으로) identify(식별)합니다. file이 존재하는 동안 unique(고유)합니다. Pintos에서는 inode의 sector number를 inode number로 사용하기에 적합합니다.

### Soft Link (소프트 링크)

**Pintos에서 soft link mechanism(소프트 링크 메커니즘)을 구현하세요.** Soft link, 또는 symbolic link(심볼릭 링크)는 다른 file이나 directory를 refer(참조)하는 pseudo file object(의사 파일 객체)입니다. 이 file은 가리키는 file의 path(경로) 정보를 absolute path 또는 relative path 형태로 포함합니다. 다음 상황을 가정해 봅시다.
    
    
    /
    ├── a
    │   ├── link1 -> /file
    │   │
    │   └── link2 -> ../file
    └── file
    

`/a` 안의 `link1`이라는 soft-link는 `/file`을 absolute path로 가리키고, `/a` 안의 `link2`는 `../file`을 relative path로 가리킵니다. `link1`이나 `link2` file을 read하는 것은 `/file`을 read하는 것과 equivalent(동등)합니다.

* * *
    
    
    int symlink (const char *target, const char *linkpath);
    

> string target(문자열 target)을 포함하는 linkpath라는 symbolic link를 create합니다. 성공하면 zero(0)를 반환합니다. 그렇지 않으면 -1을 반환합니다.
