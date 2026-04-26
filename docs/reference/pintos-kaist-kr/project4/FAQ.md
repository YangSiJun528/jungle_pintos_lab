# FAQ

### Can BLOCK_SECTOR_SIZE change? (BLOCK_SECTOR_SIZE가 바뀔 수 있나요?)

아니요. BLOCK_SECTOR_SIZE는 512로 고정되어 있습니다. IDE disk(디스크)의 경우 이 값은 hardware(하드웨어)의 고정 property(속성)입니다. 다른 disk가 반드시 512-byte sector(섹터)를 가지는 것은 아니지만, 단순화를 위해 Pintos는 그런 disk만 support(지원)합니다.

### Indexed Files FAQ (인덱스 파일 FAQ)

#### What is the largest file size that we are supposed to support? (지원해야 하는 최대 file size는 얼마인가요?)

생성하는 file system partition(파일 시스템 파티션)은 8 MB 이하입니다. 하지만 individual file(개별 파일)은 metadata(메타데이터)를 수용하기 위해 partition보다 작아야 합니다. inode organization(아이노드 구성)을 결정할 때 이를 고려해야 합니다.

### Subdirectories FAQ (하위 디렉터리 FAQ)

#### How should a file name like `a//b` be interpreted? (`a//b` 같은 file name은 어떻게 해석해야 하나요?)

여러 개의 consecutive slash(연속 슬래시)는 하나의 slash와 equivalent(동등)하므로, 이 file name은 `a/b`와 같습니다.

#### How about a file name like `/../x`? (`/../x` 같은 file name은 어떤가요?)

root directory(루트 디렉터리)는 자기 자신의 parent(부모)이므로 `/x`와 equivalent합니다.

#### How should a file name that ends in `/` be treated? (`/`로 끝나는 file name은 어떻게 처리해야 하나요?)

대부분의 Unix system(유닉스 시스템)은 directory 이름 끝의 slash를 허용하고, slash로 끝나는 다른 이름은 reject(거부)합니다. 이 behavior(동작)를 허용해도 되고, slash로 끝나는 이름을 단순히 reject해도 됩니다.

### Buffer Cache FAQ (버퍼 캐시 FAQ)

#### Can we keep a `struct inode_disk` inside `struct inode`? (`struct inode` 안에 `struct inode_disk`를 유지해도 되나요?)

64-block limit(64블록 제한)의 goal(목표)은 cached file system data(캐시된 파일 시스템 데이터)의 양을 bound(제한)하는 것입니다. file data든 metadata든 disk data block(디스크 데이터 블록)을 kernel memory(커널 메모리) 어딘가에 유지한다면 64-block limit에 포함해야 합니다. `length`나 `sector_cnt` member(멤버)가 없는 `struct inode_disk`처럼 disk data block과 "similar(유사)"한 것에도 같은 rule(규칙)이 적용됩니다.

이는 현재 corresponding on-disk inode(대응되는 디스크상 inode)에 접근하는 inode implementation(아이노드 구현) 방식을 바꿔야 함을 의미합니다. 현재는 `struct inode_disk`를 `struct inode` 안에 embed(내장)하고, 생성될 때 disk에서 corresponding sector(대응 섹터)를 읽기 때문입니다. inode의 extra copy(추가 복사본)를 유지하면 cache에 부과한 64-block limitation을 subvert(우회)하게 됩니다.

`struct inode` 안에 inode data에 대한 pointer(포인터)를 저장할 수는 있지만, 그렇게 한다면 이것이 OS를 동시에 open 가능한 file 64개로 limit하지 않도록 조심스럽게 확인해야 합니다. 필요할 때 inode를 찾는 데 도움이 되는 다른 information(정보)을 저장할 수도 있습니다. 마찬가지로 64 cache entry(캐시 항목) 각각에 metadata 일부를 저장할 수 있습니다.

원한다면 free map(빈 공간 지도)의 cached copy(캐시된 복사본)를 memory에 영구적으로 유지할 수 있습니다. 이는 cache size(캐시 크기)에 포함될 필요가 없습니다.

`filesys/inode.c`의 `byte_to_sector()`는 해당 sector가 storage hierarchy(저장 계층)의 어디에 있든 먼저 읽어 오지 않고 `struct inode_disk`를 직접 사용합니다. 이제 이는 동작하지 않습니다. `inode_byte_to_sector()`가 `struct inode_disk`를 사용하기 전에 cache에서 얻도록 변경해야 합니다.
