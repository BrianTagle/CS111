//NAME: Brian Tagle
//EMAIL: taglebrian@gmail.com
//ID: 604907076
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <time.h>


#include "ext2_fs.h"

#define SB_OFFSET 1024
int diskFd = -1;

struct ext2_super_block super;
struct ext2_group_desc group;
struct ext2_inode inode;
//struct ext2_dir_entry dir;

int blockSize;
int inodeSize;


void print_times()
{
  char change_time[25], mod_time[25], access_time[25];
  time_t raw = inode.i_ctime;
  struct tm* tm = gmtime(&raw);
  strftime(change_time, 25 , "%m/%d/%y %H:%M:%S" , tm);

  raw = inode.i_mtime;
  tm = gmtime(&raw);
  strftime(mod_time, 25 , "%m/%d/%y %H:%M:%S" , tm);

  raw = inode.i_atime;
  tm = gmtime(&raw);
  strftime(access_time, 25 , "%m/%d/%y %H:%M:%S" , tm);

  fprintf(stdout, "%s,%s,%s," , change_time, mod_time, access_time);
}

void print_csv_block_bitmap()
{ //group.bg_block_bitmap, sb.s_blocks_count
  
  char* bitmap = (char*)malloc(blockSize);
  unsigned int start = super.s_first_data_block;
  pread(diskFd, bitmap, blockSize, (group.bg_block_bitmap)*blockSize);
  for(unsigned int i = 0; i < (super.s_blocks_count); i++)
    {
      char bit = bitmap[i];
      for(int j=0; j <8; j++) //go through each bit in the byte
	{
	  int free = 1 & bit;
	  if (free == 0)
	    {
	      fprintf(stdout, "BFREE,%d\n", start + j);
	    }
	  bit >>= 1;
	}
      start+=8;
    }
  free(bitmap);
}



void print_csv_superblock()
{
  pread(diskFd, &super, sizeof(struct ext2_super_block), SB_OFFSET);

  blockSize = 1024 << super.s_log_block_size;
  inodeSize = super.s_inode_size;
      
  fprintf(stdout, "SUPERBLOCK,"); //1. SUPERBLOCK
  fprintf(stdout, "%u,", super.s_blocks_count); //2. total number of blocks
  fprintf(stdout, "%u,", super.s_inodes_count); //3. total number of inodes
  fprintf(stdout, "%u,", blockSize); //4. block size
  fprintf(stdout, "%u,", inodeSize); //5. inode size
  fprintf(stdout, "%u,", super.s_blocks_per_group); //6. blocks per group
  fprintf(stdout, "%u,", super.s_inodes_per_group); //7. inodes per group
  fprintf(stdout, "%u\n", super.s_first_ino); //8. first non-reserved inode

}



void read_Entry( int parent_inode,  int block)
{
  struct ext2_dir_entry Entry;
  long offset = 1024 + (block-1)*blockSize;

  for( int bytes = 0; bytes < blockSize; bytes += Entry.rec_len)
    {
      
      pread(diskFd, &Entry, sizeof(struct ext2_dir_entry), offset + bytes);
      if (Entry.inode == 0) //empty
	{
	  return;
	}
      else
	{
	  
	  fprintf(stdout, "DIRENT,"); //1. DIRENT
	  fprintf(stdout, "%d,", parent_inode); //2. Inode number of owning directory
	  fprintf(stdout, "%d,", bytes); //3. byte offset of this entry within directory
	  fprintf(stdout, "%d,", Entry.inode); //4. inode number of entry in directory
	  fprintf(stdout, "%d,", Entry.rec_len); //5. length of entry
	  fprintf(stdout, "%d,", Entry.name_len); //6. length of the name of entry
	  fprintf(stdout, "'%s'\n",Entry.name); //7. name of entry
	}

	
    }
}


