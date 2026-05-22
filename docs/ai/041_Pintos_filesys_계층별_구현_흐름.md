# Pintos filesys 계층별 구현 흐름

작성일: 2026-05-20

이 문서는 Pintos Project 4 filesys 구현을 계층별로 설명한다. 목표는 `disk.h`/`disk.c`가 제공하는 sector I/O 위에 `fat.c`가 어떤 구조를 만들고, 그 FAT 기반 저장 방식을 `inode.c`가 파일 배치로 바꾸며, 그 위의 `file.c`와 `syscall.c`가 어떤 식으로 사용자가 보는 파일 기능을 제공하는지 이해하는 것이다.

핵심 흐름은 다음이다.

```text
사용자 프로그램
  |
  | open, read, write, seek, close, readdir ...
  v
userprog/syscall.c
  |
  | fd 조회, user buffer 검증, filesys_lock
  v
filesys/file.c
  |
  | 열린 파일 객체 관리, pos 관리, read/write 위임
  v
filesys/inode.c
  |
  | file offset -> disk sector 변환
  | file growth, inode metadata 관리
  v
filesys/fat.c
  |
  | cluster chain 할당, 연결, 해제
  v
devices/disk.c / include/devices/disk.h
  |
  | sector 단위 disk_read(), disk_write()
  v
ATA/IDE disk
```

주의할 점은 `file.c`가 `fat.c`를 직접 호출하지 않는다는 것이다. `file.c`는 `inode_read_at()`, `inode_write_at()` 같은 inode API만 사용한다. FAT을 직접 다루는 계층은 `inode.c`다. 따라서 실제 의존 관계는 다음처럼 이해해야 한다.

```text
syscall.c -> file.c -> inode.c -> fat.c -> disk.c
```

## 1. Disk 계층: sector 단위 장치 인터페이스

가장 아래 계층은 `include/devices/disk.h`와 `devices/disk.c`다.

`disk.h`는 filesys가 사용할 수 있는 디스크 인터페이스를 제공한다.

```c
#define DISK_SECTOR_SIZE 512

struct disk *disk_get (int chan_no, int dev_no);
disk_sector_t disk_size (struct disk *);
void disk_read (struct disk *, disk_sector_t, void *);
void disk_write (struct disk *, disk_sector_t, const void *);
```

실제 `disk_read()` 구현은 sector 번호를 ATA controller에 설정하고, read command를 발행한 뒤 interrupt를 기다렸다가 data port에서 한 sector를 읽는다.

출처: `pintos/devices/disk.c`

```c
void
disk_read (struct disk *d, disk_sector_t sec_no, void *buffer) {
  struct channel *c;

  ASSERT (d != NULL);
  ASSERT (buffer != NULL);

  c = d->channel;
  lock_acquire (&c->lock);
  select_sector (d, sec_no);
  issue_pio_command (c, CMD_READ_SECTOR_RETRY);
  sema_down (&c->completion_wait);
  if (!wait_while_busy (d))
    PANIC ("%s: disk read failed, sector=%"PRDSNu, d->name, sec_no);
  input_sector (c, buffer);
  d->read_cnt++;
  lock_release (&c->lock);
}
```

`disk_write()`도 같은 구조로 동작한다. 차이는 write command를 발행한 뒤 data port로 sector 내용을 내보낸다는 점이다.

```c
void
disk_write (struct disk *d, disk_sector_t sec_no, const void *buffer) {
  struct channel *c;

  ASSERT (d != NULL);
  ASSERT (buffer != NULL);

  c = d->channel;
  lock_acquire (&c->lock);
  select_sector (d, sec_no);
  issue_pio_command (c, CMD_WRITE_SECTOR_RETRY);
  if (!wait_while_busy (d))
    PANIC ("%s: disk write failed, sector=%"PRDSNu, d->name, sec_no);
  output_sector (c, buffer);
  sema_down (&c->completion_wait);
  d->write_cnt++;
  lock_release (&c->lock);
}
```

이 계층에서 중요한 단위는 sector다. Pintos의 디스크 read/write는 "sector 번호 하나"와 "512바이트 buffer 하나"를 주고받는다.

```text
disk_read(disk, 10, buffer)
  -> disk의 10번 sector 512바이트를 buffer에 읽는다.

disk_write(disk, 10, buffer)
  -> buffer의 512바이트를 disk의 10번 sector에 쓴다.
```

`devices/disk.c`는 간단한 ATA/IDE 디스크 드라이버 역할을 한다. 내부적으로는 다음 일을 한다.

- legacy ATA channel을 초기화한다.
- `disk_get(0, 1)` 같은 방식으로 특정 디스크를 가져오게 한다.
- `READ SECTOR`, `WRITE SECTOR` 명령을 ATA register에 쓴다.
- disk interrupt를 기다린다.
- data port를 통해 512바이트를 전송한다.
- channel lock으로 같은 controller에 대한 동시 접근을 직렬화한다.

Pintos convention에서 디스크 번호는 다음처럼 쓰인다.

```text
0:0 - boot loader, command line args, kernel
0:1 - file system
1:0 - scratch
1:1 - swap
```

`filesys_init()`은 파일 시스템용 디스크를 다음처럼 가져온다.

```c
filesys_disk = disk_get (0, 1);
```

따라서 filesys 위쪽 계층은 "디스크 드라이버를 직접 구현"하지 않는다. 위쪽 계층은 sector 번호를 계산하고 `disk_read()`/`disk_write()`를 호출한다.

## 2. FAT 계층: sector 위에 cluster chain 만들기

`fat.c`는 raw sector I/O 위에 FAT(File Allocation Table) 기반 cluster chain을 만든다.

기본 아이디어는 간단하다.

```text
파일 데이터가 cluster 5, 9, 12에 있다면:

FAT[5]  = 9
FAT[9]  = 12
FAT[12] = EOChain
```

inode는 첫 cluster만 알고 있으면 된다. 나머지는 FAT entry를 따라간다.

```text
inode.start = 5
5 -> 9 -> 12 -> EOChain
```

KAIST Pintos skeleton에서는 `SECTORS_PER_CLUSTER == 1`이므로 cluster 하나가 disk sector 하나와 대응된다. 그래도 코드에서는 cluster와 sector를 구분한다. FAT은 cluster 번호를 다루고, disk는 sector 번호를 다룬다.

### 2.1 `fat_fs_init()`

