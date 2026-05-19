#ifndef FILESYS_FAT_H
#define FILESYS_FAT_H

#include "devices/disk.h"
#include "filesys/file.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t cluster_t;  /* Index of a cluster within FAT. */
/* FAT 안에서 cluster의 index. */

#define FAT_MAGIC 0xEB3C9000 /* MAGIC string to identify FAT disk */
/* FAT 디스크를 식별하기 위한 MAGIC string. */
#define EOChain 0x0FFFFFFF   /* End of cluster chain */
/* cluster chain의 끝. */

/* Sectors of FAT information. */
/* FAT 정보가 저장되는 sector들. */
#define SECTORS_PER_CLUSTER 1 /* Number of sectors per cluster */
/* cluster당 sector 수. */
#define FAT_BOOT_SECTOR 0     /* FAT boot sector. */
/* FAT boot sector이다. */
#define ROOT_DIR_CLUSTER 1    /* Cluster for the root directory */
/* root 디렉터리용 cluster. */

void fat_init (void);
void fat_open (void);
void fat_close (void);
void fat_create (void);
void fat_close (void);

cluster_t fat_create_chain (
    cluster_t clst /* Cluster # to stretch, 0: Create a new chain */
    /* 늘릴 cluster 번호, 0이면 새 chain을 만든다. */
);
void fat_remove_chain (
    cluster_t clst, /* Cluster # to be removed */
    /* 제거할 cluster 번호. */
    cluster_t pclst /* Previous cluster of clst, 0: clst is the start of chain */
    /* clst의 이전 cluster, 0이면 clst가 chain의 시작이다. */
);
cluster_t fat_get (cluster_t clst);
void fat_put (cluster_t clst, cluster_t val);
disk_sector_t cluster_to_sector (cluster_t clst);
cluster_t sector_to_cluster (disk_sector_t sector);

#endif /* filesys/fat.h */
