## Indexed and Extensible Files (인덱싱된 확장 가능 파일)

basic file system(기본 파일 시스템)은 file(파일)을 single extent(단일 연속 영역)로 allocate(할당)하므로 external fragmentation(외부 단편화)에 취약합니다. 즉 *n* block file을 allocate해야 할 때 *n*개의 block이 free(비어 있음)여도 allocate할 수 없는 경우가 있을 수 있습니다. on-disk inode structure(디스크상의 inode 구조)를 수정하여 이 문제를 제거하세요.

실제로는 direct, indirect, doubly indirect block(직접/간접/이중 간접 블록)을 갖는 index structure(인덱스 구조)를 사용하는 것을 의미할 가능성이 큽니다. 이전 학기에는 대부분의 학생이 multi-level indexing(다단계 인덱싱)을 사용하는 *Berkeley UNIX FFS*와 비슷한 것을 채택했습니다. 하지만 더 쉽게 진행할 수 있도록 여기서는 더 쉬운 방식인 FAT를 구현하게 합니다. **제공된 skeleton code(골격 코드)로 FAT를 반드시 구현해야 합니다.** 여러분의 code에는 multi-level indexing(강의의 FFS)에 대한 code가 포함되어서는 안 됩니다. 그러면 file growth(파일 증가) 부분에서 0점을 받습니다.

NOTE: file system partition(파티션)은 8 MB보다 크지 않다고 가정할 수 있습니다. partition 크기( metadata(메타데이터) 제외)만큼 큰 file을 support(지원)해야 합니다. 각 inode는 하나의 disk sector(디스크 섹터)에 저장되므로 포함할 수 있는 block pointer(블록 포인터)의 수가 제한됩니다.

### Indexing large files with FAT (File Allocation Table) (FAT로 큰 파일 인덱싱)

> Warning(주의): 이 document(문서)는 강의에서 general filesystem(일반 파일 시스템)과 FAT의 기본 원리를 이미 이해했다고 가정합니다. 그렇지 않다면 lecture note(강의 노트)로 돌아가 filesystem과 FAT가 무엇인지 이해하세요.

이전 project에서 사용한 basic filesystem에서는 file이 여러 disk sector에 걸친 contiguous single chunk(연속 단일 덩어리)로 저장되었습니다. cluster(클러스터, 묶음)는 하나 이상의 연속 disk sector를 포함할 수 있으므로 이 *contiguous chunk*를 *cluster*라고 부릅시다. 이 관점에서 basic file system의 cluster size(클러스터 크기)는 그 cluster에 저장된 file의 크기와 같았습니다.

external fragmentation을 완화하기 위해 cluster size를 줄일 수 있습니다(virtual memory(가상 메모리)의 page size를 떠올리세요). 단순화를 위해 skeleton code에서는 cluster 안의 sector 수를 `1`로 고정했습니다. 이렇게 작은 cluster를 사용하면 하나의 cluster가 전체 file을 저장하기에 충분하지 않을 수 있습니다. 이 경우 file 하나에 여러 cluster가 필요하므로, inode 안에서 file의 cluster를 index할 data structure(자료구조)가 필요합니다. 가장 쉬운 방법 중 하나는 linked-list(연결 리스트), 즉 *chain*(체인)을 사용하는 것입니다. inode가 file의 첫 block sector number(섹터 번호)를 담고, 첫 block이 두 번째 block의 sector number를 담을 수 있습니다. 하지만 이 naive approach(순진한 접근)는 너무 느립니다. 실제로 필요한 것이 마지막 block뿐이어도 file의 모든 block을 읽어야 하기 때문입니다. 이를 극복하기 위해 FAT(File Allocation Table)는 block의 connectivity(연결 관계)를 block 자체가 아니라 fixed-size File Allocation Table(고정 크기 파일 할당 테이블)에 둡니다. FAT는 실제 data가 아니라 connectivity value만 담으므로 DRAM(주 메모리)에 cache(캐시)될 만큼 작습니다. 결과적으로 table에서 해당 entry(항목)만 읽으면 됩니다.

`filesys/fat.c`에 제공된 skeleton code로 inode indexing(아이노드 인덱싱)을 구현합니다. 이 section은 `fat.c`에 이미 구현된 function이 무엇을 하는지와 여러분이 무엇을 구현해야 하는지 간단히 설명합니다.