`fat_fs_init()`은 FAT 파일 시스템의 배치를 계산한다.

핵심 필드는 다음이다.

```text
fat_start  = FAT table이 시작되는 sector
fat_sectors = FAT table이 차지하는 sector 수
data_start = 실제 파일 data cluster가 시작되는 sector
last_clst  = 마지막 cluster 번호
fat_length = FAT entry 개수
```

계산 흐름은 다음처럼 볼 수 있다.

```text
boot sector
  |
  v
FAT 영역
  |
  v
data 영역
```

`data_start`는 FAT 영역 바로 뒤 sector다.

```c
fat_fs->data_start = fat_fs->bs.fat_start + fat_fs->bs.fat_sectors;
```

실제 구현은 `data_start`, 마지막 cluster 번호, FAT entry 수를 계산하고 FAT 수정용 lock을 초기화한다.

출처: `pintos/filesys/fat.c`

```c
void
fat_fs_init (void) {
  fat_fs->data_start = fat_fs->bs.fat_start + fat_fs->bs.fat_sectors;
  fat_fs->last_clst =
      (fat_fs->bs.total_sectors - fat_fs->data_start)
      / fat_fs->bs.sectors_per_cluster;
  fat_fs->fat_length = fat_fs->last_clst + 1;
  lock_init (&fat_fs->write_lock);
}
```

즉 `fat.c`는 전체 디스크를 "FAT metadata 영역"과 "실제 파일 data 영역"으로 나누어 이해한다.

### 2.2 `fat_create_chain(clst)`

`fat_create_chain()`은 새 cluster를 하나 할당한다.

동작은 두 경우로 나뉜다.

```text
fat_create_chain(0)
  -> 새 chain을 시작한다.

fat_create_chain(existing_last)
  -> existing_last 뒤에 새 cluster를 붙인다.
```

예를 들어 비어 있는 cluster 5를 찾으면:

```text
fat_create_chain(0)

FAT[5] = EOChain
return 5
```

기존 chain 끝이 5이고 새 cluster 9를 붙이면:

```text
fat_create_chain(5)

FAT[5] = 9
FAT[9] = EOChain
return 9
```

이 함수는 FAT table을 수정하므로 `fat_fs->write_lock`으로 보호된다.

출처: `pintos/filesys/fat.c`

```c
cluster_t
fat_create_chain (cluster_t clst) {
  cluster_t new_clst = 0;

  lock_acquire (&fat_fs->write_lock);
  for (cluster_t i = 1; i <= fat_fs->last_clst; i++) {
    if (fat_fs->fat[i] == 0) {
      new_clst = i;
      break;
    }
  }

  if (new_clst != 0) {
    fat_fs->fat[new_clst] = EOChain;
    if (clst != 0)
      fat_fs->fat[clst] = new_clst;
  }
  lock_release (&fat_fs->write_lock);

  return new_clst;
}
```

이 코드에서 `fat_fs->fat[i] == 0`은 free cluster를 뜻한다. 새 cluster는 `EOChain`으로 표시되고, 기존 cluster `clst`가 있으면 `FAT[clst] = new_clst`로 chain 뒤에 붙는다.

### 2.3 `fat_remove_chain(clst, pclst)`

`fat_remove_chain()`은 cluster chain을 free 상태로 되돌린다.

예를 들어:

```text
5 -> 9 -> 12 -> EOChain
```

여기서 `fat_remove_chain(9, 5)`를 호출하면 9부터 뒤 chain을 제거하고, 이전 cluster인 5를 새 끝으로 만든다.

```text
FAT[5] = EOChain
FAT[9] = 0
FAT[12] = 0
```

`fat_remove_chain(5, 0)`처럼 첫 cluster부터 제거하면 전체 chain을 반환한다.

출처: `pintos/filesys/fat.c`

```c
void
fat_remove_chain (cluster_t clst, cluster_t pclst) {
  if (clst == 0)
    return;

  lock_acquire (&fat_fs->write_lock);
  if (pclst != 0)
    fat_fs->fat[pclst] = EOChain;

  while (clst != 0 && clst <= fat_fs->last_clst) {
    cluster_t next = fat_fs->fat[clst];
    fat_fs->fat[clst] = 0;
    if (next == EOChain)
      break;
    clst = next;
  }
  lock_release (&fat_fs->write_lock);
}
```

이 구현은 제거 시작점부터 chain을 따라가며 FAT entry를 0으로 되돌린다. `pclst`가 있으면 제거 구간 앞 cluster를 새 chain 끝으로 만든다.

### 2.4 `fat_get()`, `cluster_to_sector()`

`fat_get(clst)`는 "이 cluster 다음 cluster가 무엇인지"를 읽는다.

```text
fat_get(5) -> 9
fat_get(9) -> 12
fat_get(12) -> EOChain
```

`cluster_to_sector(clst)`는 cluster 번호를 실제 disk sector 번호로 바꾼다.

```c
return fat_fs->data_start
       + (clst - 1) * fat_fs->bs.sectors_per_cluster;
```

출처: `pintos/filesys/fat.c`

```c
cluster_t
fat_get (cluster_t clst) {
  if (clst > fat_fs->last_clst)
    return 0;
  return fat_fs->fat[clst];
}

disk_sector_t
cluster_to_sector (cluster_t clst) {
  ASSERT (clst >= 1 && clst <= fat_fs->last_clst);
  return fat_fs->data_start
         + (clst - 1) * fat_fs->bs.sectors_per_cluster;
}
```

이 함수가 있기 때문에 위쪽 계층은 cluster 기반으로 파일 배치를 생각하다가, 실제 disk I/O 직전에 sector 번호로 변환할 수 있다.

## 3. Inode 계층: FAT을 파일 배치로 바꾸기

`inode.c`는 FAT을 직접 사용하는 핵심 계층이다. `file.c`가 요청한 "파일 offset N에서 읽기/쓰기"를 "실제 disk sector S에서 읽기/쓰기"로 바꾼다.

현재 on-disk inode는 FAT 방식에 맞게 다음 정보를 가진다.

```c
struct inode_disk {
  cluster_t start;   /* 첫 data cluster. */
  off_t length;      /* 파일 길이. */
  unsigned magic;
  uint32_t type;     /* 일반 파일 / 디렉터리 / symlink. */
  uint32_t unused[124];
};
```

출처: `pintos/filesys/inode.c`

