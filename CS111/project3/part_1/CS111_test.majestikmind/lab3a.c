#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "ext2_fs.h"
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>


/* The superblock is always located at byte
 offset 1024 from the beginning of the file */
#define SUPERBLK_OFFSET 1024
#define INODE_TBL_LEN   214
#define BLOCK_PTR_LEN   60

int BLOCK_SIZE;
int fs_fd; // File System File Descriptor for pread

void
print_ind_blks(int blk_nmbr, int inode_number, int offset, int lvl)
{
    __u32 ind_blocks[ BLOCK_SIZE * sizeof(__u32) ];
    int n_ind_ptrs_per_blk = BLOCK_SIZE / sizeof(__u32);
    int block_offset = blk_nmbr * BLOCK_SIZE;
    int ret_val = pread(fs_fd, ind_blocks, sizeof(BLOCK_SIZE), block_offset);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    for (int i = 0; i < n_ind_ptrs_per_blk; i++)
    {
        if (ind_blocks[i] != 0)
        {
            printf("%s,%d,%d,%d,%d,%d\n",
                   "INDIRECT",
                   inode_number,
                   lvl,
                   offset,
                   blk_nmbr,
                   ind_blocks[i]
                   );
            
            /** Recursive call:. Base case when lvl = 1. */
            if ( lvl > 1)
                print_ind_blks(ind_blocks[i], inode_number, offset, lvl-1);
        }
        
        if (lvl == 1) { offset++; }
        else if (lvl == 2) { offset += 256; }
        else if (lvl == 3) { offset += (256 * 256); }
    }
    
    return;
}

void
get_time(char *buf, time_t rawtime)
{
    
    struct tm *info;
    info = gmtime( &rawtime );
    int len;
    len = snprintf(buf, 32, "%02d/%02d/%d %02d:%02d:%02d, GMT",
           info->tm_mon + 1, /// months since January
           info->tm_mday,
           info->tm_year % 100, /// years since 1900, YY  format
           info->tm_hour,
           info->tm_min,
           info->tm_sec
           );
    
    assert(len < 32);
    return;
}


