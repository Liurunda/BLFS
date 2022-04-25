//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_SUPERBLOCK_H
#define BLFS_SUPERBLOCK_H

#include <linux/types.h>

class Superblock {
public:
    Superblock(Superblock &) = delete;

    Superblock(Superblock &&) = delete;

    void operator=(const Superblock &) = delete;

    static Superblock *get_instance() {
        if (superblock_instance == nullptr) {
            superblock_instance = new Superblock();
        }
        return superblock_instance;
    }

    static void destroy_instance() {
        if (superblock_instance != nullptr) {
            delete superblock_instance;
            superblock_instance = nullptr;
        }
    }

    void traverse_settings_to_data(void *buf);

    void traverse_data_to_settings(void *buf);

    __le32 s_inodes_count;                  // Total inode count.
    __le32 s_blocks_count_lo;               // Total block count.
    __le32 s_r_blocks_count_lo;             // This number of blocks can only be allocated by the super-user.
    __le32 s_free_blocks_count_lo;          // Free block count.
    __le32 s_free_inodes_count;             // Free inode count.
    __le32 s_first_data_block;              // First data block. This must be at least 1 for 1k-block filesystems and is typically 0 for all other block sizes.
    __le32 s_log_block_size;                // Block size is 2 ^ (10 + s_log_block_size).
    __le32 s_log_cluster_size;              // Cluster size is 2 ^ (10 + s_log_cluster_size) blocks if bigalloc is enabled. Otherwise s_log_cluster_size must equal s_log_block_size.
    __le32 s_blocks_per_group;              // Blocks per group.
    __le32 s_clusters_per_group;            // Clusters per group, if bigalloc is enabled. Otherwise s_clusters_per_group must equal s_blocks_per_group.
    __le32 s_inodes_per_group;              // Inodes per group.
    __le32 s_mtime;                         // Mount time, in seconds since the epoch.
    __le32 s_wtime;                         // Write time, in seconds since the epoch.
    __le16 s_mnt_count;                     // Number of mounts since the last fsck.
    __le16 s_max_mnt_count;                 // Number of mounts beyond which a fsck is needed.
    __le16 s_magic;                         // Magic signature, 0xEF53
    __le16 s_state;                         // File system state.
    __le16 s_errors;                        // Behaviour when detecting errors.
    __le16 s_minor_rev_level;               // Minor revision level.
    __le32 s_lastcheck;                     // Time of last check, in seconds since the epoch.
    __le32 s_checkinterval;                 // Maximum time between checks, in seconds.
    __le32 s_creator_os;                    // Creator OS.
    __le32 s_rev_level;                     // Revision level.
    __le16 s_def_resuid;                    // Default uid for reserved blocks.
    __le16 s_def_resgid;                    // Default gid for reserved blocks.
    /*
     * These fields are for EXT4_DYNAMIC_REV superblocks only.
     * Note: the difference between the compatible feature set and the incompatible feature set is that if there is a bit set in the incompatible feature set that the kernel doesn’t know about, it should refuse to mount the filesystem.
     * e2fsck’s requirements are more strict; if it doesn’t know about a feature in either the compatible or incompatible feature set, it must abort and not try to meddle with things it doesn’t understand…
     */
    __le32 s_first_ino;                     // First non-reserved inode.
    __le16 s_inode_size;                    // Size of inode structure, in bytes.
    __le16 s_block_group_nr;                // Block group # of this superblock.
    __le32 s_feature_compat;                // Compatible feature set flags. Kernel can still read/write this fs even if it doesn’t understand a flag; fsck should not do that.
    __le32 s_feature_incompat;              // Incompatible feature set. If the kernel or fsck doesn’t understand one of these bits, it should stop.
    __le32 s_feature_ro_compat;             // Readonly-compatible feature set. If the kernel doesn’t understand one of these bits, it can still mount read-only.
    __u8 s_uuid[16];                        // 128-bit UUID for volume.
    char s_volume_name[16];                 // Volume label.
    char s_last_mounted[64];                // Directory where filesystem was last mounted.
    __le32 s_algorithm_usage_bitmap;        // For compression (Not used in e2fsprogs/Linux)
    /*
     * Performance hints. Directory preallocation should only happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
     */
    __u8 s_prealloc_blocks;                 // #. of blocks to try to preallocate for … files? (Not used in e2fsprogs/Linux)
    __u8 s_prealloc_dir_blocks;             // #. of blocks to preallocate for directories. (Not used in e2fsprogs/Linux)
    __le16 s_reserved_gdt_blocks;           // Number of reserved GDT entries for future filesystem expansion.
    /*
     * Journalling support is valid only if EXT4_FEATURE_COMPAT_HAS_JOURNAL is set.
     */
    __u8 s_journal_uuid[16];                // UUID of journal superblock
    __le32 s_journal_inum;                  // inode number of journal file.
    __le32 s_journal_dev;                   // Device number of journal file, if the external journal feature flag is set.
    __le32 s_last_orphan;                   // Start of list of orphaned inodes to delete.
    __le32 s_hash_seed[4];                  // HTREE hash seed.
    __u8 s_def_hash_version;                // Default hash algorithm to use for directory hashes.
    __u8 s_jnl_backup_type;                 // If this value is 0 or EXT3_JNL_BACKUP_BLOCKS (1), then the s_jnl_blocks field contains a duplicate copy of the inode’s i_block[] array and i_size.
    __le16 s_desc_size;                     // Size of group descriptors, in bytes, if the 64bit incompat feature flag is set.
    __le32 s_default_mount_opts;            // Default mount options.
    __le32 s_first_meta_bg;                 // First metablock block group, if the meta_bg feature is enabled.
    __le32 s_mkfs_time;                     // When the filesystem was created, in seconds since the epoch.
    __le32 s_jnl_blocks[17];                // Backup copy of the journal inode’s i_block[] array in the first 15 elements and i_size_high and i_size in the 16th and 17th elements, respectively.
    /*
     * 64bit support is valid only if EXT4_FEATURE_COMPAT_64BIT is set.
     */
    __le32 s_blocks_count_hi;               // High 32-bits of the block count.
    __le32 s_r_blocks_count_hi;             // High 32-bits of the reserved block count.
    __le32 s_free_blocks_count_hi;          // High 32-bits of the free block count.
    __le16 s_min_extra_isize;               // All inodes have at least # bytes.
    __le16 s_want_extra_isize;              // New inodes should reserve # bytes.
    __le32 s_flags;                         // Miscellaneous flags.
    __le16 s_raid_stride;                   // RAID stride. This is the number of logical blocks read from or written to the disk before moving to the next disk. This affects the placement of filesystem metadata, which will hopefully make RAID storage faster.
    __le64 s_mmp_interval;                  // #. seconds to wait in multi-mount prevention (MMP) checking. In theory, MMP is a mechanism to record in the superblock which host and device have mounted the filesystem, in order to prevent multiple mounts. This feature does not seem to be implemented…
    __le64 s_mmp_block;                     // Block # for multi-mount protection data.
    __le32 s_raid_stripe_width;             // RAID stripe width. This is the number of logical blocks read from or written to the disk before coming back to the current disk. This is used by the block allocator to try to reduce the number of read-modify-write operations in a RAID5/6.
    __u8 s_log_groups_per_flex;             // Size of a flexible block group is 2 ^ s_log_groups_per_flex.
    __u8 s_checksum_type;                   // Metadata checksum algorithm type. The only valid value is 1 (crc32c).
    __le16 s_reserved_pad;
    __le64 s_kbytes_written;                // Number of KiB written to this filesystem over its lifetime.
    __le32 s_snapshot_inum;                 // inode number of active snapshot. (Not used in e2fsprogs/Linux.)
    __le32 s_snapshot_id;                   // Sequential ID of active snapshot. (Not used in e2fsprogs/Linux.)
    __le64 s_snapshot_r_blocks_count;       // Number of blocks reserved for active snapshot’s future use. (Not used in e2fsprogs/Linux.)
    __le32 s_snapshot_list;                 // inode number of the head of the on-disk snapshot list. (Not used in e2fsprogs/Linux.)
    __le32 s_error_count;                   // Number of errors seen.
    __le32 s_first_error_time;              // First time an error happened, in seconds since the epoch.
    __le32 s_first_error_ino;               // inode involved in first error.
    __le64 s_first_error_block;             // Number of block involved of first error.
    __u8 s_first_error_func[32];            // Name of function where the error happened.
    __le32 s_first_error_line;              // Line number where error happened.
    __le32 s_last_error_time;               // Time of most recent error, in seconds since the epoch.
    __le32 s_last_error_ino;                // inode involved in most recent error.
    __le32 s_last_error_line;               // Line number where most recent error happened.
    __le64 s_last_error_block;              // Number of block involved in most recent error.
    __u8 s_last_error_func[32];             // Name of function where the most recent error happened.
    __u8 s_mount_opts[64];                  // ASCIIZ string of mount options.
    __le32 s_usr_quota_inum;                // Inode number of user quota file.
    __le32 s_grp_quota_inum;                // Inode number of group quota file.
    __le32 s_overhead_blocks;               // Overhead blocks/clusters in fs. (Huh? This field is always zero, which means that the kernel calculates it dynamically.)
    __le32 s_backup_bgs[2];                 // Block groups containing superblock backups (if sparse_super2)
    __u8 s_encrypt_algos[4];                // Encryption algorithms in use. There can be up to four algorithms in use at any time; valid algorithm codes are given in the super_encrypt table below.
    __u8 s_encrypt_pw_salt[16];             // Salt for the string2key algorithm for encryption.
    __le32 s_lpf_ino;                       // Inode number of lost+found
    __le32 s_prj_quota_inum;                // Inode that tracks project quotas.
    __le32 s_checksum_seed;                 // Checksum seed used for metadata_csum calculations. This value is crc32c(~0, $orig_fs_uuid).
    __u8 s_wtime_hi;                        // Upper 8 bits of the s_wtime field.
    __u8 s_mtime_hi;                        // Upper 8 bits of the s_mtime field.
    __u8 s_mkfs_time_hi;                    // Upper 8 bits of the s_mkfs_time field.
    __u8 s_lastcheck_hi;                    // Upper 8 bits of the s_lastcheck_hi field.
    __u8 s_first_error_time_hi;             // Upper 8 bits of the s_first_error_time_hi field.
    __u8 s_last_error_time_hi;              // Upper 8 bits of the s_last_error_time_hi field.
    __u8 s_pad[2];                          // Zero padding.
    __le16 s_encoding;                      // Filename charset encoding.
    __le16 s_encoding_flags;                // Filename charset encoding flags.
    __le32 s_orphan_file_inum;              // Orphan file inode number.
    __le32 s_reserved[94];                  // Padding to the end of the block.
    __le32 s_checksum;                      // Superblock checksum.

    static const int SUPERBLOCK_SIZE = 0x400;
private:
    Superblock();

    ~Superblock() = default;

    static Superblock *superblock_instance;
};

#endif //BLFS_SUPERBLOCK_H
