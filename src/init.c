/* 
* /giis/init.c This file  Initialize file system informations.
*
* Copyright (C)  2005. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include"giis.h"

/*
* init() - Just invokes other init functions
*/

int init ()
{
int temp_fd,i;
char buf[100]="";
  if (search4fs () == -1)
  {
    printf ("\nFile system Not Detected....Aborting\n");

    return -1;
  }
  else
    printf ("found.\n\tDetected file system %s ", device_name);

  
  printf ("\n\nSearching for Superblock....");


  if (search4super_block () == -1)
  {
    printf ("\nCan't  find Super block...Aborting\n");
    exit(0);
  }
  else
    printf ("found.\n\tSuper block is at %lu.\n\nGathering informations.......",
            fs.super_block_offset);


  set_file_system ();

  if (search4group_desc () == -1)
  {
    printf ("\nUnable to initialize Group Descriptor...Aborting\n");
    return -1;
  }
  else
    printf ("\n\tGroup Descriptor found.");

  if (init_files () == -1)
  {
    printf ("\n\n Files not initialized Aborting....");
    return -1;
  }
  else
    printf ("\n\nAll files are Initialized...");

  return 1;
}

/*
* search4fs() will search for device name in /etc/mtab file. 
*/

int search4fs ()
{
  int fp, i = 0, c = 1;
  char a;
  fp = open ("/etc/mtab", 0);
  if (fp == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  do
  {
    i = read (fp, &a, 1);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  while (a != '/');
  c = 0;
  while (a != ' ')
  {
    device_name[c] = a;
    c++;
    i = read (fp, &a, 1);
  }
  device_name[c] = '\0';
 // i = open (device_name, 0);
//Verify whether it's a valid device 
//Check giis.conf file for device name
  if(access(device_name,F_OK)!=0){
	printf("\n\tAuto-Detect:%s:Device not found ",device_name);
	printf("\n\tReading Configuration file /etc/giis.conf\n");
  	read_config_file("$DEVICE = ");
	
	
		if(access(device_name,F_OK)!=0){
		printf("\n\tSearch Device : Both Auto-Detect and Config file Failed..Aborting ");
		exit(0);
		}
		
	}
	//get auto update time to auto_time variable.
	read_config_file("$TIME = ");
	//convert into int
	auto_time2=atoi(auto_time);
	//convert into seconds
	auto_time2=auto_time2*60;

	//get dir max depth into s_level_value variable.
	read_config_file("$LEVEL_VALUE = ");
	//convert into int
	level_value=atoi(s_level_value);
	
	return JSJ_NO_RETURN;
}

/*
* search4super_block() - Search for super block in the file system. 
*/

int search4super_block ()
{
  int i, offset = DEFAULT_OFFSET_OF_SUPER_BLOCK;

  fd = open (device_name, ACCESS);
  if (fd == -1)
  {
    perror ("");
    printf ("Error Number:%d ");
    printf("\n Please make sure you edited /etc/giis.conf with correct device name");
    return -1;
  }
  /*
     Now we opened the devide say /dev/hda7 Then search for superblock on its
     offset which is normally at 1024
   */
  while (offset < 4096)
  {
    lseek (fd, offset, 0);
    i = read (fd, isb.buffer, SUPER_BLOCK_SIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    if (isb.sb.s_magic == 61267)
    {
      fs.super_block_offset = offset;
      return 1;
    }
    offset += offset;
  }
  return -1;
}

/*
* set_file_system()- Initialize  file system details.
*/


void set_file_system ()
{
  int i, pow;
  fs.inodes_per_group = isb.sb.s_inodes_per_group;
  fs.blocks_per_group = isb.sb.s_blocks_per_group;
  fs.inodes_count = isb.sb.s_inodes_count;
  fs.blocks_count = isb.sb.s_blocks_count;
  for (i = 1, pow = 1; i <= isb.sb.s_log_block_size; i++)
    pow = pow * 2;
  fs.block_size = 1024 * pow;
  fs.inode_size = isb.sb.s_inode_size;
  fs.inodes_per_block = fs.block_size / fs.inode_size;
  fs.inode_table_size = fs.inodes_per_group / fs.inodes_per_block;
  fs.first_data_block = isb.sb.s_first_data_block;
  fs.first_group_desc = fs.block_size;
  fs.groups_count =
    (fs.blocks_count - fs.first_data_block + fs.blocks_per_group) / fs.blocks_per_group;
  printf (" recorded...proceed...");
}

/* 
* search4group_desc() - Initialize Group descriptor details. 
*/

int search4group_desc ()
{
  int i;
  lseek (fd, fs.first_group_desc, 0);
  i = read (fd, igd.buffer, GROUP_DESC_SIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }

  return 0;
}

/*
* init_files() - Create or open files or directories 
*/

int init_files ()
{
  int fp;


  /* Create Directory. one can use O_CREAT | O_EXCL but i prefer... */

  fp = open (INSTALL_DIR1, 2);
  if (errno == ENOENT)
  {
    fp = mkdir (INSTALL_DIR1, 040777);
    if (fp == -1)
    {                           /* This error is can not be  ENOENT :) */
      perror ("creat:");
      return -1;
    }
    else
    {
      printf ("\n\n Creating files....");
      printf ("\n %s created...", INSTALL_DIR1);
    }

  }
 // close (fp);

  /* Create Directory */

  fp = open (INSTALL_DIR2, 2);
  if (errno == ENOENT)
  {
    fp = mkdir (INSTALL_DIR2, 040777);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", INSTALL_DIR2);

  }
//  close (fp);

  /* Create Directory */

  fp = open (INSTALL_DIR3, 2);
  if (errno == ENOENT)
  {
    fp = mkdir (INSTALL_DIR3, 040777);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", INSTALL_DIR3);

  }
//  close (fp);


  /* Create or open  DIR_INFO_FILE */

  fp = open (DIR_INFO_FILE, 2);
  if (errno == ENOENT)
  {
    fp = creat (DIR_INFO_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", DIR_INFO_FILE);

  }
  //close (fp);

  /* Create or open  FILE_INFO_FILE */

  fp = open (FILE_INFO_FILE, 2);
  if (errno == ENOENT)
  {
    fp = creat (FILE_INFO_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", FILE_INFO_FILE);

  }
  //close (fp);

  /* Same ...  */

  fp = open (SIND_INFO_FILE, 2);
  if (errno == ENOENT)
  {
    fp = creat (SIND_INFO_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", SIND_INFO_FILE);

  }
  //close (fp);

  /* Same again.. */

  fp = open (DIND_INFO_FILE, 2);
  if (errno == ENOENT)
  {
    fp = creat (DIND_INFO_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", DIND_INFO_FILE);
  }
  //close (fp);

  /* Cut and Pasted  again except file name ;-) */

  fp = open (SAMPLE_DATA_FILE, 2);
  if (errno == ENOENT)
  {
    fp = creat (SAMPLE_DATA_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", SAMPLE_DATA_FILE);
  }
  //close (fp);
  //For Unrecoverable Files
  fp = open (INSTALL_DIR4, 2);
  if (errno == ENOENT)
  {
    fp = mkdir (INSTALL_DIR4, 040777);
    if (fp == -1)
    {                           /* This error is can not be  ENOENT :) */
      perror ("creat:");
      return -1;
    }
    else
    {
      printf ("\n %s created...", INSTALL_DIR4);
    }

  }
  //close (fp);

/* Create or open  GIIS_RERE_FILE */

  fp = open (GIIS_RERE_FILE, O_CREAT | O_RDWR | O_TRUNC);
  if (errno == ENOENT)
  {
    fp = creat (GIIS_RERE_FILE, 0700);
    if (fp == -1)
    {
      perror ("creat:");
      return -1;
    }
    else
      printf ("\n %s created...", GIIS_RERE_FILE);

  }

  return 1;
}

/*
* fs_display()	: Display file system details 
*/
int fs_display ()
{
  printf ("\n\n");
  printf ("\n\t\t File System details of Your System ");
  printf ("\n\t\t -----------------------------------\n");
  printf ("\n\t Root Device       : %s ", device_name);
  printf ("\n\t Magic Number      : %x ", isb.sb.s_magic);
  printf ("\n\t Root Inode offset : %llu ", fs.root_inode_offset);
  printf ("\n\t Total Inodes      : %llu ", fs.inodes_count);
  printf ("\n\t Total Blocks      : %llu ", fs.blocks_count);
  printf ("\n\t Inodes per Group  : %lu ", fs.inodes_per_group);
  printf ("\n\t Blocks per Group  : %lu ", fs.blocks_per_group);
  printf ("\n\t Block size        : %u ", fs.block_size);
  printf ("\n\t Inodes per Block  : %u ", fs.inodes_per_block);
  printf ("\n\t Inode Table size  : %u ", fs.inode_table_size);
  printf ("\n\t Groups count      : %u ", fs.groups_count);
  return 1;
}