```c
struct inode_disk {
  cluster_t start;                    /* First data cluster. */
  off_t length;                       /* File size in bytes. */
  unsigned magic;                     /* Magic number. */
  uint32_t type;                      /* enum inode_type. */
  uint32_t unused[124];               /* Not used. */
};
```

이 구조에서 중요한 것은 `start`다. 파일의 전체 cluster 목록을 inode 안에 다 넣지 않고, 첫 cluster만 저장한다. 나머지는 FAT table을 따라간다.

```text
inode.start = 5
inode.length = 1500

FAT[5] = 9
FAT[9] = 12
FAT[12] = EOChain
```

### 3.1 `byte_to_sector()`

`byte_to_sector()`는 파일 안의 byte offset을 실제 disk sector로 바꾼다.

예를 들어 sector 크기가 512바이트라면:

```text
offset 0    -> 0번째 cluster
offset 511  -> 0번째 cluster
offset 512  -> 1번째 cluster
offset 1024 -> 2번째 cluster
```

FAT 기반 구현에서는 다음 순서로 동작한다.

```text
1. offset이 inode.length 안에 있는지 확인한다.
2. skip = offset / DISK_SECTOR_SIZE를 계산한다.
3. inode.start에서 시작한다.
4. FAT chain을 skip번 따라간다.
5. 찾은 cluster를 cluster_to_sector()로 sector 번호로 바꾼다.
```

예시:

```text
inode.start = 5
FAT[5] = 9
FAT[9] = 12

offset = 1024
skip = 1024 / 512 = 2

5에서 시작
1번 이동 -> 9
2번 이동 -> 12

cluster_to_sector(12)을 반환
```

즉 `byte_to_sector()`가 FAT chain과 disk sector 사이의 핵심 번역기다.

출처: `pintos/filesys/inode.c`

```c
static disk_sector_t
byte_to_sector (const struct inode *inode, off_t pos) {
  ASSERT (inode != NULL);
  if (pos >= inode->data.length)
    return -1;

#ifdef EFILESYS
  cluster_t clst = inode->data.start;
  size_t skip = pos / DISK_SECTOR_SIZE;

  if (clst == 0)
    return -1;

  while (skip-- > 0) {
    clst = fat_get (clst);
    if (clst == 0 || clst == EOChain)
      return -1;
  }
  return cluster_to_sector (clst);
#else
  return inode->data.start + pos / DISK_SECTOR_SIZE;
#endif
}
```

실제 코드에서도 `fat_get()`으로 chain을 따라가고, 마지막에 `cluster_to_sector()`로 disk sector 번호를 만든다.

### 3.2 `inode_create_typed()`

`inode_create_typed()`는 일반 파일, 디렉터리, symlink inode를 공통으로 만든다.

초기 크기가 0이면 data cluster를 할당하지 않고 inode metadata만 쓴다.

```text
length = 0
start = 0
type = INODE_FILE / INODE_DIR / INODE_SYMLINK
```

초기 크기가 있으면 필요한 sector 수만큼 cluster를 할당한다.

```text
sectors = bytes_to_sectors(length)

for each needed sector:
  new = fat_create_chain(last)
  inode.start가 비어 있으면 new를 start로 설정
  zero_cluster(new)
```

이렇게 생성된 파일은 처음부터 FAT chain을 가진다.

출처: `pintos/filesys/inode.c`

```c
bool
inode_create_typed (disk_sector_t sector, off_t length, enum inode_type type) {
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);
  ASSERT (sizeof *disk_inode == DISK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL) {
    size_t sectors = bytes_to_sectors (length);
    disk_inode->length = length;
    disk_inode->magic = INODE_MAGIC;
    disk_inode->type = type;
#ifdef EFILESYS
    if (sectors == 0) {
      disk_write (filesys_disk, sector, disk_inode);
      success = true;
    } else {
      cluster_t last_clst = 0;
      bool ok = true;

      for (size_t i = 0; i < sectors; i++) {
        cluster_t new_clst = fat_create_chain (last_clst);
        if (new_clst == 0) {
          ok = false;
          break;
        }
        if (disk_inode->start == 0)
          disk_inode->start = new_clst;
        last_clst = new_clst;
        zero_cluster (new_clst);
      }

      if (ok) {
        disk_write (filesys_disk, sector, disk_inode);
        success = true;
      } else if (disk_inode->start != 0) {
        fat_remove_chain (disk_inode->start, 0);
      }
    }
#endif
    free (disk_inode);
  }
  return success;
}
```

여기서 `fat_create_chain(last_clst)`가 새 파일의 data chain을 만들고, 실패하면 `fat_remove_chain()`으로 이미 붙인 chain을 되돌린다.

### 3.3 `inode_extend()`

`inode_extend()`는 Project 4의 file growth 요구사항을 구현하는 핵심 함수다.

`inode_write_at()`에서 write 범위가 기존 EOF를 넘으면 호출된다.

```c
if (size > 0 && offset + size > inode->data.length) {
  if (!inode_extend (inode, offset + size))
    return 0;
}
```

`inode_extend()` 흐름은 다음과 같다.

```text
1. 기존 length에 필요한 sector 수를 계산한다.
2. 새 length에 필요한 sector 수를 계산한다.
3. 부족한 sector 수만큼 FAT chain 뒤에 cluster를 붙인다.
4. 새 cluster는 0으로 초기화한다.
5. inode.length를 새 length로 갱신한다.
6. inode metadata를 disk에 쓴다.
```

EOF보다 뒤에 write해서 중간 gap이 생기는 경우에도 새 cluster를 실제로 할당하고 0으로 초기화한다. 즉 현재 구현은 sparse file 방식이 아니라 실제 zero block을 할당하는 방식이다.

출처: `pintos/filesys/inode.c`

```c
static bool
inode_extend (struct inode *inode, off_t length) {
  size_t old_sectors = bytes_to_sectors (inode->data.length);
  size_t new_sectors = bytes_to_sectors (length);
  cluster_t last_clst;
  cluster_t first_new = 0;
  cluster_t before_new;

  ASSERT (length >= inode->data.length);
  if (new_sectors == old_sectors) {
    inode->data.length = length;
    disk_write (filesys_disk, inode->sector, &inode->data);
    return true;
  }

  last_clst = inode_last_cluster (&inode->data);
  before_new = last_clst;
  for (size_t i = old_sectors; i < new_sectors; i++) {
    cluster_t new_clst = fat_create_chain (last_clst);
    if (new_clst == 0) {
      if (first_new != 0) {
        fat_remove_chain (first_new, before_new);
        if (before_new == 0)
          inode->data.start = 0;
      }
      return false;
    }
    if (inode->data.start == 0)
      inode->data.start = new_clst;
    if (first_new == 0)
      first_new = new_clst;
    last_clst = new_clst;
    zero_cluster (new_clst);
  }

  inode->data.length = length;
  disk_write (filesys_disk, inode->sector, &inode->data);
  return true;
}
```