void read_indirect_inode(unsigned int inode_num, int num_indirection)//, int num_indirection)
{
  int b_ptrs[4096];
  int ib_ptrs[4096];
  int ib2_ptrs[4096];
  //single indirect	
  if (num_indirection == 1 )
    {
      int logical_offset = 12;
      pread(diskFd, b_ptrs, blockSize, 1024 +(inode.i_block[12]-1)*blockSize );
      int i = 0 ;
      while( i < blockSize/4)
	{
	if (b_ptrs[i] != 0)
	  {

	  fprintf(stdout, "INDIRECT,");
	  fprintf(stdout, "%d,", inode_num);
	  fprintf(stdout, "%d,", num_indirection);
	  fprintf(stdout, "%d,", logical_offset);
	  fprintf(stdout, "%d,", inode.i_block[12]);
	  fprintf(stdout, "%d\n", b_ptrs[i]);
	    
	}
	i++;
	logical_offset++;
      }

    }

  //double indirect
  if (num_indirection == 2 )
    {

      pread(diskFd, ib_ptrs, blockSize, 1024 +(inode.i_block[13]-1)*blockSize );
      int i = 0;
      int logical_offset_i = 268 ; //256+12
      while ( i < blockSize/4 )
	{
	  if (ib_ptrs[i] != 0)
	    {

	      fprintf(stdout, "INDIRECT,");
	      fprintf(stdout, "%d,", inode_num);
	      fprintf(stdout, "%d,", num_indirection);
	      fprintf(stdout, "%d,", logical_offset_i);
	      fprintf(stdout, "%d,", inode.i_block[13]);
	      fprintf(stdout, "%d\n", ib_ptrs[i]);

        

	      pread(diskFd, b_ptrs, blockSize, 1024 +(ib_ptrs[i]-1)*blockSize );

	      int j = 0;
	      int logical_offset = 268;
	      while (j < blockSize/4)
		{
		  if (b_ptrs[j] != 0)
		    {

		      fprintf(stdout, "INDIRECT,");
		      fprintf(stdout, "%d,", inode_num);
		      fprintf(stdout, "%d,", num_indirection-1);
		      fprintf(stdout, "%d,", logical_offset);
		      fprintf(stdout, "%d,", ib_ptrs[i]);
		      fprintf(stdout, "%d\n", b_ptrs[j]);
	    
		    }
		  j++;
		  logical_offset++;
		}

	    }
	  i++;
	  logical_offset_i++;
	}

    }

  //triple indirect
  if (num_indirection == 3) {


    pread(diskFd, ib2_ptrs, blockSize, 1024 +(inode.i_block[14]-1)*blockSize);

    int logical_offset_i2 = 65804;//65536+256 +12;
    int i = 0;
    while( i < blockSize/4 )
      {
	if (ib2_ptrs[i] != 0)
	  {

	    fprintf(stdout, "INDIRECT,");
	    fprintf(stdout, "%d,", inode_num);
	    fprintf(stdout, "%d,", num_indirection);
	    fprintf(stdout, "%d,", logical_offset_i2);
	    fprintf(stdout, "%d,", inode.i_block[14]);
	    fprintf(stdout, "%d\n", ib2_ptrs[i]);

	    pread(diskFd, ib_ptrs, blockSize, 1024 +(ib2_ptrs[i]-1)*blockSize);

	    int logical_offset_i = 65804;//65536+256 +12;
	    int j=0;
	    while( j < blockSize/4)
	      {
		if (ib_ptrs[j] != 0)
		  {

		    fprintf(stdout, "INDIRECT,");
		    fprintf(stdout, "%d,", inode_num);
		    fprintf(stdout, "%d,", num_indirection-1);
		    fprintf(stdout, "%d,", logical_offset_i);
		    fprintf(stdout, "%d,", ib2_ptrs[i]);
		    fprintf(stdout, "%d\n", ib_ptrs[j]);

		    pread(diskFd, b_ptrs, blockSize, 1024 +(ib_ptrs[j]-1)*blockSize);

		    int logical_offset = 65804;//65536+256 +12;
		    int k = 0;
		    while ( k < blockSize/4)
		      {
			if (b_ptrs[k] != 0)
			  {

			    fprintf(stdout, "INDIRECT,");
			    fprintf(stdout, "%d,", inode_num);
			    fprintf(stdout, "%d,", num_indirection-2);
			    fprintf(stdout, "%d,", logical_offset);
			    fprintf(stdout, "%d,", ib_ptrs[j]);
			    fprintf(stdout, "%d\n", b_ptrs[k]);
			  }
			k++;
			logical_offset++;
		      }
				        
		  }
		j++;
		logical_offset_i++;
	      }
		        
	  }
	i++;
	logical_offset_i2++;
      }
        
  }

}

