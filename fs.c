/*
 * Implementation of a Unix-like file system.
*/
#include "util.h"
#include "common.h"
#include "block.h"
#include "fs.h"

#ifdef FAKE
#include <stdio.h>
#define ERROR_MSG(m) printf m;
#else
#define ERROR_MSG(m)
#endif
#define MAGIC 4275476
#define SUPER_LOC 1
#define SUPER_COPY_LOC FS_SIZE - 1
#define MAX_INODES 256



#define FS_SIZE_IN_BYTES (FS_SIZE*SECTOR_SIZE)//2048*512
#define NUM_OF_BLOCKS (FS_SIZE_IN_BYTES/BLOCK_SIZE)


int work_dir;
#define ROOT_INODE_N 0
#define MAX_FD_NUM 64
#define OK 1
#define ENTRIES_PER_BLOCK (BLOCK_SIZE/sizeof(dir_t))
#define POINTERS_PER_BLOCK (BLOCK_SIZE/sizeof(int))

sb_t * superblock;
char sb_buffer[BLOCK_SIZE];

inode_t * inode_read(int inode_index,char * block_buffer);
void inode_free(int inode_number);
int remove_inode(int work_dir,int inode_number);
//copy a string from t to s
char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
	;
  return os;
}
unsigned int
mystrlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
	;
  return n;
}
int strcmp(char *s,char *t){
	while(0 != (*s == *t) && (0 != *s))
		{s++;t++;}
	return *s - *t;
}
//---------------------------------------------------------------------------------------
//some useful macros for common operations
#define CEILING_DIV(m,n) ((m-1)/n)+1
#define INODES_PER_BLOCK ((BLOCK_SIZE)/sizeof(inode_t))
//"bytemap"
#define BITS_PER_BYTE 1
fd_t file_d_table[MAX_FD_NUM];
//---------------------------------------------------------------------------------------
//START of bitmap
void bitmap_init(sb_t* sb){
	char temp[BLOCK_SIZE];
	int i;
	bzero_block(temp);
	if(sb->data_addr<=BLOCK_SIZE-1){
		temp[BLOCK_SIZE - 1] = 1;     //mark the last block to be busy
		block_write(sb->bitmap_addr+sb->n_bitmap_blocks-1,temp);//because it is used to keep copy of superblock

		temp[BLOCK_SIZE - 1] = 0;
		for(i=0;i<sb->data_addr;i++)
			temp[i] = 1;
		block_write(sb->bitmap_addr,temp);
	}
	else{
		//print_str(0, 0, "waiting to be completed ");
	}

}
//allocate the bitmap block given the date block number
int bitmap_block(int data_block_index){
	return superblock->bitmap_addr + 
	(data_block_index / BLOCK_SIZE);
}

char * map_read(int data_block_index,char *buffer){
	block_read(bitmap_block(data_block_index),buffer);
	return buffer + (data_block_index % BLOCK_SIZE);
}

void map_write(int data_block_index, char *buffer){
	block_write(bitmap_block(data_block_index),buffer);
}

//END of bitmap
//---------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//START of superblock


void sb_init(sb_t *sb) {
	sb->fs_size = FS_SIZE;
	sb->magic_number = MAGIC;

	sb->inode0_addr = SUPER_LOC + 1;
	sb->n_inodes = MAX_INODES;
	sb->inode_blocks = CEILING_DIV(MAX_INODES, INODES_PER_BLOCK);
	
	sb->bitmap_addr = sb->inode0_addr + sb->inode_blocks;
	sb->n_bitmap_blocks = CEILING_DIV(FS_SIZE, BLOCK_SIZE*BITS_PER_BYTE);
	
	sb->data_addr = sb->bitmap_addr + sb->n_bitmap_blocks;
	sb->n_data = FS_SIZE-sb->data_addr-1;
	/*ERROR_MSG(("size of inode struct:%d\n",sizeof(inode_t)))
	ERROR_MSG(("size of directory entry struct:%d\n",sizeof(dir_t)))
	ERROR_MSG((("size of superblock struct:%d\n"),sizeof(sb_t)))
	ERROR_MSG(("file system layout: size: %d, inode start at: %d, inode end at: %d,each block contain %d inodes, so there are %d inodes in total. bitmap start at block %d, there are %d bitmap blocks, each block stores the info about %d blocks, data blocks start at %d, there are %d data blocks. Block %d is reserved for superblock verbosity.\n",sb->fs_size,sb->inode0_addr,sb->bitmap_addr-1,INODES_PER_BLOCK,INODES_PER_BLOCK*sb->inode_blocks,sb->bitmap_addr,sb->n_bitmap_blocks,BLOCK_SIZE,sb->data_addr,sb->n_data,FS_SIZE-1))
*/}

//END of superblock
//----------------------------------------------------------------------------------------------
 
