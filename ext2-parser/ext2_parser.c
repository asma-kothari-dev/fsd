#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include <stdio.h>

#define SUPERBLOCK_ADDR 1
#define BLOCKGROUP_DESC_ADDR (SUPERBLOCK_ADDR + 1)
#define BLOCK_SIZE 1024
#define DIRECT_BLOCKS 12

#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000
#define EXT2_INDEX_FL 0x00001000

struct SUPERBLOCK {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
} sb;

struct BLOCKGROUP_DESCRIPTOR {
		uint32_t bg_block_bitmap;
		uint32_t bg_inode_bitmap;
		uint32_t bg_inode_table;
		uint16_t bg_free_blocks_count;
		uint16_t bg_free_inodes_count;
		uint16_t bg_used_dirs_count;
		uint16_t bg_pad;
		uint32_t bg_reserved[3];
} bg_descriptor;

struct INODE_TABLE {
		uint16_t	i_mode;
		uint16_t	i_uid;
		uint32_t	i_size;
		uint32_t	i_atime;
		uint32_t	i_ctime;
		uint32_t	i_mtime;
		uint32_t	i_dtime;
		uint16_t	i_gid;
		uint16_t	i_links_count;
		uint32_t	i_blocks;
		uint32_t	i_flags;
		uint32_t	i_osd1;
		uint32_t	i_block[15];
		uint32_t	i_generation;
		uint32_t	i_file_acl;
		uint32_t	i_dir_acl;
		uint32_t	i_faddr;
		uint32_t	i_osd2[3];
} inode_table_entry;

struct DIRENT {
		uint32_t inode;
		uint16_t rec_len;
		uint8_t	name_len;
		uint8_t	file_type;
		uint8_t	name[255];
} dirent;

void read_dirent(int disk_fd, int data_block){
	int total_dirent = 0;
	uint8_t block[BLOCK_SIZE];
	int offset = data_block * BLOCK_SIZE;

	int seekp = lseek(disk_fd, offset, 0);
	//int bytes_read = read(disk_fd, block, BLOCK_SIZE);
	
	int bytes_read = read(disk_fd, &dirent, sizeof(dirent));
	printf("Read inode %d and name %s\n", dirent.inode, dirent.name);
	
	printf("Current offset is %d and updated by %d", offset, dirent.rec_len);
	offset += dirent.rec_len;
	bytes_read = read(disk_fd, &dirent, sizeof(dirent));
	printf("Read inode %d and name %s\n", dirent.inode, dirent.name);
}

void print_directory(int disk_fd, int i_ino, struct INODE_TABLE inode){

	int total_blocks = inode.i_blocks > DIRECT_BLOCKS ? DIRECT_BLOCKS : inode.i_blocks;
	for (int i = 0; i < total_blocks; i++){
		read_dirent(disk_fd, inode.i_block[i]);		
		break;
	}
}

void print_inode_table_entry(int disk_fd, int i_ino, struct INODE_TABLE inode) {
	if (inode.i_mode & EXT2_S_IFREG){
		printf("Inode %d is a regular file\n", i_ino);
	}
	else if (inode.i_mode & EXT2_S_IFDIR){
		printf("Inode %d is a directory\n", i_ino);
		if (inode.i_flags & EXT2_INDEX_FL)	printf("Hashed index directory mode set\n");
		print_directory(disk_fd, i_ino, inode);
	}
}

void print_inode_table(int disk_fd, int inode_table_addr, int start_inode, int total_inodes){
	int offset = inode_table_addr * BLOCK_SIZE;
	int seekp = lseek(disk_fd, offset, 0);

	for (int i = 0; i < total_inodes; i++){
		if (i < start_inode) continue;

		int bytes_read = read(disk_fd, &inode_table_entry, sizeof(inode_table_entry));
		print_inode_table_entry(disk_fd, i, inode_table_entry);
	}
}

void read_blockgroup_descriptor_table(int disk_fd, int total_blockgroups, int total_inodes){
	int offset = BLOCKGROUP_DESC_ADDR * BLOCK_SIZE;
	int seekp = lseek(disk_fd, offset, 0);

	printf("Going to read %d blockgroups\n", total_blockgroups);
	for (int i = 0; i < total_blockgroups; i++) {
		int bytes_read = read(disk_fd, &bg_descriptor, sizeof(bg_descriptor));
		printf("[%d] Blockgroup_desc->bg_inode_table -> %d\n", i, bg_descriptor.bg_inode_table);
		printf("[%d] Blockgroup_desc->bg_block_bitmap -> %d\n", i, bg_descriptor.bg_block_bitmap);
		printf("[%d] Blockgroup_desc->bg_inode_bitmap -> %d\n", i, bg_descriptor.bg_inode_bitmap);

		print_inode_table(disk_fd, bg_descriptor.bg_inode_table, 0, total_inodes);
		break;
	}
}


void read_block(int block_no, void *buffer, int disk_fd, int count){
	int offset = block_no * BLOCK_SIZE;
	lseek(disk_fd, offset, 0);
	int bytes_read = read(disk_fd, buffer, BLOCK_SIZE * count);
}

int main() {
    int disk_fd = open("./test_disk", O_RDONLY);
    printf("Hello WOrld %lu\n", sizeof(disk_fd));

    if (disk_fd == -1) goto exit_err;

    printf("Got disk_fd flag %d\n", disk_fd);

    int offset = SUPERBLOCK_ADDR * BLOCK_SIZE;

    int seekp = lseek(disk_fd, offset, 0);
    printf("Seeked pointer to %d\n", seekp);

    int bytes_read = read(disk_fd, &sb, sizeof(sb));
    if (bytes_read == -1) goto exit_err;

    printf("Superblock.s_inodes_count ---> %d\n", sb.s_inodes_count);
    printf("Superblock.s_blocks_count ---> %d\n", sb.s_blocks_count);
    printf("Superblock.s_free_blocks_count ---> %d\n", sb.s_free_blocks_count);
    printf("Superblock.s_free_inodes_count ---> %d\n", sb.s_free_inodes_count);
    printf("Superblock.s_blocks_per_group ---> %d\n", sb.s_blocks_per_group);
    printf("Superblock.s_inodes_per_group ---> %d\n", sb.s_inodes_per_group);

	int total_blockgroups = (sb.s_blocks_count + BLOCK_SIZE + 1) / BLOCK_SIZE;
	printf("Total Blockgroups are %d\n", total_blockgroups);
	read_blockgroup_descriptor_table(disk_fd, total_blockgroups, sb.s_inodes_per_group);
exit_err:
    perror("Failed to process file");
}