void indirect_directories(unsigned int inode_num, int num_indirection)
{

  int b_ptrs[4096];
  int ib_ptrs[4096];
  int ib2_ptrs[4096];
  //single indirect
  if (num_indirection == 1 && inode.i_block[12] != 0)
    {

      pread(diskFd, b_ptrs, blockSize, 1024 +(inode.i_block[12]-1)*blockSize );
      int i = 0 ;
      while (i < blockSize/4)
	{
	  if (b_ptrs[i] != 0)
	    {	  
	      read_Entry(inode_num, b_ptrs[i]);
	    }
	  i+=1;
	}

    }

  //double indirect
  else if (num_indirection == 2 && inode.i_block[13] != 0)
    {
 
      pread(diskFd, ib_ptrs, blockSize, 1024 +(inode.i_block[13]-1)*blockSize);

      for (int i = 0; i < blockSize/4; i++)
	{
	  if (ib_ptrs[i] != 0)
	    {

	      pread(diskFd, b_ptrs, blockSize, 1024 +(ib_ptrs[i]-1)*blockSize );

	      for (int k = 0; k < blockSize/4; k++) {
	  
		if (b_ptrs[k] != 0)
		  {
		    read_Entry(inode_num, b_ptrs[k]);
		  }
	      }

	    }
	}

    }

  //triple indirect
  else if (num_indirection == 3 && inode.i_block[14] != 0)
    {
      pread(diskFd, ib2_ptrs, blockSize, 1024 +(inode.i_block[14]-1)*blockSize);

      for (int j = 0; j < blockSize/4; j++)
	{
	  if (ib2_ptrs[j] != 0)
	    {
	      pread(diskFd, ib_ptrs, blockSize, 1024 +(ib2_ptrs[j]-1)*blockSize);


	      for (int k = 0; k < blockSize/4; k++)
		{
		  if (ib_ptrs[k] != 0)
		    {
		      pread(diskFd, b_ptrs, blockSize, 1024 +(ib_ptrs[k]-1)*blockSize);


		      for (int l = 0; l < blockSize/4; l++)
			{
			  if (b_ptrs[l] != 0)
			    {
			      read_Entry(inode_num, b_ptrs[l]);
			    }
			}
				        
		    }
		}
		        
	    }
	}
        
    }

}