//----------------------------------------------------------------------------------------
//START of data blocks
void data_read(int block_number, char *block_buffer) {
	block_read(superblock->data_addr + block_number, block_buffer);
}
int balloc(){
	int i, j;
	char block_buffer[BLOCK_SIZE];
	//search for free block
	for (i = 0; i < superblock->n_bitmap_blocks; i++) 
	{
		block_read(superblock->bitmap_addr + i, block_buffer);
		for (j = 0; j < BLOCK_SIZE; j++) 
		{
			if (block_buffer[j] == 0)
			 {
				// Mark block as used on disk
				block_buffer[j] = 1;
				block_write(superblock->bitmap_addr + i, block_buffer);
				// Return block number of newly allocated block
				return (i * BLOCK_SIZE) + j;
			}
		}
	}

	// No free blocks found
	return ERROR;
}   
void data_write(int block_number, char *block_buffer) {
	block_write(superblock->data_addr + block_number, block_buffer);
}

void block_free(int data_block_index) {
	//no need to clear the block to all 0s
	//just set the bitmap
	char *map_entry;
	char block_buffer[BLOCK_SIZE];
	map_entry = map_read(data_block_index, block_buffer);
	*map_entry = 0; // set this byte to 0
	map_write(data_block_index, block_buffer);    
}
//END of data blocks
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
//START of directory
//insert source file into dest directory
//with file name
int insert_entry_into_dir(int source_inode_number,int dest_inode_number,char *file_name){
	inode_t *directory_inode;
	char inode_buffer[BLOCK_SIZE]; 
	directory_inode = inode_read(dest_inode_number,inode_buffer);
	//must be a directory, or it cannot contain any files
	if(directory_inode->type!=DIRECTORY)
		return ERROR;
	int current_entries;
	current_entries = directory_inode->file_size / sizeof(dir_t);

	/* if too many entries in this directory */
	int which_block;//which block
	int which_entry;//in a block
	which_block = current_entries / ENTRIES_PER_BLOCK;
	which_entry = current_entries % ENTRIES_PER_BLOCK;   
	if(which_block >= N_DIRECT_BLOCK + N_INDIRECT_BLOCK*POINTERS_PER_BLOCK){
		//printf("too many entries, waiting to be completed\n");
		return ERROR;
	}
	if(which_block >= directory_inode->n_blocks){
		int new_block;
		new_block = balloc();
		//ERROR_MSG(("%d\n",new_block))
		if(new_block == ERROR){
			//ASSERT(0);
			return ERROR;
		}
		if(which_block < N_DIRECT_BLOCK){
			directory_inode->dir_blk_addr[which_block] = new_block;
		}
		else{
			int j;
			int indirect_block;
			int indirect_index;
			int *po;
			char pointer_buffer[BLOCK_SIZE];
			indirect_block = (which_block - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK;
			indirect_index = (which_block - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK;
			if(indirect_index == 0){
				//use j as  an indirect pointer block
				directory_inode->ind_blk_addr[indirect_block] = new_block; 
				block_read(new_block, pointer_buffer);
				po = (int *)pointer_buffer;
				//allocate a new block
				po[0] = balloc();
				if(po[0]==ERROR){
					//NO free blocks
					//do i need to free new_block?
				return ERROR;
				}
				block_write(new_block,pointer_buffer);
			}
			else{
				//allocate a block
				j = directory_inode->ind_blk_addr[indirect_block];
				block_read(j, pointer_buffer);
				po = (int *)pointer_buffer;
				po[indirect_index] = balloc();
				if(po[indirect_index] == ERROR){
					return ERROR;
				}
				block_write(j,pointer_buffer);
			}
		}
		directory_inode->n_blocks++;
		// ASSERT(0);
	}
	int blk_n;
	char data_buffer[BLOCK_SIZE];
	blk_n = find_block(directory_inode, which_block);
	block_read(blk_n, data_buffer); //read this block into data_buffer
	dir_t * dir_struct_ptr;
	dir_struct_ptr = (dir_t *)data_buffer;
	dir_struct_ptr[which_entry].inode = source_inode_number;

	if( mystrlen(file_name) >= MAX_FILE_NAME){
		//in other words, only support MAX_FILE_NAME-1 chars
		return ERROR;
	}

	strcpy(dir_struct_ptr[which_entry].name, file_name);
	block_write(blk_n, data_buffer);//write back
	directory_inode->file_size += sizeof(dir_t);
	inode_write(dest_inode_number,inode_buffer);
//	find_file_inode_in_dir(dest_inode_number,file_name);
	return 0;      
}

int find_file_inode_in_dir(int directory_inode,char *file_name){
	if(!file_name||strlen(file_name)>=MAX_FILE_NAME)//null ptr
		return ERROR;
	if(directory_inode < 0 || directory_inode > MAX_INODES - 1)
		return ERROR;//wrong directory inode

	char block_buffer[BLOCK_SIZE];
	inode_t * inode_ptr;
	inode_ptr = inode_read(directory_inode,block_buffer);
	if(inode_ptr->type != DIRECTORY)
		return ERROR;

	int current_entries;
	current_entries = inode_ptr->file_size / sizeof(dir_t);
//	printf("wanna find %s,%d entries in directory %d\n",file_name, current_entries,directory_inode);
	int how_many_blocks;
	int last_entry_last_block;
	how_many_blocks = current_entries / ENTRIES_PER_BLOCK;
	last_entry_last_block = current_entries % ENTRIES_PER_BLOCK;   
	
	int i,j;
	dir_t * dir_struct_ptr;
	int temp;
	char data_buffer[BLOCK_SIZE];
	for(i = 0;i <= how_many_blocks; i++)
	{//TODO
		temp = find_block(inode_ptr,i);
		block_read(temp,data_buffer);
		//block_read(inode_ptr->dir_blk_addr[i],data_buffer);
		dir_struct_ptr = (dir_t *)data_buffer;
		if(i == how_many_blocks )
		{
			//final block
			for(j = 0; j < last_entry_last_block; j++)
			{
				if(strcmp(file_name,dir_struct_ptr[j].name) == 0)
				{
					return dir_struct_ptr[j].inode;
				}
			}
		}
		else
		{
			//scan all entries
			for(j = 0; j < ENTRIES_PER_BLOCK; j++)
			{
				if(strcmp(file_name,dir_struct_ptr[j].name) == 0)
				{
					return dir_struct_ptr[j].inode;
				}
			}
		}
	}
	return ERROR;
}

//END of directory
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
//START of inode
int inode_in_which_block(int inode_index){
	return superblock->inode0_addr+inode_index/INODES_PER_BLOCK;
}
void inode_write(int inode_index,char * block_buffer){
	int block_number;
	block_number = inode_in_which_block(inode_index);
	block_write(block_number,block_buffer);
}

void inode_init(inode_t *inode, int type) {
	inode->type = type;
	inode->link_count = 1;
	inode->descriptor_count = 0;
	inode->file_size = 0;
	//make sure those pointers don't point to sth. wrong
	bzero((char *)inode->dir_blk_addr, sizeof(inode->dir_blk_addr));
	bzero((char *)inode->ind_blk_addr, sizeof(inode->ind_blk_addr));
	inode->n_blocks = 0;
}
//usage: read the block which contains the inode wanted into the given buffer address
//return the pointer to the inode
//so the caller needs basically 2 pointers to manage the inode.
inode_t * inode_read(int inode_index,char * block_buffer){
	int block_number;
	block_number = inode_in_which_block(inode_index);
	block_read(block_number,block_buffer);
	inode_t * inode;
	inode = (inode_t *)block_buffer;
	int offset;
	offset = inode_index % INODES_PER_BLOCK;
	return inode + offset;
}
void inode_free(int inode_number){
	//TODO:is there a need to clear sth else
	//such as link count and so on
	inode_t * inode;
	char inode_buffer[BLOCK_SIZE];
	inode = inode_read(inode_number,inode_buffer);
	int i;
	int temp;
	for(i = 0; i < inode->n_blocks; i++){
		//free all data blocks
		temp = find_block(inode,i);
		block_free(temp);
	}
	for(i = N_DIRECT_BLOCK; i < inode->n_blocks; i++){
		//then free the pointer blocks
		if((i - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK == 0){
			temp = (i - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK;
			block_free(inode->ind_blk_addr[temp]);
		}
	}
	inode->type = FREE_INODE;
	inode->file_size = 0;
	inode->n_blocks = 0;
	inode_write(inode_number,inode_buffer);
}
int inode_alloc(int type){
	//look for a free inode and return the inode number
	int block_number;
	int inode_number_in_block;
	char data_buffer[BLOCK_SIZE];
	int j;
	inode_t * inode_ptr;
	for(block_number = superblock->inode0_addr; 
		block_number < superblock->bitmap_addr; 
		block_number++)
	{
		block_read(block_number,data_buffer);
		inode_ptr = (inode_t *)data_buffer;
		
			//scan all entries
		for(j = 0; j < INODES_PER_BLOCK; j++)
		{
			if(inode_ptr[j].type == 0)
			{
				inode_init(&inode_ptr[j],type);
				inode_ptr[j].inode_number =
				(block_number - superblock->inode0_addr)*INODES_PER_BLOCK
				+ j;
				block_write(block_number,data_buffer);
				return inode_ptr[j].inode_number;
			}
		}
				
	}
}
//END of inode
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//START of fd
int open_in_fd(int inode_number,int mode){
	int i;
	for(i=0; i<MAX_FD_NUM;i++){
		if(file_d_table[i].used == FALSE){
			file_d_table[i].used = TRUE;
			file_d_table[i].offset = 0;
			file_d_table[i].inode_number = inode_number;
			file_d_table[i].access_mode = mode;
			return i;
		}
	}
	return ERROR;
}
//END of fd
//----------------------------------------------------------------------------------------------

void 
fs_init( void) {
	block_init();
	/* More code HERE */
	block_read(SUPER_LOC, sb_buffer);
	superblock = (sb_t * ) sb_buffer;

	if(superblock->magic_number != MAGIC){
		fs_mkfs();
	}
	else{
		work_dir = ROOT_INODE_N;
		bzero((char *)file_d_table,sizeof(fd_t));
	}

}

int
fs_mkfs( void) {
	char block_buffer[BLOCK_SIZE];
	int i;
	//zero out all disk sectors(file system blocks, more appropriately)
	bzero_block(block_buffer);
	for(i=0;i<FS_SIZE;i++){
		block_write(i,block_buffer);
	}
	
	//super block 
	bzero_block(sb_buffer);
	superblock = (sb_t *)sb_buffer;
	sb_init(superblock);
	//writing to disk:both original and copy
	block_write(SUPER_LOC,sb_buffer);
	block_write(SUPER_COPY_LOC,sb_buffer);

	//initialize bit(byte)map
	bitmap_init(superblock);

	//root dir management
	//make the inode for root directory
	inode_t * inode;
	inode = inode_read(ROOT_INODE_N,block_buffer);
	inode_init(inode, DIRECTORY);
	//write to disk
	inode_write(ROOT_INODE_N,block_buffer);

	int result;
	result = insert_entry_into_dir(ROOT_INODE_N,ROOT_INODE_N,".");

	if(result == ERROR){
		inode_free(ROOT_INODE_N);
		return ERROR;
	}
	result = insert_entry_into_dir(ROOT_INODE_N,ROOT_INODE_N,"..");

	if(result == ERROR){
		inode_free(ROOT_INODE_N);
		return ERROR;
	}
	work_dir = ROOT_INODE_N;
	bzero((char *)file_d_table,sizeof(file_d_table));

	return 0;
}

int 
fs_open( char *fileName, int flags) {
	if( !fileName)// if the addr given is invalid
		return ERROR;
	if(flags!=FS_O_RDONLY && flags!=FS_O_WRONLY && flags!=FS_O_RDWR){
//			ERROR_MSG(("out of range\n"))
		return ERROR;
	}
	int open_inode_number;
	open_inode_number = find_file_inode_in_dir(work_dir,fileName);
	int new_file_created = 0;
	if(open_inode_number == ERROR)
	{
		if(flags == FS_O_RDONLY)
			return ERROR;
		open_inode_number = inode_alloc(FILE_TYPE);
		if(open_inode_number == ERROR)
			return ERROR;
		//insert new inode into current dir
		int result;
		result = insert_entry_into_dir(open_inode_number,work_dir,fileName);
		if(result == ERROR)
		{
			inode_free(open_inode_number);
			return ERROR;
		}    
		new_file_created = 1;
	}
	inode_t * inode;
	char inode_buffer[BLOCK_SIZE];
	inode = inode_read(open_inode_number,inode_buffer);
	if(inode->type == DIRECTORY && flags != FS_O_RDONLY){
		//open a directory with write mode
		return ERROR;
	}

	//file descriptor
	int fd;
	fd = open_in_fd(open_inode_number,flags);
	if(fd == ERROR){
		//no file descriptor available

		if(new_file_created){
			inode_free(open_inode_number);
		}
		return ERROR;
	}
	inode->descriptor_count++;
	inode_write(open_inode_number,inode_buffer);
	return fd;
}
/*
 fs_close() closes a file descriptor, so that it no longer refers to any
 file and may be reused.  It returns zero on success, and -1 on failure.
 If the descriptor was the last reference to a file which has been 
 removed using unlink, the file is deleted.
*/
int 
fs_close( int fd) {
	if(file_d_table[fd].used == FALSE)
		return ERROR;
	inode_t * inode;
	char inode_buffer[BLOCK_SIZE];
	inode = inode_read(file_d_table[fd].inode_number, inode_buffer);
	inode->descriptor_count--;
	file_d_table[fd].used = FALSE;
	if(inode->descriptor_count == 0&&inode->link_count == 0){
		inode_free(file_d_table[fd].inode_number);
	}
	else{
		inode_write(file_d_table[fd].inode_number, inode_buffer);
	}
	return 0;
}
/*
fs_read() attempts to read up to count bytes from file descriptor
fd into the buffer starting at buf.  If count is zero, fs_read()
returns zero and has no other results.  On success, the number 
of bytes successfully read is returned, and the file position 
is advanced by this number.  It is not an error if this number 
is smaller than the number of bytes requested; this may happen 
for example because fewer bytes are actually available right now.
On error, -1 is returned. In the error case it is left unspecified
whether the file position changed.
*/
int 
fs_read( int fd, char *buf, int count) {
	if(file_d_table[fd].access_mode == FS_O_WRONLY)
		return ERROR;
	if(file_d_table[fd].used==FALSE)
		return ERROR;
	if(count == 0)
		return 0;
	int inode_number;
	inode_number = file_d_table[fd].inode_number;
	char inode_buffer[BLOCK_SIZE];
	inode_t *  inode_ptr;
	inode_ptr = inode_read(inode_number, inode_buffer);
	int offset;
	offset = file_d_table[fd].offset;
	if(count < 0){
//		ERROR_MSG(("negative count\n"))
		return ERROR;
	}
	if(offset+count > inode_ptr->file_size){
		//only read till end of file
		count = inode_ptr->file_size - offset;
	}
	int si;//start block
	si = offset / BLOCK_SIZE;
	int sj;//start in-block-addr
	sj = offset % BLOCK_SIZE;
	int ti;//end block
	ti = (offset + count) / BLOCK_SIZE;
	int tj;//end in-block-addr
	tj = (offset + count) % BLOCK_SIZE;
	char data_buffer[BLOCK_SIZE];
	int i;
	int j;
	int k;
	int temp;
	if(ti - si == 0){
		//start and end within 1 block
		temp = find_block(inode_ptr, si);
		block_read(temp, data_buffer);
		//block_read(inode_ptr->dir_blk_addr[si],data_buffer);
		i = 0;
		while(i < tj - sj){
			//copy from data_buffer into buf
			buf[i] = data_buffer[i+sj];
			i++;
		}
		buf[i] = 0;//no need to add \0? 
	}
	else if(ti - si == 1){
		//start with a block
		//end with the next block
		temp = find_block(inode_ptr, si);
		block_read(temp, data_buffer);
		i = 0;
		while(i < BLOCK_SIZE - sj){
			buf[i] = data_buffer[i+sj];
			i++;
		}
		temp = find_block(inode_ptr, ti);
		block_read(temp, data_buffer);
		//block_read(inode_ptr->dir_blk_addr[ti],data_buffer);
		j = 0;
		while(j < tj){
		   buf[i++] = data_buffer[j++];
		}
		buf[i] = 0;
	}
	else{
		//start with last few bytes
		//block_read(inode_ptr->dir_blk_addr[si],data_buffer);
		temp = find_block(inode_ptr, si);
		block_read(temp, data_buffer);
		i = 0;
		while(i < BLOCK_SIZE - sj){
			buf[i] = data_buffer[i+sj];
			i++;
		}
		//then continue copying whole blocks
		//2,3,...n-1
		for(j = si + 1; j < ti; j++){
			temp = find_block(inode_ptr, j);
			block_read(temp, data_buffer);
			//block_read(inode_ptr->dir_blk_addr[j],data_buffer);
			k = 0;
			while(k<BLOCK_SIZE){
				buf[i++] = data_buffer[k++];
			}
		}
		//end with block n
		temp = find_block(inode_ptr, ti);
		block_read(temp, data_buffer);
		//block_read(inode_ptr->dir_blk_addr[ti],data_buffer);
		j = 0;
		while(j < tj){
		   buf[i++] = data_buffer[j++];
		}
		buf[i] = 0;        
	}
	file_d_table[fd].offset = offset + count;
	return count;
}
/*
fs_write() writes up to count bytes to the file referenced by the
file descriptor fd from the buffer starting at buf.

On success, the number of bytes written from buf are returned 
(a number less than count can be returned), and the file position
 is advanced by this number.  On error, -1 is returned. It is an 
 error to attempt a write at a point beyond the maximum size of a
 file. It is an error if count is greater than zero but no bytes 
 were written.

A file of size zero should not take up any data blocks.
Writing padding (see fs_lseek()) should be all or nothing.
If count is zero, 0 will be returned without causing any other 
effects. 
 */
	
int 
fs_write( int fd, char *buf, int count) {
   if(file_d_table[fd].access_mode == FS_O_RDONLY)
		return ERROR;
   if(file_d_table[fd].used == FALSE)
		return ERROR;
	if(count == 0)
		return 0;
	int inode_number;
	inode_number = file_d_table[fd].inode_number;
	char inode_buffer[BLOCK_SIZE];
	inode_t *  inode_ptr;
	inode_ptr = inode_read(inode_number, inode_buffer);
	int offset;
	offset = file_d_table[fd].offset;
	if(count < 0){
//		ERROR_MSG(("negative count\n"))
		return ERROR;
	}
	//printf("offset : %d \n",offset );
	int si;//start block
	si = offset / BLOCK_SIZE;
	int sj;//start in-block-addr
	sj = offset % BLOCK_SIZE;
	int ti;//end block
	ti = (offset + count -1) / BLOCK_SIZE;
	int tj;//end in-block-addr
	tj = (offset + count -1) % BLOCK_SIZE;
	char data_buffer[BLOCK_SIZE];
	int i;
	int j;
	int k;
	char pointer_buffer[BLOCK_SIZE];
	int * po;
	if(si + 1 > inode_ptr->n_blocks){
		//need to alloc more blocks
		for(i=si; i<=ti; i++){
			j = balloc();
			if(j == ERROR){
				return ERROR;
			}
			if(i > N_DIRECT_BLOCK - 1){
			//NEED TO USE INDIRECT BLOCK
			//if it is first time when an indirect block is not setup
			int indirect_block;
			int indirect_index;
			indirect_block = (i - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK;
			indirect_index = (i - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK;
			if(indirect_block >= N_INDIRECT_BLOCK){//too large
				return ERROR;
			}
			if(indirect_index == 0){
				//use j as  an indirect pointer block
				inode_ptr->ind_blk_addr[indirect_block] = j;
				block_read(j,pointer_buffer);
				po = (int *)pointer_buffer;
				//allocate a new block
				po[0] = balloc();
				if(po[0]==ERROR){
					//NO free blocks
					//do i need to free j?
				return ERROR;
				}
				block_write(j,pointer_buffer);
			}
			else{
				//allocate a block
				j = inode_ptr->ind_blk_addr[indirect_block];
				block_read(j, pointer_buffer);
				po = (int *)pointer_buffer;
				po[indirect_index] = balloc();
				if(po[indirect_index] == ERROR){
					return ERROR;
				}
				block_write(j,pointer_buffer);
			}
			}
			else{
			inode_ptr->dir_blk_addr[i] = j;	
			}				
		}	
		inode_ptr->n_blocks = ti + 1;
	}
	inode_ptr->file_size = (offset + count > inode_ptr->file_size)? offset + count : inode_ptr->file_size;
	inode_write(inode_number, inode_buffer);
	int temp;
	if(ti - si == 0){
		//start and end within 1 block
		j = find_block(inode_ptr,si);
		//block_read(inode_ptr->dir_blk_addr[si],data_buffer);
		block_read(j,data_buffer);
		i = 0;
		while(i <= tj - sj){
			//copy from data_buffer into buf
			data_buffer[i+sj] = buf[i]; 
			i++;
		}
		//block_write(inode_ptr->dir_blk_addr[si],data_buffer);
		block_write(j,data_buffer);
	}
	else if(ti - si == 1){
		//start with a block
		//end with the next block
		j = find_block(inode_ptr, si);
		block_read(j,data_buffer);
		//block_read(inode_ptr->dir_blk_addr[si],data_buffer);
		i = 0;
		while(i < BLOCK_SIZE - sj){
			data_buffer[i+sj] = buf[i]; 
			i++;
		}
		//block_write(inode_ptr->dir_blk_addr[si],data_buffer);  
		block_write(j,data_buffer);	
		k = find_block(inode_ptr, ti);
		block_read(k,data_buffer);
		//block_read(inode_ptr->dir_blk_addr[ti],data_buffer);
		j = 0;
		while(j <= tj){
			 data_buffer[j++] = buf[i++];
		}
		block_write(k,data_buffer);
		//block_write(inode_ptr->dir_blk_addr[ti],data_buffer);
	}
	else{
		//start with last few bytes
		j = find_block(inode_ptr,si);
		block_read(j,data_buffer);
		//block_read(inode_ptr->dir_blk_addr[si],data_buffer);
		i = 0;
		while(i < BLOCK_SIZE - sj){
			data_buffer[i+sj] = buf[i];
			i++;
		}
		block_write(j,data_buffer);
		//block_write(inode_ptr->dir_blk_addr[si],data_buffer);
		//then continue copying whole blocks
		//2,3,...n-1
		for(j = si + 1; j < ti; j++){
			temp = find_block(inode_ptr,j);
			block_read(temp,data_buffer);
			//block_read(inode_ptr->dir_blk_addr[j],data_buffer);
			k = 0;
			while(k < BLOCK_SIZE){
				data_buffer[k++] = buf[i++];
			}
			block_write(temp,data_buffer);
		}
		//end with block n
		temp = find_block(inode_ptr,ti);
		block_read(temp,data_buffer);		
		//block_read(inode_ptr->dir_blk_addr[ti],data_buffer);
		j = 0;
		while(j <= tj){
		   data_buffer[j++] = buf[i++];
		}
		block_write(temp,data_buffer);       
	}
	file_d_table[fd].offset += count;
	return count;    
}
int find_block(inode_t * ino, int ord){
	//ord -> the ord + 1 th block in inode
	if(ino->n_blocks <= ord)//offset out of current blocks
		return ERROR;
	if(ord < N_DIRECT_BLOCK)
		return ino->dir_blk_addr[ord];
	int i,j;
	char buf[BLOCK_SIZE];
	i = (ord - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK;
	j = (ord - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK;
	block_read(ino->ind_blk_addr[i],buf);
	int *po;
	po = (int *)buf;
	int block_num;
	block_num = po[j];
	return block_num;
}

int 
fs_lseek( int fd, int offset) {
	//start from 0-th byte of file
	file_d_table[fd].offset = offset;
	return offset;
	//return -1;
}
/*
fs_mkdir() attempts to create a directory named dirname.
It returns zero on success, or -1 if an error occurred.
fs_mkdir() should fail if the directory dirname already
exists.

New directories must contain "." and ".." entries. It is an
error to try to create a directory without them.
*/
int 
fs_mkdir( char *fileName) {
	//allocate a new inode
	int result;
	result = find_file_inode_in_dir(work_dir,fileName);
	if(result != ERROR)//no such file
		return ERROR;
	int open_inode_number;
	open_inode_number = inode_alloc(DIRECTORY);
	if(open_inode_number == ERROR)
	   return ERROR;
	//insert new inode into current dir
	result = insert_entry_into_dir(open_inode_number,work_dir,fileName);
	if(result == ERROR)
	{
		inode_free(open_inode_number);
		return ERROR;
	}    
/*  inode_t * inode_ptr;
	char inode_buffer[BLOCK_SIZE];
	inode_ptr = inode_read(open_inode_number, inode_buffer);
*/
	result = insert_entry_into_dir(open_inode_number,open_inode_number,".");
	if(result == ERROR){
		inode_free(open_inode_number);
		return ERROR;
	}
	result = insert_entry_into_dir(work_dir,open_inode_number,"..");
	if(result == ERROR){
		inode_free(open_inode_number);
		return ERROR;
	}   
	return 0;
}
/*
fs_rmdir() deletes a directory, which must be empty.  
On success, zero is returned; on error, -1 is returned
(e.g. attempting to delete a non-empty directory).
*/
int 
fs_rmdir( char *fileName) {
	//remove directory named fileName
	//find directory
	int rm_inode_number;
	rm_inode_number = find_file_inode_in_dir(work_dir,fileName);
	if(rm_inode_number == ERROR)
		return -1;
	if(strcmp(fileName,".") == 0 || strcmp(fileName,"..") == 0)
		//should never try to delete . or ..
		return ERROR;
	inode_t *inode;
	char inode_buffer[BLOCK_SIZE];
	inode = inode_read(rm_inode_number,inode_buffer);
	if(inode->type != DIRECTORY)
		return ERROR;
	if(inode->file_size != 2*sizeof(dir_t)){
		//dir not empty
//		ERROR_MSG(("dir not empty:%d\n entries",inode->file_size/sizeof(dir_t)))
		return ERROR;
	}
	int result;
	result = remove_inode(work_dir,rm_inode_number);
	if(result!=0)
		return ERROR;
	inode_free(rm_inode_number);
	return 0;
}
int remove_inode(int work_dir,int inode_number){
	//given a directory and the inode entry user wants to remove
	//remove this entry
	inode_t * parent_inode;
	char parent_buffer[BLOCK_SIZE];
	parent_inode = inode_read(work_dir,parent_buffer);
	//ASSERT(parent_inode->type == DIRECTORY);
	int i,j;
	int offset;
	offset = ((parent_inode->file_size - sizeof(dir_t)) % BLOCK_SIZE)/sizeof(dir_t) + 1;
	char entry_buffer[BLOCK_SIZE];
	dir_t * entry;
	char entry_buffer2[BLOCK_SIZE];
	dir_t * entry2;
	int temp;
	int temp2;
	for(i=0;i<parent_inode->n_blocks;i++){
		temp = find_block(parent_inode,i);
		/*block_read(parent_inode->dir_blk_addr[i],entry_buffer);*/
		block_read(temp,entry_buffer);
		entry = (dir_t *)entry_buffer;
		if(i==parent_inode->n_blocks - 1){
			//last block
			for(j=0;j<offset;j++){
				if(entry[j].inode == inode_number){
					if(j == offset - 1){
						//if it is exactly the last entry
						//then just remove it
						/*bzero((char *)(entry + j),sizeof(entry[j]));*/
						parent_inode->file_size -= sizeof(entry[j]);
						if(parent_inode->file_size / BLOCK_SIZE < parent_inode->n_blocks){
							//need to free a data block
							parent_inode->n_blocks--;
							/*block_free(parent_inode->dir_blk_addr[parent_inode->n_blocks]);*/
							block_free(temp);
							if(parent_inode->n_blocks > N_DIRECT_BLOCK && ((parent_inode->n_blocks - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK == 0)){
								//if just removed the last block that an indirect pointer points to
								//need to free the last indirect pointer block
								temp = (parent_inode->n_blocks - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK - 1;//-1 is for fitting into the disgusting C array subscript
								block_free(parent_inode->ind_blk_addr[temp]);
							}
							/*parent_inode->dir_blk_addr[parent_inode->n_blocks] = 0;*/
						}
					}
					else{
						//overwrite this entry with the last entry
						entry[j].inode = entry[offset - 1].inode;
						strcpy(entry[j].name, entry[offset - 1].name);
						/*bzero((char *)(entry + offset - 1),sizeof(entry[offset - 1]));*/
						//wrtie into the last block the new entries
						/*block_write(parent_inode->dir_blk_addr[i],entry_buffer);  */
						block_write(temp,entry_buffer);  
						parent_inode->file_size -= sizeof(entry[offset - 1]);
					}
					inode_write(work_dir, parent_buffer);
					return 0;
				}
			}
		}
		else{
			//full blocks
			for(j=0;j<ENTRIES_PER_BLOCK;j++){
				if(entry[j].inode == inode_number){
					//need to overwrite this entry with the last entry
					//read the last block
					temp2 = find_block(parent_inode, parent_inode->n_blocks-1);
					/*block_read(parent_inode->dir_blk_addr[parent_inode->n_blocks-1],entry_buffer2);*/
					block_read(temp2,entry_buffer2);
					entry2 = (dir_t *)entry_buffer2;
					entry[j].inode = entry2[offset - 1].inode;
					strcpy(entry[j].name, entry2[offset - 1].name);
					parent_inode->file_size -= sizeof(dir_t);
					if(parent_inode->file_size / BLOCK_SIZE < parent_inode->n_blocks){
							//need to free a data block
							parent_inode->n_blocks--;
							/*block_free(parent_inode->dir_blk_addr[parent_inode->n_blocks]);*/
							block_free(temp2);
							if(parent_inode->n_blocks > N_DIRECT_BLOCK && ((parent_inode->n_blocks - N_DIRECT_BLOCK) % POINTERS_PER_BLOCK == 0)){
								//if just removed the last block that an indirect pointer points to
								//need to free the last indirect pointer block
								temp2 = (parent_inode->n_blocks - N_DIRECT_BLOCK) / POINTERS_PER_BLOCK - 1;//-1 is for fitting into the disgusting C array subscript
								block_free(parent_inode->ind_blk_addr[temp2]);
							}
							/*parent_inode->dir_blk_addr[parent_inode->n_blocks] = 0;*/
					}
					/*block_write(parent_inode->dir_blk_addr[i],entry_buffer);  */
					block_write(temp,entry_buffer);  
					inode_write(work_dir, parent_buffer);
					return 0;
				}
			}
		}
	}
	//this should never happen
//	ERROR_MSG(("removing an entry which does not exist\n"))
	return ERROR;
}
int 
fs_cd( char *dirName) {
	//enter a directory
	if(!dirName)
		return ERROR;
	if(strcmp(dirName,".") == 0)
		return 0;
	int dir_inode;
	if(strcmp(dirName,"..") == 0){
		dir_inode = find_file_inode_in_dir(work_dir,"..");
		work_dir = dir_inode;
		return 0;
	}
	dir_inode = find_file_inode_in_dir(work_dir,dirName);
	if(dir_inode == ERROR)
		return ERROR;
	else{
		work_dir = dir_inode;
		return 0;
	}
}
/*
fs_link() creates a new link (also known as a hard link) to an 
existing file oldpath.  If newpath exists it will not be overwritten.
The new name may be used exactly as the old one for any operation; 
both names refer to the same file and it is impossible to tell which
name was the "original."

On success, zero is returned.  On error, -1 is returned.  It is an 
error to use this function on a directory.

Note that because there are no "paths" beyond the current directory, 
the parent, or a child directory, oldpath and newpath are actually 
both filenames and can only be in the same directory.
 */
int 
fs_link( char *old_fileName, char *new_fileName) {
	int inode_number;
	inode_number = find_file_inode_in_dir(work_dir,new_fileName);
	if(inode_number!=ERROR)//file name already exists
	{
//		ERROR_MSG(("file name %s already exists\n",new_fileName))
		return ERROR;
	}
	inode_number = find_file_inode_in_dir(work_dir,old_fileName);
	if(inode_number == ERROR){
		return ERROR;
	}
	inode_t * inode;
	char block[BLOCK_SIZE];
	inode = inode_read(inode_number,block);
	if(inode->type == DIRECTORY)// It is an error to use this function on a directory.
		return ERROR;
	int result;
	result = insert_entry_into_dir(inode_number, work_dir, new_fileName);
	if(result == ERROR)
		return ERROR;
	inode = inode_read(inode_number,block);
	inode->link_count++;
//	printf("inode number:%d\n",inode_number );
	inode_write(inode_number, block);
//	printf("%d counts\n",(((inode_t *)block)+inode_number)->link_count );
//	ERROR_MSG(("Successfully linked, new file name %s\n",new_fileName))
//	ERROR_MSG(("Inode No         : %d\nType             : %d\nLink Count       : 2\nSize             : 0\nBlocks allocated : 0\ndescriptor count : 1",inode_number,inode->type,inode->link_count,inode->file_size,inode->n_blocks,inode->descriptor_count))
//	find_file_inode_in_dir(work_dir,new_fileName);
	return 0;
}
/*fs_unlink() deletes a name from the file system.  If that name was the last link to a file and no process has the file open, 
the file is deleted and the space it was using is made available for reuse.
If the name was the last link to a file but it is still open under an existing file descriptor, 
then it remains in existence until the last file descriptor referencing it is closed.
On success, zero is returned.  On error, -1 is returned.  It is an error to use this function on a directory.*/

int 
fs_unlink( char *fileName) {
	int inode_number;
	inode_number = find_file_inode_in_dir(work_dir,fileName);
	if(inode_number == ERROR)
		return ERROR;
	int result;
	result = remove_inode(work_dir,inode_number);
	if(result==ERROR)
		return ERROR;
	char block[BLOCK_SIZE];
	inode_t * inode;
	inode = inode_read(inode_number,block);
	inode->link_count--;
	if(inode->link_count==0&&inode->descriptor_count==0)
	{
		inode_free(inode_number);
	}
	else{
		inode_write(inode_number,block);
	}
	return 0;
	//SHOULD NEVER REACH HERE ASSERT(0);
}

int 
fs_stat( char *fileName, fileStat *buf) {
	if(!fileName||!buf)
		return ERROR;
	int inode_number;
	inode_number = find_file_inode_in_dir(work_dir,fileName);
	if(inode_number == ERROR){
//		printf("file name %s not found in inode %d\n",fileName,work_dir);
		return ERROR;
	}
	else{
		inode_t * inode;
		char inode_buffer[BLOCK_SIZE];
		inode = inode_read(inode_number,inode_buffer);
		buf->inodeNo = inode_number;
		buf->type = inode->type;
		buf->links = inode->link_count;
		buf->descriptor_count = inode->descriptor_count;
		buf->size = inode->file_size;
		buf->numBlocks = inode->n_blocks;
		return 0;
	}
}

