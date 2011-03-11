/*
* /giis/searchnupdate.c - This file performs search and update operations on files.
*
* Copyright (C) 2007 G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include"giis.h"

/*
* search4dir() - This function is used to record all entries of Directory into 
* corresponding  info_files.
* Modification for giis2:
* First we record directories and avoid any duplication with the help avoid_dups()
* After recording all directories then record files by setting install_file.And 
* make a recursive call to search4dir().
* Modification for giis2.1:
* Use of update_dir variable
*/


int search4dir ()
{
  int i, fp, level_count = 0;

	if(install == 0){

	//get dir.list from config file
	read_config_file2();
	read_conf_file_inum();
	
	}
   while (level_count < level_value)
  {
   
    fp = open (DIR_INFO_FILE, 0);
    if (fp == -1)
    {
      perror ("open");
      return -1;
    }

    i = read (fp, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }


    while (i > 0)
    {
      if (giis_d.info.search_flag == 1)
      {
	
        fs.inode_number = giis_d.info.inode_number;
        i = find_inode_offset ();
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        i = read_inode ();
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        i = set_content_offset ();
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        if (dir)
        {

          i = read_dir ();
          if (i == -1)
          {
            perror ("");
            printf ("Error Number:%d", errno);
            return -1;
          }
          i = show_dir ();
          if (i == -1)
          {
            perror ("");
            printf ("Error Number:%d", errno);
            return -1;
          }
        }

      }
      i = read (fp, giis_d.buffer, GIIS_DSIZE);
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
    }
	if(new_dir_added){
	install == 1;
	install_file == 0;
	}
    if ((install == 1 && install_file == 0))
    {
      i = update_dir_info_file (level_count);
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      i = avoid_dups ();
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
    }
    /* First update all directories above and now update files */
    if (update_dir == 1)
    {
      close (fp);
      update_dir = 0;
      i = search4dir ();        /* Update files */
      if (i == 444)
      {
        close (fp);
	//Set device name :
	      search4fs();

        get_sample_data ();
        printf ("\n\n\t\t giis updated...");
        return 1;

      }


    }

    if (install_file == 1 || update_file == 1)  /* Because we want record files only once . */
      return 444;

    level_count++;
    close (fp);
  }



  /* During installation we need to record fiels so now store 
     only those files from DIR file  */

  if (install == 1 && install_file == 0)
  {
    install_file = 1;
    search4dir ();              /* Recursive */
  }

  return 1;
}


/* 
* update_dir_info_file()-After entries  are recorded directories search_flags are set.
* This is called during installation.
* This update_dir_info_file() ironically will not be called during updates.
*/

int update_dir_info_file (int level_count)
{
  int fp, i, flag, count = 0;


  fp = open (DIR_INFO_FILE, 2);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }
  i = read (fp, giis_d.buffer, GIIS_DSIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }

  /* Set flags of  newly found entries */

  while (i > 0)
  {
    if ((giis_d.info.search_flag == 0) && (giis_d.info.parent_inode_number != 2))
    {
      flag = get_parent_inode_flag (giis_d.info.parent_inode_number);
      if (flag == -1)
        break;
      if (flag == 1)
      {                         /* Parent is set so set sub-directory flag */
        lseek (fp, count * GIIS_DSIZE, 0);
        i = read (fp, giis_d.buffer, GIIS_DSIZE);
        if (i != GIIS_DSIZE)
        {
          perror ("read");
          return -1;
        }
        giis_d.info.search_flag = 1;
        lseek (fp, -GIIS_DSIZE, 1);
        i = write (fp, giis_d.buffer, GIIS_DSIZE);
        if (i != GIIS_DSIZE)
        {
          perror ("write");
          return -1;
        }

      }
    }
    i = read (fp, giis_d.buffer, GIIS_DSIZE);
    /* if end of file just quits loop */
    count++;

  }

  printf ("\n\t Files at  Directory Level %d are updated......", level_count + 1);
  close (fp);
  return 1;
}

/*
* get_parent_inode_flag() is a helper function to update_dir_file_info().
* It returns parent inode flag.
*/

int get_parent_inode_flag (unsigned long parent)
{
  int fp, i;

  fp = open (DIR_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }

  i = read (fp, giis_d.buffer, GIIS_DSIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }

  while (i > 0)
  {
    if (giis_d.info.inode_number == parent)
    {
      close (fp);
      return 1;
    }
    i = read (fp, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }

  }

  close (fp);
  return -1;
}