int
main(int argc, char** argv) {
    
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments: ./lab3a [image]\n");
        exit(1);
    }
    
    
    /** On Linux 2.6 and later, O_EXCL can be used without O_CREAT if pathname
     refers to a block device. If the block device is in use by the system
     (e.g., mounted), open() fails with the error EBUSY. */
    fs_fd = open(argv[1], O_RDONLY | O_EXCL);
    if (fs_fd == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    
    ssize_t ret_val;
    // SUPERBLOCK SUMMARY
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    char superblk[sizeof(struct ext2_super_block)];
    struct ext2_super_block *fs_superblk = (struct ext2_super_block*) superblk;
    
    /** Get superblock data* */
    ret_val = pread(fs_fd, fs_superblk, sizeof(struct ext2_super_block), SUPERBLK_OFFSET);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    BLOCK_SIZE = EXT2_MIN_BLOCK_SIZE << (fs_superblk->s_log_block_size);
    
    
    printf("%s,%i,%i,%i,%i,%i,%i,%i\n",
           "SUPERBLOCK",
           fs_superblk->s_blocks_count,
           fs_superblk->s_inodes_count,
           BLOCK_SIZE,
           fs_superblk->s_inode_size,
           fs_superblk->s_blocks_per_group,
           fs_superblk->s_inodes_per_group,
           fs_superblk->s_first_ino
           );
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    
    // BLOCK GROUP SUMMARIES
    // Current implementation assumes that only one block group is present on the image.
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    int group_number = 0;
    
    /** Read in block group descriptor */
    char blk_tbl[sizeof(struct ext2_group_desc)];
    struct ext2_group_desc *fs_blk_tbl = (struct ext2_group_desc*) blk_tbl;
    ret_val = pread(fs_fd, fs_blk_tbl, sizeof(struct ext2_group_desc), SUPERBLK_OFFSET + BLOCK_SIZE);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    /* TODO: Calculate number of blocks and inodes.
        Current implementation assumes that only one
        block group is present on the image. */
 
    
    printf("%s,%d,%i,%i,%i,%i,%i,%i,%i\n",
           "GROUP",
           group_number,
           fs_superblk->s_blocks_count,
           fs_superblk->s_inodes_count,
           fs_blk_tbl->bg_free_blocks_count,
           fs_blk_tbl->bg_free_inodes_count,
           fs_blk_tbl->bg_block_bitmap,
           fs_blk_tbl->bg_inode_bitmap,
           fs_blk_tbl->bg_inode_table
           );
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    
    // FREE BLOCK SUMMARY (PER GROUP)
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    char block_bitmap[BLOCK_SIZE];
    ret_val = pread(fs_fd, block_bitmap, BLOCK_SIZE, (fs_blk_tbl->bg_block_bitmap) * BLOCK_SIZE);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    int blk_cnt = fs_superblk->s_blocks_per_group;
    int bitmap_mask = 0x01;
    int block_bitmap_sz = BLOCK_SIZE;
    int free_blk_sum = 0;
    /** We need to check each byte in the bitmap:  Each bit represent the current state of a
     block within that block group, where 1 means “used” and 0 “free/available”. The first
     block of this block group is represented by bit 0 of byte 0, the second by bit 1 of byte 0...  */
    for (int i = 0; i < block_bitmap_sz; i++)
    {
        /** Each byte in the bitmap has 8 bits, each of which must be checked */
        for (int j = 0; j < 8; j++)
        {
            if ( ! (bitmap_mask & block_bitmap[i]) )
            {
                /** Each byte has eight bits, each bit is indexed by j */
                printf("BFREE, %d\n", (8 * i) + j + 1);
                free_blk_sum++;
            }
            
            /** Right Arithmetic Shift */
            block_bitmap[i] >>= 1;
            
            /** Stop iterating over bitmap one we've accounted for every block in the group */
            blk_cnt--;
            if (blk_cnt == 0) { break; }
            
        }
    }
    
    fprintf(stderr, "Block bitmap yields %d free blocks\n", free_blk_sum);
    assert(free_blk_sum == fs_blk_tbl->bg_free_blocks_count);
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    
    // FREE INODE SUMMARY (PER GROUP)
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    char inode_bitmap[BLOCK_SIZE];
    ret_val = pread(fs_fd, inode_bitmap, BLOCK_SIZE, (fs_blk_tbl->bg_inode_bitmap) * BLOCK_SIZE);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    
    int inode_cnt = fs_superblk->s_inodes_per_group;
    int inode_bitmap_sz = BLOCK_SIZE;
    int free_inode_sum = 0;
    /** We need to check each byte in the bitmap:  Each bit represent the current state of a
     block within that block group, where 1 means “used” and 0 “free/available”. The first
     block of this block group is represented by bit 0 of byte 0, the second by bit 1 of byte 0...  */
    for (int i = 0; i < inode_bitmap_sz; i++)
    {
        /** Each byte in the bitmap has 8 bits, each of which must be checked */
        for (int j = 0; j < 8; j++)
        {
            if ( ! (bitmap_mask & inode_bitmap[i]) )
            {
                /** Since inode numbers start from 1 rather than 0, the first bit in
                 the first block group's inode bitmap represent inode number 1. */
                printf("IFREE, %d\n", (8 * i) + j + 1);
                free_inode_sum++;
            }
            
            /** Right Arithmetic Shift */
            inode_bitmap[i] >>= 1;
            
            /** Stop iterating over bitmap one we've accounted for every block in the group */
            inode_cnt--;
            if (inode_cnt == 0) { break; }
            
        }
    }
    
    fprintf(stderr, "Block bitmap yields %d free blocks\n", free_inode_sum);
    assert(free_inode_sum == fs_blk_tbl->bg_free_inodes_count);
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    
    
    // INODE TABLE SUMMARY (PER GROUP)
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    size_t inode_table_size = fs_superblk->s_inodes_per_group * sizeof(struct ext2_inode);
    char inode_tbl_data[inode_table_size];
    struct ext2_inode *inode_tbl_ptr = (struct ext2_inode*) inode_tbl_data;
    ret_val = pread(fs_fd, inode_tbl_ptr, inode_table_size, (fs_blk_tbl->bg_inode_table) * BLOCK_SIZE);
    if (ret_val == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    
    int mode_mask = 0x0fff;
    int inode_number = 1; ///
    int n_inode_allocated = 0;
    int max_inodes = fs_superblk->s_inodes_per_group;
    for (int i = 0; i < max_inodes; i++)
    {
        struct ext2_inode inode_curr = inode_tbl_ptr[i];
        /// Only consider allocated (non-zero mode and non-zero link count) inodes
        if ( (inode_curr.i_mode != 0000) && (inode_curr.i_links_count != 0) )
        {
            /// Determine file type
            char file_type;
            if ( S_ISDIR(inode_curr.i_mode) ) { file_type = 'd'; }
            else if ( S_ISREG(inode_curr.i_mode) ) { file_type = 'f'; }
            else if ( S_ISLNK(inode_curr.i_mode) ) { file_type = 's'; }
            else { file_type = '?'; }
            
            
            char blk_addrs[128];
            bzero(blk_addrs, sizeof(blk_addrs));
            /** Determine if block addresses need to be printed */
            bool print_blk_addrs = false;
            if (file_type == 'f')
                print_blk_addrs = true;
            else if (file_type == 'd')
                print_blk_addrs  = true;
            else if (file_type == 's')
                if (inode_curr.i_size > BLOCK_PTR_LEN /** 60 bytes */ )
                    print_blk_addrs = true;
            
            if (print_blk_addrs == true) {
                char blk_addr[8];
                for (int i = 0; i < EXT2_N_BLOCKS - 1; i++) {
                    sprintf(blk_addr, "%d,", inode_curr.i_block[i] );
                    strcat(blk_addrs, blk_addr);
                }
                /// Last block number  is followed by newline instead of comma
                sprintf(blk_addr, "%d", inode_curr.i_block[ EXT2_N_BLOCKS - 1 ] );
                strcat(blk_addrs, blk_addr);
            }
            
            char ctime_buf[32], mtime_buf[32], atime_buf[32];
            get_time(ctime_buf, inode_curr.i_ctime);
            get_time(mtime_buf, inode_curr.i_mtime);
            get_time(atime_buf, inode_curr.i_atime);
            
            printf("%s,%d,%c,%04o,%d,%d,%d,%s,%s,%s,%d,%d,%s\n",
                   "INODE",
                   inode_number,
                   file_type,
                   inode_curr.i_mode & mode_mask,
                   inode_curr.i_uid,
                   inode_curr.i_gid,
                   inode_curr.i_links_count,
                   ctime_buf,
                   mtime_buf,
                   atime_buf,
                   inode_curr.i_size,
                   inode_curr.i_blocks,
                   blk_addrs
                   );

            
            // DIRECTORY ENTIRES
            if (file_type == 'd')
            {
                char dir_data[ sizeof(struct ext2_dir_entry) ];
                struct ext2_dir_entry *dir_cur = (struct ext2_dir_entry*) dir_data;
                int logical_offset = 0;
                for (int i = 0; i < EXT2_NDIR_BLOCKS; i++)
                {
                    if (inode_curr.i_block[i] != 0)
                    {
                        int inode_index = inode_curr.i_block[i];
                        while (logical_offset < BLOCK_SIZE) {
                            int dir_offset = (inode_index*BLOCK_SIZE) + logical_offset;
                            ret_val = pread(fs_fd, dir_cur, sizeof(struct ext2_dir_entry), dir_offset);
                            if (ret_val == -1) {
                                fprintf(stderr, "%s\n", strerror(errno));
                                exit(1);
                            }

                            if (dir_cur->inode != 0)
                            {
                                printf("%s,%d,%d,%d,%d,%d,%s\n",
                                       "DIRECT",
                                       inode_number,
                                       logical_offset,
                                       dir_cur->inode,
                                       dir_cur->rec_len,
                                       dir_cur->name_len,
                                       dir_cur->name
                                       );
                            }

                            logical_offset += dir_cur->rec_len;
                        }
                    }
                }
            }

            
            if (inode_curr.i_block[EXT2_IND_BLOCK] != 0) {
                int blk_nmbr = inode_curr.i_block[EXT2_IND_BLOCK];
                print_ind_blks(blk_nmbr, inode_number, 12, 1);
            }

            if (inode_curr.i_block[EXT2_DIND_BLOCK] != 0) {
                int blk_nmbr = inode_curr.i_block[EXT2_DIND_BLOCK];
                print_ind_blks(blk_nmbr, inode_number, 268, 2);
            }

            if (inode_curr.i_block[EXT2_TIND_BLOCK] != 0) {
                int blk_nmbr = inode_curr.i_block[EXT2_TIND_BLOCK];
                print_ind_blks(blk_nmbr, inode_number, 65804, 3);
            }

            // For error-checking
            n_inode_allocated++;
            fprintf(stderr, "...\n");
            
        }
        
        inode_number++;
        
    }
    // TODO: This assertion test fails. Indirection?
    //     printf("inodes counted: %d\n", n_inode_allocated);
    //     assert(n_inode_allocated == (fs_superblk->s_inodes_count));
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
    
    
    
}