이 코드는 파일 길이가 커질 때마다 부족한 cluster를 FAT chain 뒤에 붙인다. 새 cluster는 `zero_cluster()`로 0 초기화되므로 EOF와 새 write 위치 사이의 gap도 0으로 채워진다.

### 3.4 `inode_read_at()`과 `inode_write_at()`

`inode_read_at()`은 byte 단위 read 요청을 sector 단위 read로 바꾼다.

```text
while 읽을 byte가 남아 있음:
  sector_idx = byte_to_sector(inode, offset)
  sector 안 offset 계산
  전체 sector면 disk_read()로 바로 읽기
  일부 sector면 bounce buffer로 sector 전체를 읽고 필요한 부분만 복사
```

`inode_write_at()`도 비슷하지만, 쓰기 전에 파일 확장이 필요할 수 있다.

```text
if write가 EOF를 넘음:
  inode_extend()

while 쓸 byte가 남아 있음:
  sector_idx = byte_to_sector(inode, offset)
  전체 sector면 disk_write()로 바로 쓰기
  일부 sector면 기존 sector를 bounce buffer로 읽고 일부만 수정한 뒤 disk_write()
```

따라서 `inode.c`는 두 가지 책임을 동시에 가진다.

- FAT chain을 따라 파일 offset을 sector로 바꾼다.
- sector 단위 장치 위에 byte 단위 파일 read/write를 제공한다.

출처: `pintos/filesys/inode.c`

```c
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
    off_t offset) {
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
    return 0;

#ifdef EFILESYS
  if (size > 0 && offset + size > inode->data.length) {
    if (!inode_extend (inode, offset + size))
      return 0;
  }
#endif

  while (size > 0) {
    disk_sector_t sector_idx = byte_to_sector (inode, offset);
    int sector_ofs = offset % DISK_SECTOR_SIZE;
    off_t inode_left = inode_length (inode) - offset;
    int sector_left = DISK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) {
      disk_write (filesys_disk, sector_idx, buffer + bytes_written);
    } else {
      if (bounce == NULL) {
        bounce = malloc (DISK_SECTOR_SIZE);
        if (bounce == NULL)
          break;
      }
      if (sector_ofs > 0 || chunk_size < sector_left)
        disk_read (filesys_disk, sector_idx, bounce);
      else
        memset (bounce, 0, DISK_SECTOR_SIZE);
      memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
      disk_write (filesys_disk, sector_idx, bounce);
    }

    size -= chunk_size;
    offset += chunk_size;
    bytes_written += chunk_size;
  }
  free (bounce);

  return bytes_written;
}
```

이 구현에서 `inode_extend()`는 쓰기 전에 파일 크기를 보장하고, `byte_to_sector()`는 매 반복마다 현재 offset에 해당하는 sector를 찾는다.

### 3.5 `inode_close()`와 삭제된 파일의 cluster 반환

`remove()`가 호출되어도 inode와 data cluster를 즉시 지우지 않을 수 있다. 이미 열려 있는 fd가 있을 수 있기 때문이다.

현재 구현은 `inode_remove()`로 removed 표시를 하고, 마지막 opener가 `inode_close()`할 때 실제 cluster를 반환한다.

EFILESYS에서는 다음이 수행된다.

```text
if inode->removed:
  data chain이 있으면 fat_remove_chain(inode->data.start, 0)
  inode sector도 sector_to_cluster()로 cluster로 바꾸어 fat_remove_chain()
```

즉 삭제의 최종 정리는 FAT chain 반환으로 끝난다.

출처: `pintos/filesys/inode.c`

```c
void
inode_close (struct inode *inode) {
  if (inode == NULL)
    return;

  if (--inode->open_cnt == 0) {
    list_remove (&inode->elem);

    if (inode->removed) {
#ifdef EFILESYS
      if (inode->data.start != 0)
        fat_remove_chain (inode->data.start, 0);
      fat_remove_chain (sector_to_cluster (inode->sector), 0);
#else
      free_map_release (inode->sector, 1);
      free_map_release (inode->data.start,
          bytes_to_sectors (inode->data.length));
#endif
    }

    free (inode);
  }
}
```

마지막 opener가 닫힐 때만 실제 cluster를 반환하므로, 열린 fd가 남아 있는 삭제 파일의 데이터를 갑자기 잃지 않는다.

## 4. File 계층: 열린 파일 객체와 사용자 관점의 파일 동작

`file.h`와 `file.c`는 열린 파일 객체를 제공한다. 사용자 프로그램의 fd가 최종적으로 가리키는 kernel 객체가 `struct file`이다.

현재 `struct file`은 다음 상태를 가진다.

```c
struct file {
  struct inode *inode;
  struct dir *dir;
  off_t pos;
  bool deny_write;
};
```

출처: `pintos/filesys/file.c`

```c
struct file {
  struct inode *inode;        /* File's inode. */
  struct dir *dir;            /* Non-null if this file is a directory. */
  off_t pos;                  /* Current position. */
  bool deny_write;            /* Has file_deny_write() been called? */
};
```

각 필드의 의미는 다음과 같다.

| 필드 | 의미 |
| --- | --- |
| `inode` | 이 열린 파일이 가리키는 파일/디렉터리의 inode |
| `dir` | directory fd이면 non-null, 일반 파일이면 null |
| `pos` | 다음 read/write 위치 |
| `deny_write` | 이 file handle이 write deny를 걸었는지 여부 |

### 4.1 `file_open()`

`file_open(inode)`은 inode를 받아 새 `struct file`을 만든다.

Project 4에서는 디렉터리도 `open()`할 수 있어야 하므로 inode type을 확인한다.

```text
inode가 directory이면:
  dir_open(inode)로 struct dir 생성
  file->dir = 그 dir
  file->inode = dir_get_inode(file->dir)

일반 파일이면:
  file->inode = inode
  file->dir = NULL

공통:
  file->pos = 0
  file->deny_write = false
```