/*
* search4sequence12() 
* In giis1 holes concept used:
* If we have holes less than  five then store it in member d/sindirect_block_hole[]
* Since if store it in a files which uses fixed record size of 1024 unsigned long.
* If we have 4 records then remaining 1020 are wasted.
* To avoid this we used s/dindirect_block_hole.-Thus saves space when holes<5 
* And hole=100 we use file though still there will be 924 are wasted.
* Lot of space still wasted. 
*
* Modified for giis.2
* Get rid of that holes concept since we use variable size records instead of fixed size
* record.Now with variable records  which doesnot waste space.We do not need holes at all.
*
* In giis2 we have following flags,
* sfragment_flag=0 files not uses single indirect at all.
* sfragment_flag=1 file uses single indirect but it's already in sequence.
* sfragment_flag=2 file uses single indirect with SIND FILE .
*/


int search4sequence12 ()
{
  unsigned long indirect_block[1024], prev;
  int i, hole, fp, count;
  count = hole = 0;
  lseek64 (fd, (unsigned long long) giis_f.info.data_block[12] * fs.block_size, 0);
  read (fd, indirect_block, fs.block_size);

  /* Check 4 holes */
  if (indirect_block[0] - giis_f.info.data_block[12] != 1)
  {
    idata_block[count++] = giis_f.info.data_block[12];
    idata_block[count++] = indirect_block[0];
    hole++;
  }

  prev = indirect_block[0];

  for (i = 1; i < 1024 && indirect_block[i]; i++)
  {

    if (indirect_block[i] - prev == 1)
    {
      prev = indirect_block[i];
      continue;
    }

    else
    {                           /* Hole uses {this_block,that_block} format */
      idata_block[count++] = prev;
      idata_block[count++] = indirect_block[i];
      hole++;
      prev = indirect_block[i];
    }

  }

  /* Update is_offset and sfragment_flag and last_data_block */

  if ((i == 1024 || indirect_block[i] == 0) && (hole == 0))
  {
    giis_f.info.is_offset = 0;
    giis_f.info.sfragment_flag = 1;
    giis_f.info.last_data_block = indirect_block[i - 1];
    return (hole);
  }

  if ((i == 1024 || indirect_block[i] == 0) && hole)
  {
    giis_f.info.is_offset = count;
    giis_f.info.sfragment_flag = 2;
    giis_f.info.last_data_block = indirect_block[i - 1];


    /* Write into file */
    fp = open (SIND_INFO_FILE, 1);
    if (fp == -1)
    {
      perror ("Open:");
      printf ("Error no :%d", errno);
      return -1;
    }
    lseek (fp, 0, 2);
    i = write (fp, idata_block, count * sizeof (unsigned long));
    if (i != count * sizeof (unsigned long))
    {
      perror ("write");
      printf ("Error no :%d", errno);
      return -1;
    }
    close (fp);
    return 1;
  }

  return 1;
}

/* 
* search4sequence13() Exactly sames as above but it deals with double indirect blocks
* Modified for giis.2
* Get rid of that holes.variable size records used instead of fixed size records.
*
* In giis2 we have following flags,
* dfragment_flag=0 files not uses double indirect at all.
* dfragment_flag=1 file uses double indirect but it's already in sequence.
* dfragment_flag=2 file uses double indirect with DIND FILE .
*
* Here files greater than 8.04MB are also recorded with an recursive call to 
* search4sequence13() untill all blocks are scanned.
*
*/

