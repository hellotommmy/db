/*
 * Implementation of a Unix-like file system.
*/
#ifndef FS_INCLUDED
#define FS_INCLUDED

//number of sectors 
#define FS_SIZE 2048

#define N_INDIRECT_BLOCK 3
#define N_DIRECT_BLOCK 9
#define SUPER_BLOCK_SIZE sizeof(sb_t)
#define INODE_SIZE sizeof(inode_t)
typedef struct {
	int fs_size;
	int magic_number;

	int bitmap_addr;
	int n_bitmap_blocks;

	int n_inodes;
	int inode0_addr;
	//further info: how many blocks needed to contain all inodes?
	int inode_blocks;

	int n_data;
	int data_addr;

} sb_t;
typedef struct {//4+4+4+48+4 = 64
	char type;
	unsigned char link_count;

	unsigned char descriptor_count;
	char padding;

	int file_size;//in bytes
	int n_blocks;
	
	int dir_blk_addr[N_DIRECT_BLOCK];
	int ind_blk_addr[N_INDIRECT_BLOCK];

	int inode_number;
} inode_t;

typedef struct 
{
	int inode_number;
	int access_mode;
	int offset;
	bool_t used; //used when it is pointing to an inode
} fd_t;
#define MAX_FILE_NAME 32
#define MAX_PATH_NAME 256  // This is the maximum supported "full" path len, eg: /foo/bar/test.txt, rather than the maximum individual filename len.
#define ERROR -1
typedef struct 
{
	int inode;
	char name[MAX_FILE_NAME];
	char paddings[64-sizeof(int)-MAX_FILE_NAME];
} dir_t;
void fs_init( void);
int fs_mkfs( void);
int fs_open( char *fileName, int flags);
int fs_close( int fd);
int fs_read( int fd, char *buf, int count);
int fs_write( int fd, char *buf, int count);
int fs_lseek( int fd, int offset);
int fs_mkdir( char *fileName);
int fs_rmdir( char *fileName);
int fs_cd( char *dirName);
int fs_link( char *old_fileName, char *new_fileName);
int fs_unlink( char *fileName);
int fs_stat( char *fileName, fileStat *buf);

#define MAX_FILE_NAME 32
#define MAX_PATH_NAME 256  // This is the maximum supported "full" path len, eg: /foo/bar/test.txt, rather than the maximum individual filename len.

#endif