이 구조 덕분에 fd table은 일반 파일과 디렉터리를 모두 `struct file *` 하나로 다룰 수 있다.

출처: `pintos/filesys/file.c`

```c
struct file *
file_open (struct inode *inode) {
  struct file *file = calloc (1, sizeof *file);
  if (inode != NULL && file != NULL) {
    if (inode_is_dir (inode)) {
      file->dir = dir_open (inode);
      if (file->dir == NULL) {
        free (file);
        return NULL;
      }
      file->inode = dir_get_inode (file->dir);
    } else {
      file->inode = inode;
    }
    file->pos = 0;
    file->deny_write = false;
    return file;
  } else {
    inode_close (inode);
    free (file);
    return NULL;
  }
}
```

실제 코드에서 directory inode이면 `dir_open()`으로 감싸고, 일반 파일 inode이면 그대로 `file->inode`에 저장한다.

### 4.2 `file_read()`와 `file_write()`

`file_read()`는 열린 파일의 현재 위치에서 읽고, 읽은 만큼 `pos`를 전진시킨다.

```text
file_read(file, buffer, size)
  if file이 directory이면 실패
  bytes = inode_read_at(file->inode, buffer, size, file->pos)
  file->pos += bytes
  return bytes
```

`file_write()`도 같은 구조다.

```text
file_write(file, buffer, size)
  if file이 directory이면 실패
  bytes = inode_write_at(file->inode, buffer, size, file->pos)
  file->pos += bytes
  return bytes
```

여기서 FAT은 직접 나오지 않는다. 하지만 `inode_read_at()`과 `inode_write_at()` 내부에서 `byte_to_sector()`, `inode_extend()`, `fat_get()`, `fat_create_chain()`이 사용된다.

즉 read/write 전체 흐름은 다음과 같다.

```text
file_read()
  -> inode_read_at()
     -> byte_to_sector()
        -> fat_get()
        -> cluster_to_sector()
     -> disk_read()

file_write()
  -> inode_write_at()
     -> inode_extend() if EOF 넘어감
        -> fat_create_chain()
        -> cluster_to_sector()
        -> disk_write() zero fill
     -> byte_to_sector()
        -> fat_get()
        -> cluster_to_sector()
     -> disk_write()
```

출처: `pintos/filesys/file.c`

```c
off_t
file_read (struct file *file, void *buffer, off_t size) {
  if (file_is_dir (file))
    return -1;
  off_t bytes_read = inode_read_at (file->inode, buffer, size, file->pos);
  file->pos += bytes_read;
  return bytes_read;
}

off_t
file_write (struct file *file, const void *buffer, off_t size) {
  if (file_is_dir (file))
    return -1;
  off_t bytes_written = inode_write_at (file->inode, buffer, size, file->pos);
  file->pos += bytes_written;
  return bytes_written;
}
```

여기서 `file.c`는 현재 위치 `file->pos`만 관리한다. 실제 block 위치 계산과 FAT chain 탐색은 `inode_read_at()`/`inode_write_at()` 아래에서 일어난다.

### 4.3 `file_seek()`, `file_tell()`, `file_length()`

`file_seek()`는 `struct file`의 현재 위치만 바꾼다.

```text
file_seek(file, 1000)
  -> file->pos = 1000
```

seek 자체는 파일을 확장하지 않는다. Project 4 요구사항에 따라 EOF 뒤로 seek하는 것은 가능하고, 실제 확장은 그 위치에 `write()`할 때 `inode_write_at()`에서 일어난다.

`file_tell()`은 현재 `pos`를 반환한다.

`file_length()`는 file object의 상태가 아니라 underlying inode의 길이를 반환한다.

```text
file_length()
  -> inode_length(file->inode)
```

출처: `pintos/filesys/file.c`

```c
off_t
file_length (struct file *file) {
  ASSERT (file != NULL);
  return inode_length (file->inode);
}

void
file_seek (struct file *file, off_t new_pos) {
  ASSERT (file != NULL);
  ASSERT (new_pos >= 0);
  file->pos = new_pos;
}

off_t
file_tell (struct file *file) {
  ASSERT (file != NULL);
  return file->pos;
}
```

`file_seek()`는 `pos`만 바꾸므로 disk I/O도 FAT 수정도 하지 않는다. EOF 뒤 seek 후 실제 write가 들어올 때 `inode_write_at()`이 파일을 확장한다.

### 4.4 `file_reopen()`과 `file_duplicate()`

`file_reopen()`은 같은 inode에 대한 새 `struct file`을 만든다. 새 file object의 `pos`는 0부터 시작한다.

```text
file_reopen(file)
  -> inode_reopen(file_get_inode(file))
  -> file_open(...)
```

`file_duplicate()`도 같은 inode에 대한 새 `struct file`을 만들지만, 현재 `pos`와 `deny_write` 상태를 복사한다.

```text
file_duplicate(file)
  -> 새 file object 생성
  -> nfile->pos = file->pos
  -> file->deny_write가 true이면 nfile에도 deny_write 적용
```

현재 구현에서는 `fork()`에서 부모의 file descriptor와 executable handle을 자식에게 복제할 때 사용된다.

출처: `pintos/filesys/file.c`

```c
struct file *
file_reopen (struct file *file) {
  return file_open (inode_reopen (file_get_inode (file)));
}

struct file *
file_duplicate (struct file *file) {
  struct file *nfile = file_open (inode_reopen (file_get_inode (file)));
  if (nfile) {
    nfile->pos = file->pos;
    if (file->deny_write)
      file_deny_write (nfile);
  }
  return nfile;
}
```

둘 다 같은 inode에 대한 새 `struct file`을 만들지만, `file_duplicate()`는 현재 `pos`와 `deny_write` 상태를 복사한다.

### 4.5 directory fd 지원

Project 4에서는 directory도 fd로 열 수 있어야 한다.

이를 위해 `file.c`에 다음 기능이 추가되어 있다.

```text
file_is_dir(file)
  -> file->dir != NULL

file_readdir(file, name)
  -> file이 directory인지 확인
  -> dir_readdir(file->dir, name)
```

반대로 directory fd에 대해 일반 `read()`나 `write()`를 하면 실패한다.

```text
file_read(directory_fd)  -> -1
file_write(directory_fd) -> -1
```