int search4sequence13 ()
{
  unsigned long indirect_block[1024];
  int i, fp, count, err, ret;
  static unsigned long prev;
  count = 0, ret = 0;

  /*
     Consider the following cases,
     1) When all block number of file is less than 1048575 there is no problem.
     2) When all block number of file is greater than 1048575 still no problem.
     3) When there is a file with block numbers that switches from less to
     greater than 1048575 we come acroess some problem.

     When we use block numbers more than 1048575 we make shift beyond range of 
     unsigned long (i.e)4294967295.But for some reasons-Which i don't know- it                    throws up raw data. So in these cases where we make a shift from blocks
     less then greater we tend the skip the second lseek().

     *Since I'm not sure what's the problem is? I'm unsure about this solution
     too.
   */

  if (gi_round == 0)
  {
    count = hole = 0;           /* Counts holes */
    lseek64 (fd, (unsigned long long) giis_f.info.data_block[13] * fs.block_size, 0);
    read (fd, indirect_block, fs.block_size);

    lseek64 (fd, (unsigned long long) indirect_block[0] * fs.block_size, 0);
    read (fd, indirect_block, fs.block_size);
    prev = giis_f.info.data_block[13];

  }
  else
  {
    lseek64 (fd, (unsigned long long) prev * 4096, 0);
    read (fd, indirect_block, fs.block_size);
    lseek64 (fd, (unsigned long long) indirect_block[0] * 4096, 0);
    read (fd, indirect_block, fs.block_size);
  }

  if (indirect_block[0] - prev != 2)
  {

    idata_block[count++] = prev;
    idata_block[count++] = indirect_block[0];
    hole++;
  }

  prev = indirect_block[0];

  for (i = 1; i < 1024 && indirect_block[i]; i++)
  {

    if (indirect_block[i] - prev == 1)
    {
      prev = indirect_block[i];
      continue;
    }

    else
    {
      idata_block[count++] = prev;
      idata_block[count++] = indirect_block[i];
      hole++;
      prev = indirect_block[i];
    }


  }

  /* Update id_offset and dfragment_flag and last_data_block */

  if ((indirect_block[i] == 0) && (hole == 0))
  {
    giis_f.info.id_offset = 0;
    giis_f.info.dfragment_flag = 1;
    giis_f.info.last_data_block = indirect_block[i - 1];

    return 444;
  }

  if ((i == 1024 || indirect_block[i] == 0) && hole)
  {
    giis_f.info.id_offset += count;
    giis_f.info.dfragment_flag = 2;
    giis_f.info.last_data_block = indirect_block[i - 1];

    /* Write into file */

    fp = open (DIND_INFO_FILE, 1);
    if (fp == -1)
    {
      perror ("Open:");
      printf ("Error no :%d", errno);
      return -1;
    }
    lseek (fp, 0, 2);
    err = write (fp, idata_block, count * sizeof (unsigned long));
    if (err != count * sizeof (unsigned long))
    {
      perror ("write");
      printf ("Error no :%d", errno);
      return -1;
    }
    close (fp);
  }

  if (i == 1024 && indirect_block[1023] != 0)
  {
    prev = indirect_block[1023];
    gi_round++;
    ret = search4sequence13 ();
    if (ret == 444)
      return 1;

  }


  return 444;                   /* Will be caught by recursive function */
}

/*	 
* Introduced for giis2:
* avoid_dups():Called Only during installtion 
* This is used to figureout whether we have any duplication of directory entry.
* call2remove() if found.
*/

int avoid_dups ()
{
  int fp1, fp2, i, record_no, bity;
  unsigned long temp, end;

  fp1 = open (DIR_INFO_FILE, 0);
  if (fp1 == -1)
  {
    perror ("open:");
    printf ("\nError No:%d", errno);
    close (fp1);
    return -1;
  }
  fp2 = open (DIR_INFO_FILE, 0);
  if (fp2 == -1)
  {
    perror ("open:");
    printf ("\nError No:%d", errno);
    close (fp2);
    return -1;
  }

  /* Not so elegant code/method to check 4 repeatation 
     but i think...it's different */



  end = lseek (fp1, 0, 2);      /* EOF */
  lseek (fp1, 0, 0);
  temp = bity = 0;

  while (temp < (end - (int)GIIS_DSIZE))
  {
    i = read (fp1, giis_d.buffer, GIIS_DSIZE);
    if (i != GIIS_DSIZE)
    {
      perror ("read1()");
      printf ("\nError No:%d \t fp=%d", errno, fp1);
      close (fp1);
      close (fp2);
      if(i==0){
	printf("\n Fatal Error in %s",DIR_INFO_FILE);
	exit(0);
	}
      return -1;
    }
    temp += GIIS_DSIZE;

    /* Read next record in fp2 from current record of fp1 */

    lseek (fp2, lseek (fp1, 0, 1), 0);

    while (i > 0)
    {
      i = read (fp2, giis_dt.buffer, GIIS_DSIZE);

      if ((giis_d.info.inode_number == giis_dt.info.inode_number)
          && (giis_dt.info.search_flag == 1))
      {

        /* Get the duplicate record number */

        record_no = lseek (fp2, 0, 1) / GIIS_DSIZE;
        call2remove (record_no);
      }
    }
  }
  close (fp1);
  close (fp2);
  return 1;
}