//group.bg_inode_bitmap, super.s_inodes_count, group.bg_inode_table
void print_csv_inodes()
{
  
  
  int num_bytes = super.s_inodes_count / 8;
  char* bitmap = (char*) malloc(num_bytes);

  unsigned int current = 1; //start from first inode

  pread(diskFd, bitmap, num_bytes, (group.bg_inode_bitmap)*blockSize);

  for (int i = 0; i < num_bytes; i++)
    {
      char bit = bitmap[i];
      for (int j = 0; j < 8; j++)
	{
	  int free= 1 & bit;
	  if (free == 0) //free inode
	    {
	      fprintf(stdout, "IFREE,%d\n", current+j);
    			        
	    }
	  else //allocated inode
	    {

	      unsigned long offset = SB_OFFSET + (group.bg_inode_table-1)*blockSize + (current+j-1) * sizeof(inode);
	      pread(diskFd, &inode, sizeof(inode), offset);
	      if (inode.i_mode == 0 || inode.i_links_count == 0)
		{
		  continue;
		}
	      char fileType = '?';
	      if(S_ISDIR(inode.i_mode)) //directory
		{
		  fileType = 'd';

		  for (int k = 0; k < 12; k++) { //"direct" directories
		    if (inode.i_block[k] != 0 && fileType == 'd') {
		      read_Entry(current+j, inode.i_block[k]);
		    }
		  }
		  indirect_directories(current+j, 1);
		  indirect_directories(current+j, 2);
		  indirect_directories(current+j, 3);
		}
	      else if(S_ISREG(inode.i_mode)) //regular
		{
		  fileType = 'f';
		}
	      else if(S_ISLNK(inode.i_mode)) //sym link
		{
		  fileType = 's';
		}

	      fprintf(stdout, "INODE,"); //1.
	      fprintf(stdout, "%d," , current+j); //2. INODE number
	      fprintf(stdout, "%c," , fileType); //3. filetype
	      fprintf(stdout, "%o," , inode.i_mode & 0xFFF); //4. mode
	      fprintf(stdout, "%d," , inode.i_uid); //5. owner
	      fprintf(stdout, "%d," , inode.i_gid); //6. group
	      fprintf(stdout, "%d," , inode.i_links_count); //7. link count
	      print_times(); //8,9,10. times	
	      fprintf(stdout, "%d," ,	inode.i_size); //11. file size
	      fprintf(stdout, "%d" , inode.i_blocks); //12. number of blocks taken up by this file
	      if(fileType != 's')
		{
		  for(unsigned int k = 0; k < 15; k++){
		    fprintf(stdout, ",%u", inode.i_block[k]);
		  }
		}
	      else if ( (fileType == 's') && (inode.i_size > 60) )
		{
		  for(unsigned int k = 0; k < 15; k++){
		    fprintf(stdout, ",%u", inode.i_block[k]);
		  }
		}
	      else
		{
		  fprintf(stdout, ",%u", inode.i_block[0]);
		}
	      fprintf(stdout,"\n");
	      
	      if(inode.i_block[12] != 0)
		{
		  read_indirect_inode( current+j,1 );
		}
	      if(inode.i_block[13] != 0)
		{
		  read_indirect_inode( current+j,2 );
		}
	      if( inode.i_block[14] != 0)
		{
		  read_indirect_inode( current+j,3 );
		}
	    }
	  bit >>= 1;

	}
      current+=8;
    }
  free(bitmap);
  
}

  



void print_csv_group() {
  pread(diskFd, &group, sizeof(struct ext2_group_desc), 1024 + sizeof(struct ext2_super_block));

  fprintf(stdout, "GROUP,0,"); //1. GROUP, 2. group num (there is only one group) 
  fprintf(stdout, "%u,", super.s_blocks_count); //3. total number of blocks
  fprintf(stdout, "%u,", super.s_inodes_count); //4. total number of inodes
  fprintf(stdout, "%u,", group.bg_free_blocks_count); //5. number of free blocks
  fprintf(stdout, "%u,",  group.bg_free_inodes_count); //6. number of free inodes
  fprintf(stdout, "%u,",  group.bg_block_bitmap); //7. block number of free block bitmap
  fprintf(stdout, "%u,",  group.bg_inode_bitmap); //8. block number of free inode bitmap
  fprintf(stdout, "%u\n", group.bg_inode_table); //9. block number of first block of inodes

}

int main(int argc, char** argv)
{
  if(argc != 2)
    {
      fprintf(stderr, "No input image provided\n");
      exit(1);
    }
  if((diskFd = open(argv[1], O_RDONLY)) == -1)
    {
      fprintf(stderr, "Unable to mount input image\n");
      exit(2);
    }
  print_csv_superblock();
  print_csv_group();
  print_csv_block_bitmap();
  print_csv_inodes();
}