이렇게 해서 디렉터리 내부 포맷은 `readdir()` 같은 전용 API로만 노출된다.

출처: `pintos/filesys/file.c`

```c
bool
file_is_dir (struct file *file) {
  return file != NULL && file->dir != NULL;
}

bool
file_readdir (struct file *file, char name[NAME_MAX + 1]) {
  if (!file_is_dir (file))
    return false;
  return dir_readdir (file->dir, name);
}
```

directory fd도 `struct file *`로 fd table에 들어가지만, 실제 directory entry 순회는 `dir_readdir()`에 위임된다.

## 5. Syscall 계층: 사용자 요청을 file API로 연결하기

`syscall.c`는 사용자 프로그램이 호출하는 syscall을 kernel 내부 file/filesys API로 연결한다.

이 계층의 주요 책임은 다음이다.

- syscall 번호와 인자를 해석한다.
- 사용자 pointer와 buffer가 유효한지 검사한다.
- fd table에서 `struct file *`을 찾는다.
- filesys 공유 자료구조 보호를 위해 `filesys_lock`을 잡는다.
- `filesys_*` 또는 `file_*` 함수를 호출한다.

### 5.1 `open(path)`

사용자 프로그램은 path 문자열만 넘긴다.

```c
int fd = open("/home/a.txt");
```

kernel 내부 흐름은 다음과 같다.

```text
handle_open()
  -> user string 검증
  -> filesys_lock 획득
  -> filesys_open(path)
     -> resolve_path(path)
     -> inode 찾기
     -> file_open(inode)
  -> filesys_lock 해제
  -> fd_alloc(file)
  -> fd 반환
```

`filesys_open()`은 경로 해석을 담당하고, `file_open()`은 inode를 열린 파일 객체로 감싼다. 여기서 일반 파일이면 일반 file object가 되고, 디렉터리이면 directory-capable file object가 된다.

출처: `pintos/userprog/syscall.c`

```c
static void
handle_open (struct syscall_entry *entry) {
  entry->should_return_value = true;
  const char *filename = (const char *) entry->args[0];

  if (!is_valid_user_string (filename)) {
    _exit (-1);
  }

  lock_acquire (&filesys_lock);
  struct file *file = filesys_open (filename);
  lock_release (&filesys_lock);
  if (file == NULL) {
    entry->return_value = -1;
    return;
  }

  int fd = fd_alloc (file);
  if (fd == -1) {
    lock_acquire (&filesys_lock);
    file_close (file);
    lock_release (&filesys_lock);
    entry->return_value = -1;
    return;
  }

  entry->return_value = fd;
}
```

이 코드에서 syscall 계층은 path를 직접 inode로 바꾸지 않는다. `filesys_open()`이 경로 해석과 `file_open()` 호출을 처리하고, syscall 계층은 반환된 `struct file *`를 fd table에 등록한다.

### 5.2 `read(fd, buffer, size)`

일반 파일 fd에 대한 read 흐름은 다음이다.

```text
handle_read()
  -> user buffer writable 검증
  -> fd_lookup(fd)로 struct file * 찾기
  -> filesys_lock 획득
  -> file_read(file, buffer, size)
     -> inode_read_at(file->inode, buffer, size, file->pos)
        -> byte_to_sector()
           -> FAT chain 탐색
           -> sector 번호 계산
        -> disk_read()
     -> file->pos 전진
  -> filesys_lock 해제
  -> 읽은 byte 수 반환
```

`read()`에서는 kernel이 사용자 buffer에 데이터를 써야 하므로 writable buffer 검증이 필요하다.

출처: `pintos/userprog/syscall.c`

```c
static void
handle_read (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];
  uint8_t *buffer = (void *) entry->args[1];
  size_t size = entry->args[2];

  if (!is_valid_user_buffer (buffer, size, true)) {
    _exit (-1);
  }

  if (fd == STDOUT_FILENO)
    _exit (-1);

  if (fd == STDIN_FILENO) {
    size_t idx = 0;
    while (idx == (size - 1)) {
      buffer[idx] = input_getc();
      idx++;
    }
    entry->return_value = size;
    return;
  }

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  touch_user_buffer (buffer, size, true);

  lock_acquire (&filesys_lock);
  entry->return_value = file_read (file, buffer, size);
  lock_release (&filesys_lock);
}
```

파일 fd인 경우 핵심 호출은 `fd_lookup(fd)` 다음의 `file_read(file, buffer, size)`다. 그 아래에서 `inode_read_at()`과 FAT/disk 계층으로 내려간다.

### 5.3 `write(fd, buffer, size)`

일반 파일 fd에 대한 write 흐름은 다음이다.

```text
handle_write()
  -> user buffer readable 검증
  -> fd_lookup(fd)로 struct file * 찾기
  -> filesys_lock 획득
  -> file_write(file, buffer, size)
     -> inode_write_at(file->inode, buffer, size, file->pos)
        -> 필요하면 inode_extend()
           -> fat_create_chain()
           -> 새 cluster zero fill
           -> inode.length 갱신
        -> byte_to_sector()
           -> FAT chain 탐색
           -> sector 번호 계산
        -> disk_write()
     -> file->pos 전진
  -> filesys_lock 해제
  -> 쓴 byte 수 반환
```

Project 4에서 file growth가 구현되었기 때문에 EOF 뒤에 write하면 `inode_extend()`가 FAT chain을 늘린다.

출처: `pintos/userprog/syscall.c`

```c
static void
handle_write (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];
  const void *buffer = (const void *) entry->args[1];
  size_t size = entry->args[2];

  if (!is_valid_user_buffer ((void *) buffer, size, false)) {
    _exit (-1);
  }

  if (fd == STDOUT_FILENO) {
    putbuf (buffer, size);
    entry->return_value = size;
    return;
  }

  if (fd == STDIN_FILENO)
    _exit (-1);

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  touch_user_buffer ((void *) buffer, size, false);

  lock_acquire (&filesys_lock);
  entry->return_value = file_write (file, buffer, size);
  lock_release (&filesys_lock);
}
```

`write()`에서는 kernel이 사용자 buffer에서 읽기만 하므로 readable buffer 검증을 한다. 실제 파일 확장 여부는 syscall이 판단하지 않고 `file_write()` 아래의 `inode_write_at()`이 처리한다.

### 5.4 `filesize`, `seek`, `tell`, `close`