우선 `fat.c`의 6개 function, 즉 `fat_init()`, `fat_open()`, `fat_close()`, `fat_create()`, `fat_boot_create()`는 boot time(부팅 시점)에 disk를 initialize(초기화)하고 format(포맷)하므로 수정할 필요가 없습니다. 하지만 `fat_fs_init()`은 작성해야 하며, 이들이 무엇을 하는지 대략 이해하면 도움이 될 수 있습니다.

* * *
    
    
    cluster_t fat_fs_init (void);
    

> FAT file system을 initialize합니다. `fat_fs`의 `fat_length`와 `data_start` field(필드)를 initialize해야 합니다. `fat_length`는 filesystem 안의 cluster 수를 저장하고, `data_start`는 file을 저장하기 시작할 수 있는 sector를 저장합니다. `fat_fs->bs`에 저장된 일부 value(값)를 활용하고 싶을 수 있습니다. 이 function에서 다른 유용한 data를 initialize해도 됩니다.

* * *
    
    
    cluster_t fat_create_chain (cluster_t clst);
    

> `clst`(cluster indexing number, 클러스터 인덱스 번호)에 지정된 cluster 뒤에 cluster를 append(덧붙임)하여 chain을 extend(확장)합니다. `clst`가 0과 같으면 새 chain을 create(생성)합니다. 새로 allocate된 cluster number를 return(반환)합니다.

* * *
    
    
    void fat_remove_chain (cluster_t clst, cluster_t pclst);
    

> `clst`에서 시작하여 chain에서 cluster들을 remove(제거)합니다. `pclst`는 chain에서 바로 이전 cluster여야 합니다. 즉 이 function이 실행된 뒤에는 `pclst`가 update된 chain의 마지막 element(원소)가 되어야 합니다. `clst`가 chain의 첫 element라면 `pclst`는 0이어야 합니다.

* * *
    
    
    void fat_put (cluster_t clst, cluster_t val);
    

> cluster number `clst`가 가리키는 FAT entry를 `val`로 update(갱신)합니다. FAT의 각 entry는 chain의 다음 cluster를 가리키므로(있으면, 없으면 `EOChain`) connectivity를 update하는 데 사용할 수 있습니다.

* * *
    
    
    cluster_t fat_get (cluster_t clst);
    

> 주어진 cluster `clst`가 어떤 cluster number를 가리키는지 return합니다.

* * *
    
    
    disk_sector_t cluster_to_sector (cluster_t clst);
    

> cluster number `clst`를 대응하는 sector number로 translate하고 그 sector number를 return합니다.

* * *

basic file system을 augment(강화)하기 위해 `filesys.c`와 `inode.c`에서 이 function들을 활용하고 싶을 수 있습니다.

### File Growth (파일 증가)

**extensible file(확장 가능 파일)을 구현하세요.** basic file system에서는 file이 create(생성)될 때 file size(파일 크기)가 지정됩니다. 하지만 대부분의 modern file system(현대 파일 시스템)에서는 file이 처음에 크기 0으로 생성되고, file 끝을 넘어 write(쓰기)할 때마다 expand(확장)됩니다. 여러분의 file system은 이를 허용해야 합니다.

file size에 predetermined limit(미리 정해진 제한)이 있어서는 안 됩니다. 단 file은 file system 크기(metadata 제외)를 넘을 수 없습니다. 이는 root directory file(루트 디렉터리 파일)에도 적용되어야 하며, 이제 초기 제한인 file 16개를 넘어 expand할 수 있어야 합니다.

user program은 현재 end-of-file(EOF, 파일 끝)을 넘어 seek(위치 이동)할 수 있습니다. seek 자체는 file을 extend하지 않습니다. EOF를 지난 position(위치)에 write하면 file은 write되는 position까지 extend되고, 이전 EOF와 write 시작 위치 사이의 gap(빈 구간)은 zero(0)로 채워져야 합니다. EOF를 지난 position에서 시작하는 read(읽기)는 byte를 반환하지 않습니다.

EOF보다 훨씬 뒤에 write하면 완전히 zero인 block이 많이 생길 수 있습니다. 어떤 file system은 이런 implicitly zeroed block(암묵적으로 0인 블록)에 실제 data block을 allocate하고 write합니다. 다른 file system은 명시적으로 write되기 전까지 이 block들을 전혀 allocate하지 않습니다. 후자의 file system은 "sparse files(희소 파일)"을 지원한다고 말합니다. 여러분의 file system에서는 어느 allocation strategy(할당 전략)든 채택할 수 있습니다.
