/*
* /giis/group.c This file sets the group descriptor for processing.
*
* Copyright (C) 2005. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include "giis.h"

/*
* get_group_desc() This procedure sets the group number into global structure.
*/

int get_group_desc ()
{
  int i;
  if (fs.group_offset <= RANGE)
    lseek (fd, fs.group_offset, 0);     /* fd is global */
  else
    lseek64 (fd, fs.group_offset, 0);
  i = read (fd, igd.buffer, GROUP_DESC_SIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  fs.block_bitmap = igd.gd.bg_block_bitmap;
  fs.inode_bitmap = igd.gd.bg_inode_bitmap;
  fs.inode_table = igd.gd.bg_inode_table;
  fs.free_blocks_count = igd.gd.bg_free_blocks_count;
  fs.free_inodes_count = igd.gd.bg_free_inodes_count;
  return 1;
}

/*
* eye_on_gd() : Check Group descriptor of the block.
*/
int eye_on_gd ()
{
  int i;
  /* Check all */
  gd++;
  fs.group_number = 0;
  while (fs.group_number < fs.groups_count)
  {
    fs.group_offset = fs.first_group_desc + fs.group_number * GROUP_DESC_SIZE;
    i = get_group_desc ();
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    if ((fs.block_number > fs.block_bitmap)
        && (fs.block_number < (fs.block_bitmap + fs.blocks_per_group)))
      return 1;
    else
      fs.group_number++;
  }
  return -1;
}