이 syscall들은 `struct file`의 기본 상태를 다룬다.

```text
filesize(fd)
  -> fd_lookup(fd)
  -> file_length(file)
     -> inode_length(file->inode)

seek(fd, position)
  -> fd_lookup(fd)
  -> file_seek(file, position)
     -> file->pos = position

tell(fd)
  -> fd_lookup(fd)
  -> file_tell(file)
     -> file->pos 반환

close(fd)
  -> fd_close(fd)
     -> file_close(file)
        -> directory면 dir_close()
        -> 일반 파일이면 inode_close()
```

`close()`가 마지막 opener를 닫는 경우, removed inode라면 `inode_close()`에서 FAT chain 반환까지 이어질 수 있다.

```text
close(fd)
  -> file_close()
  -> inode_close()
  -> fat_remove_chain() if removed and last opener
```

출처: `pintos/userprog/syscall.c`

```c
static void
handle_filesize (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  lock_acquire (&filesys_lock);
  entry->return_value = file_length (file);
  lock_release (&filesys_lock);
}

static void
handle_seek (struct syscall_entry *entry) {
  int fd = (int) entry->args[0];
  off_t position = (off_t) entry->args[1];

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  lock_acquire (&filesys_lock);
  file_seek (file, position);
  lock_release (&filesys_lock);
}
```

`filesize()`와 `seek()`는 fd를 `struct file *`로 바꾼 뒤 file 계층의 API를 얇게 감싼다.

```c
static void
handle_tell (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  lock_acquire (&filesys_lock);
  entry->return_value = file_tell (file);
  lock_release (&filesys_lock);
}

static void
handle_close (struct syscall_entry *entry) {
  int fd = (int) entry->args[0];

  if (!is_valid_file_fd (fd))
    _exit (-1);
  lock_acquire (&filesys_lock);
  bool ok = fd_close (fd);
  lock_release (&filesys_lock);
  if (!ok)
    _exit (-1);
}
```

`close()`는 `fd_close()`를 통해 fd table entry를 제거하고 `file_close()`까지 이어진다.

### 5.5 `readdir`, `isdir`, `inumber`

Project 4에서 directory fd 지원을 위해 추가된 syscall들이다.

```text
readdir(fd, name)
  -> fd_lookup(fd)
  -> file_readdir(file, name)
     -> file_is_dir(file)
     -> dir_readdir(file->dir, name)
```

`readdir()`은 kernel이 사용자 `name` buffer에 파일명을 써 주므로 writable buffer 검증이 필요하다.

```text
isdir(fd)
  -> fd_lookup(fd)
  -> file_is_dir(file)
```

```text
inumber(fd)
  -> fd_lookup(fd)
  -> file_get_inode(file)
  -> inode_get_inumber(inode)
```

Pintos에서는 inode가 저장된 sector 번호를 inode number로 사용할 수 있다.

출처: `pintos/userprog/syscall.c`

```c
static void
handle_readdir (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];
  char *name = (char *) entry->args[1];

  if (!is_valid_user_buffer (name, NAME_MAX + 1, true))
    _exit (-1);
  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  lock_acquire (&filesys_lock);
  entry->return_value = file_readdir (file, name);
  lock_release (&filesys_lock);
}
```

`readdir()`도 fd를 `struct file *`로 바꾼 뒤 `file_readdir()`만 호출한다. directory인지 확인하고 실제 entry를 읽는 일은 file/directory 계층이 처리한다.

```c
static void
handle_isdir (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  entry->return_value = file != NULL && file_is_dir (file);
}

static void
handle_inumber (struct syscall_entry *entry) {
  entry->should_return_value = true;
  int fd = (int) entry->args[0];

  if (!is_valid_file_fd (fd))
    _exit (-1);

  struct file *file = fd_lookup (fd);
  if (file == NULL)
    _exit (-1);

  entry->return_value = inode_get_inumber (file_get_inode (file));
}
```

`isdir()`은 `file_is_dir()`, `inumber()`는 `file_get_inode()`와 `inode_get_inumber()`를 사용한다.

### 5.6 `create`, `remove`, `mkdir`, `chdir`, `symlink`

이 syscall들은 단순히 fd에 대한 작업이 아니라 경로와 directory 구조를 바꾼다. 그래서 `file.c`보다 `filesys.c`와 `directory.c`가 더 직접적으로 중요하다.

```text
create(path, size)
  -> filesys_create(path, size)
     -> resolve_parent(path)
     -> inode_create_typed(..., INODE_FILE)
     -> dir_add(parent, basename, inode_sector)
```

```text
mkdir(path)
  -> filesys_mkdir(path)
     -> resolve_parent(path)
     -> dir_create_with_parent()
        -> inode_create_typed(..., INODE_DIR)
        -> "."과 ".." entry 추가
     -> dir_add(parent, basename, inode_sector)
```

```text
remove(path)
  -> filesys_remove(path)
     -> resolve_parent(path)
     -> directory 비어 있는지 확인
     -> dir_remove()
        -> inode_remove()
     -> 마지막 close 시 inode_close()에서 FAT chain 반환
```

```text
symlink(target, linkpath)
  -> filesys_symlink(target, linkpath)
     -> inode_create_typed(..., INODE_SYMLINK)
     -> inode_write_at()으로 target 문자열 저장
     -> dir_add(parent, basename, inode_sector)
```

여기서도 데이터 저장은 결국 `inode_write_at()`을 통하고, 필요한 경우 FAT cluster chain이 할당된다.

출처: `pintos/userprog/syscall.c`

```c
static void
handle_create (struct syscall_entry *entry) {
  entry->should_return_value = true;
  const char *filename = (const char *) entry->args[0];
  size_t initial_size = entry->args[1];

  if (!is_valid_user_string (filename)) {
    _exit (-1);
  }

  lock_acquire (&filesys_lock);
  entry->return_value = filesys_create (filename, initial_size);
  lock_release (&filesys_lock);
}

static void
handle_mkdir (struct syscall_entry *entry) {
  entry->should_return_value = true;
  const char *dir = (const char *) entry->args[0];

  if (!is_valid_user_string (dir))
    _exit (-1);

  lock_acquire (&filesys_lock);
  entry->return_value = filesys_mkdir (dir);
  lock_release (&filesys_lock);
}
```