/*
* Introduced for giis2:
* call2remove()- This removes a directory entry from future processing by just resetting its 
* search flag to zero.
* This is helper funtion to avoid_dups().
*/

int call2remove (int record_no)
{
  int fdes, i;
  fdes = open (DIR_INFO_FILE, 2);
  if (fdes == -1)
  {
    perror ("open");
    return -1;
  }

  lseek (fdes, (record_no - 1) * GIIS_DSIZE, 0);
  i = read (fdes, giis_dt.buffer, GIIS_DSIZE);
  if (i != GIIS_DSIZE)
  {
    perror ("read1()");
    printf ("\nError No:%d \t fp=%d", errno, fdes);
    close (fdes);
    return -1;
  }


  giis_dt.info.search_flag = 0;
  lseek (fdes, -GIIS_DSIZE, 1);
  i = write (fdes, giis_dt.buffer, GIIS_DSIZE);

  if (i != GIIS_DSIZE)
  {
    perror ("read1()");
    printf ("\nError No:%d \t fp=%d", errno, fdes);
    close (fdes);
    return -1;
  }


  close (fdes);
  return 1;

}

/*
* Introduced for giis2.1:
* check4file_dups() : Checks whether a file is a valid entry or not.
*/
int check4file_dups (unsigned long parent)
{
  int i, fp;

  fp = open (FILE_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("check4file_dups:open");
    return -1;
  }
  if (lseek (fp, 0, 0) == lseek (fp, 0, 2))
  {                             /* Empty file */
    close (fp);
    return 1;
  }
  lseek (fp, 0, 0);             /* Reset */

  i = read (fp, giis_f.buffer, GIIS_FSIZE);
  while (i > 0)
  {
    if ((giis_f.info.inode_number == ide.de.inode)
        && (giis_f.info.parent_inode_number == parent)
        && (giis_f.info.file_size == iin.in.i_size))
    {
      close (fp);
      return 0;
    }
    i = read (fp, giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("searchnupdate.c:check4file_dups()-read:Error Number:%d", errno);
      return -1;
    }
  }
  close (fp);
  return 1;
}

