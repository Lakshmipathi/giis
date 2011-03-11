/*
* /giis/inode.c -Performs inode related opertions 
*
* Copyright (C) 2005. G.Lakshmipathi.<lakshmipathi_g@gmail.com>
*
*/

#include"giis.h"

/*
* find_inode_offset() - Computes given inodes offset.
*/

int find_inode_offset ()
{
  int i;
  fs.group_number = (fs.inode_number - 1) / fs.inodes_per_group;
  fs.index_number = (fs.inode_number - 1) % fs.inodes_per_group;
  fs.group_offset = fs.first_group_desc + fs.group_number * GROUP_DESC_SIZE;
  i = get_group_desc ();        /* Get corresponding group values */
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  fs.inode_offset =
    (unsigned long long) fs.inode_table * fs.block_size + fs.index_number * fs.inode_size;
  if (fs.inode_number == ROOT_INODE)
    fs.root_inode_offset = fs.inode_offset;
  return 1;
}

/* 
* read_inode() - All necessary informations is loaded so just go and read 
*/

int read_inode ()
{
  int i;
  if (fs.inode_offset <= RANGE)
    lseek64 (fd, fs.inode_offset, 0);
  else
    lseek64 (fd, fs.inode_offset, 0);

  i = read (fd, iin.buffer, fs.inode_size);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  if (S_ISDIR (iin.in.i_mode))
    dir = 1;

  return 1;
}

/*
* show_inode() - Display Some inode details 
*/

int show_inode ()
{
  int i;

  printf ("\nInode number : %lu", fs.inode_number);
  printf ("\nInode offset : %llu", fs.inode_offset);
  printf ("\nDelete time  : %u", iin.in.i_dtime);
  printf ("\nHard links   : %d", iin.in.i_links_count);
  printf ("\nSize         : %u", iin.in.i_size);
  printf ("\nInode type   :");

  if (iin.in.i_mode == 0)
    printf ("Free");
  if (S_ISREG (iin.in.i_mode))
    printf ("Regular file ");
  if (S_ISDIR (iin.in.i_mode))
    printf (" Directory");
  if (S_ISLNK (iin.in.i_mode))
    printf (" Symbolic Link");
  if (S_ISCHR (iin.in.i_mode))
    printf (" character device");
  if (S_ISBLK (iin.in.i_mode))
    printf (" Block device");
  for (i = 0; i < 15; i++)
    printf ("\nData blocks[%d] :%u", i, iin.in.i_block[i]);

  return JSJ_NO_RETURN;
}