이 계열 syscall은 `file_*` API보다 `filesys_*` API를 먼저 호출한다. 이유는 fd로 이미 열린 파일을 다루는 것이 아니라 path를 해석하고 directory entry를 추가/삭제해야 하기 때문이다.

출처: `pintos/filesys/filesys.c`

```c
bool
filesys_create (const char *name, off_t initial_size) {
  struct dir *dir = NULL;
  char basename[NAME_MAX + 1];
  disk_sector_t inode_sector = 0;
  bool success = false;
  struct inode *existing = NULL;

  if (!resolve_parent (name, &dir, basename))
    return false;
  if (is_special_dir_name (basename))
    goto done;
  if (dir_lookup (dir, basename, &existing)) {
    inode_close (existing);
    goto done;
  }
  if (!allocate_inode_sector (&inode_sector))
    goto done;

  success = inode_create_typed (inode_sector, initial_size, INODE_FILE)
            && dir_add (dir, basename, inode_sector);
  if (!success)
    remove_created_inode (inode_sector);

done:
  dir_close (dir);
  return success;
}
```

`filesys_create()`는 parent directory를 찾고, inode sector를 할당한 뒤, `inode_create_typed()`와 `dir_add()`를 연결한다. 따라서 create도 결국 inode/FAT 계층을 사용한다.

`symlink()`는 target path 문자열을 symlink inode의 data로 저장하므로 `inode_write_at()`을 직접 사용한다.

```c
int
filesys_symlink (const char *target, const char *linkpath) {
  struct dir *dir = NULL;
  char basename[NAME_MAX + 1];
  disk_sector_t inode_sector = 0;
  struct inode *inode = NULL;
  struct inode *existing = NULL;
  size_t target_len;
  bool success = false;

  if (target == NULL || target[0] == '\0')
    return -1;
  if (!resolve_parent (linkpath, &dir, basename))
    return -1;
  if (is_special_dir_name (basename))
    goto done;
  if (dir_lookup (dir, basename, &existing)) {
    inode_close (existing);
    goto done;
  }
  if (!allocate_inode_sector (&inode_sector))
    goto done;

  target_len = strlen (target) + 1;
  if (!inode_create_typed (inode_sector, target_len, INODE_SYMLINK)) {
    remove_created_inode (inode_sector);
    goto done;
  }
  inode = inode_open (inode_sector);
  success = inode != NULL
            && inode_write_at (inode, target, target_len, 0) == (off_t) target_len
            && dir_add (dir, basename, inode_sector);
  if (!success)
    remove_created_inode (inode_sector);

done:
  inode_close (inode);
  dir_close (dir);
  return success ? 0 : -1;
}
```

## 6. 예시로 보는 전체 흐름

### 예시 1: `write(fd, "hello", 5)`

```text
user program
  |
  | write(fd, "hello", 5)
  v
syscall.c: handle_write()
  - user buffer readable 검증
  - fd_lookup(fd)
  - filesys_lock 획득
  |
  v
file.c: file_write()
  - directory fd이면 실패
  - 현재 file->pos에서 쓰기 시작
  |
  v
inode.c: inode_write_at()
  - EOF를 넘으면 inode_extend()
  - file offset을 byte_to_sector()로 sector 번호로 변환
  |
  v
fat.c
  - inode_extend() 중 fat_create_chain()으로 cluster 할당
  - byte_to_sector() 중 fat_get()으로 chain 탐색
  - cluster_to_sector()로 sector 번호 변환
  |
  v
disk.c
  - disk_write(filesys_disk, sector, buffer)
```

결과적으로 사용자는 byte 단위로 `"hello"`를 썼지만, 내부에서는 FAT cluster를 할당하고 sector 단위로 disk write가 수행된다.

### 예시 2: EOF 뒤 write로 파일 확장

현재 파일 길이가 3바이트라고 하자.

```text
offset 0: a
offset 1: b
offset 2: c
EOF = 3
```

사용자가 offset 10으로 seek한 뒤 1바이트를 쓰면:

```text
seek(fd, 10)
write(fd, "X", 1)
```

흐름은 다음과 같다.

```text
file_seek()
  -> file->pos = 10

file_write()
  -> inode_write_at(offset = 10, size = 1)
     -> offset + size = 11이 기존 length 3보다 큼
     -> inode_extend(length = 11)
        -> 필요한 cluster 계산
        -> fat_create_chain()으로 부족한 cluster 할당
        -> 새 cluster zero fill
        -> inode.length = 11
     -> 실제 "X" 쓰기
```

결과적으로 중간 gap은 0으로 채워진다.

```text
offset 0:  a
offset 1:  b
offset 2:  c
offset 3:  0
...
offset 9:  0
offset 10: X
EOF = 11
```

### 예시 3: `readdir(fd, name)`

디렉터리 fd는 일반 파일 fd와 같은 fd table에 들어 있지만, `struct file` 안의 `dir` 필드가 non-null이다.

```text
open("/home")
  -> filesys_open()
  -> file_open(directory_inode)
     -> dir_open(inode)
     -> file->dir = struct dir *
  -> fd_alloc(file)
```

그 뒤 `readdir(fd, name)`은 다음처럼 동작한다.

```text
handle_readdir()
  -> name buffer writable 검증
  -> fd_lookup(fd)
  -> file_readdir(file, name)
     -> file_is_dir(file)
     -> dir_readdir(file->dir, name)
        -> inode_read_at(directory_inode, ...)
        -> directory entry 읽기
        -> "."과 ".."는 건너뜀
```

디렉터리도 inode를 backing store로 사용하는 파일이므로, directory entry를 읽는 작업도 내부적으로는 `inode_read_at()`과 FAT/disk 계층을 지나간다.

## 7. 한 줄 요약

계층별 책임은 다음처럼 정리할 수 있다.

```text
disk.c
  sector를 읽고 쓴다.

fat.c
  sector 위에 cluster chain을 만든다.

inode.c
  FAT chain을 이용해 파일 offset을 sector로 바꾸고 파일을 확장한다.

file.c
  열린 파일 객체, 현재 위치, directory fd, read/write API를 제공한다.

syscall.c
  사용자 요청을 검증하고 fd를 file 객체로 바꾼 뒤 file/filesys API를 호출한다.
```

따라서 Pintos filesys를 읽을 때는 `file.c`만 보거나 `fat.c`만 보면 흐름이 끊긴다. 실제 핵심은 `file.c -> inode.c -> fat.c -> disk.c`로 내려가는 변환 과정이다.