/*
* Introduced for giis2.1:
* check4dir_dups() : Checks whether a directory  is a valid entry or not.
*/
int check4dir_dups (unsigned long parent)
{
  int i, fp;

  if (ide.de.inode == ROOT_INODE)
    return 0;
  if (ide.de.name[0] == '.')
    return 0;
  fp = open (DIR_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }
  if (lseek (fp, 0, 0) == lseek (fp, 0, 2))
  {                             /* Empty file */
    close (fp);
    return 1;
  }
  lseek (fp, 0, 0);             /* Reset */

  i = read (fp, giis_d.buffer, GIIS_DSIZE);
  while (i > 0)
  {
    if (giis_d.info.inode_number == ide.de.inode)
    {                           /* We already have it */
      close (fp);
      return 0;
    }
    i = read (fp, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  close (fp);
  dir_level = 1;
  i = dir_depth (parent);
  if (i == -1)
  {
    perror ("");
    printf ("searchnupdate.c:check4dir_dups()-Error Number:%d", errno);
    return -1;
  }
  if ((i == 444) && (dir_level < level_value))
    return 1;                   /* Record this new entry  */

  if ((i == 444) && (dir_level > level_value))
    return 0;                   /* Don't record  since out of range of  LEVEL_VALUE */

  return 0;
}

/*
* Introduced for giis2.1:
* dir_depth() : Returns level_value of newly found dir.
*/
int dir_depth (unsigned long inode)
{
  int i, fp;

  if (inode == ROOT_INODE)      /* We reached the depth */
    return 444;
  fp = open (DIR_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }

  i = read (fp, giis_d.buffer, GIIS_DSIZE);

  while (i > 0)
  {
    if (inode == ROOT_INODE)
      return 444;

    if (giis_d.info.inode_number == inode)
    {                           /* When parent and grandparent is same */
      close (fp);
      dir_level++;
      i = dir_depth (giis_d.info.parent_inode_number);
      if (i == 444)
        return i;
      else
        return 0;
    }
    i = read (fp, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("searchnupdate.c:dir_depth()Error Number:%d", errno);
      return -1;
    }
  }
  close (fp);
  return 0;
}

/*
* Introduced for giis3.0:
* get_sample_data() : Get content of  each file and store them.
*/
int get_sample_data ()
{
  int sfp, ffp, dfp, i, ret;
  unsigned long device_block_number;
  char sample[32];

  //Open sample file 
  sfp = open (SAMPLE_DATA_FILE, O_RDWR);
  if (sfp == -1)
  {
    printf ("\nFile Not found %s", SAMPLE_DATA_FILE);
    return -1;
  }
  //Open the device
  dfp = open (device_name, 0);
  if (dfp == -1)
  {
    perror ("open");
    {
      perror ("");
      printf ("Searchnupdate:device_name:open:Error Number:%d", errno);
      return -1;
    }
  }

  //Open the FILE informations file 
  ffp = open (FILE_INFO_FILE, 0);
  if (ffp == -1)
  {
    perror ("Searchupdate:FILE_INFO_FILE:open");
    return -1;
  }
  //Now read each file records 
  i = read (ffp, giis_f.buffer, GIIS_FSIZE);

  while (i > 0)
  {

    /* Check whether the inode alread available in SAMPLE FILE  */
    /* During installtion this check won't be needed install = 0; */
    if(install == 0)
    ret = check4samplefile_dups (giis_f.info.inode_number);
		
    if (ret)
    {
      fs.block_number = giis_f.info.data_block[0];
      if (fs.block_number != 0)

      {

        i = eye_on_gd ();
        if (i == -1)
        {
	//giis 4.2 warn disbaled.
//          printf ("\n\t Warning : Abnormal block number: Group Descriptor not found ");
          close (ffp);
          close (dfp);
          close (sfp);
          return -1;
        }
        device_block_number =
          ((fs.block_number - 1) % fs.blocks_per_group) +
          fs.group_number * fs.blocks_per_group;
        device_block_number += fs.first_data_block;
        fs.content_offset =
          (unsigned long long) device_block_number *(unsigned long long) fs.block_size;
        fs.content_offset += fs.block_size;
        //set device file to appropriate location
        lseek64 (dfp, fs.content_offset, 0);
        //Empty the buffer - sample
        for (i = 0; i < 32; i++)
        {
          giis_s.info.sdata[i] = '\0';
        }
        //read data from disk
        i = read (dfp, sample, 32);     //0-31
        if (i != 32)
        {
          perror ("read():get_sample_data()");
          printf ("\nError No:%d", errno);
          close (ffp);
          close (dfp);
          close (sfp);
          return -1;
        }
        sample[31] = '\0';      //32 is 
        strcpy (giis_s.info.sdata, sample);
        giis_s.info.inode_number = giis_f.info.inode_number;


        //Inode and sample data is in struct now write into sample file 
        i = write (sfp, giis_s.buffer, GIIS_SSIZE);
        if (i != GIIS_SSIZE)
        {
          perror (" write():");
          printf ("\nError No:%d", errno);
          close (ffp);
          close (dfp);
          close (sfp);
          return -1;

        }
      }
    }
    //Next  record
    i = read (ffp, giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  close (ffp);
  close (dfp);
  close (sfp);
  return 0;
}

/*
* Introduced for giis4.1:
* check4samplefile_dups() : Checks whether a file's data should be retireved or not ..
*/
int check4samplefile_dups (unsigned long temp_inode_number)
{
  int i, fp;
  fp = open (SAMPLE_DATA_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }
  if (lseek (fp, 0, 0) == lseek (fp, 0, 2))
  {                             /* Empty file */
    close (fp);
    return 1;
  }
  lseek (fp, 0, 0);             /* Reset */

  i = read (fp, giis_s.buffer, GIIS_SSIZE);
  while (i > 0)
  {
    if (giis_s.info.inode_number == temp_inode_number)
    {
      close (fp);
      return 0;                 //already exists 
    }
    i = read (fp, giis_s.buffer, GIIS_SSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  close (fp);
  return 1;
}
