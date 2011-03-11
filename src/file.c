/*
* /giis/file.c This file contains procedures to read data block and displays conents
* of giis_files.
*
* Copyright (C) 2007 G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include "giis.h"

/*
* read_show_file() -  Reads data from given block and stores into a files.
* size is global variable used to specify the remaining file size after every read.
* Before recording the compare data with sample data if same continue else 
* don't hack ;-)
* Modification for giis4:
* Implementation giis dump option.
*/

int read_show_file (int fd, int fp)
{
  int i, count = 0;
  char buffer[4096], a;         /* buffer_size=fs.block_size */
  char sbuf[32];
  int firstread = 0;
  int giis_dump = 0;

  if (size == giis_f.info.file_size)
    firstread = 1;
  else
    firstread = 0;

  if (firstread)
  {
    /* Get Sample Data for this inode and compare -Compare for each file only once  */
    i = read_sample_info_file (giis_f.info.inode_number);
    /* get data from disk */
    i = read (fd, &sbuf, 32);   //0-31
    if (i != 32)
    {
      perror ("read():read_sample_info_file()");
      printf ("\nError No:%d", errno);
      close (fd);
      return -1;
    }
    sbuf[31] = '\0';            // since 31 is 32 in array 
    if (strcmp (giis_s.info.sdata, sbuf) != 0)
    {
      printf ("\n\t\t ****** giis dump ******");
      printf ("\n\t %s : File Data Modified/Overwritten\n\n", giis_f.info.name);
      printf ("\n\t File older Data   :");
      printf ("\n\t ===================");
      show_data (giis_s.info.sdata);
      printf ("\n\t Current Disk Data :");
      printf ("\n\t ===================");
      //where to look from 4h byte for this ...and first byte for sample file ...
      startbyte = 4;
      show_data (sbuf);
      startbyte = 0;
      printf ("\n\tDo You wanna dump the Current Disk data?\n\tPress 1:yes \tPress 0:No");
      printf ("\nEnter :");
      scanf ("%d", &giis_dump);
      if (giis_dump != 1)
      {
        fileoverwritten = 444;
        return JSJ_NO_RETURN;
      }
      else
        fileoverwritten = 0;
    }
    //Else Content is same -or giis dump used -Reset the device pointer
    lseek64 (fd, fs.content_offset, 0);
  }

  while ((count < fs.block_size) && (size > 0))
  {

    /*Reading char by char */

    if (size < sizeof (buffer) && size > 0)
    {
      i = read (fd, &a, 1);
      while (size && count < fs.block_size)
      {

        i = write (fp, &a, 1);
        if (i != 1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        size--;
        err_size++;
        count++;
        i = read (fd, &a, 1);
      }
    }

    /*Read block by block */

    if (size >= sizeof (buffer) && (count == 0))
    {
      i = read (fd, buffer, sizeof (buffer));
      if (i != sizeof (buffer))
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      i = write (fp, buffer, sizeof (buffer));
      if (i != sizeof (buffer))
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      size -= sizeof (buffer);
      err_size += sizeof (buffer);
      count = fs.block_size;

    }
  }
  return (size);
}

/*
* read_dir_info_file() read data from dir_info_file and displays. 
*/

int read_dir_info_file ()
{
  int i, count = 0,tfd;
  char dname[50];


  tfd = open (DIR_INFO_FILE, 0);
  if (tfd == -1)
  {
    perror ("open");
    return -1;
  }
  printf ("Enter dir:");
  scanf ("%s", dname);
  printf ("\n\tName\tinode\tParent-inode\tSearchFlag\n");
  i = read (tfd, giis_d.buffer, GIIS_DSIZE);

  while (i > 0)
  {
    if (giis_d.info.search_flag == 1 )
    {
      printf ("\n\t%s\t%lu\t", giis_d.info.name, giis_d.info.inode_number);
      printf ("%lu\t%d", giis_d.info.parent_inode_number, giis_d.info.search_flag);

      count++;
    }
    if (strcmp (dname, giis_d.info.name) == 0)
      return 1;



    i = read (tfd, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }

  printf ("\n Total No.of Dir : %d", count);
  close (tfd);
  return 1;
}

/*
* read_file_info_file() read data from file_info_file and displays. 
* Modification for giis2:
* New members like id_offset and is_offset used to keep trace of no.of blocks entry of
* particular files in sind and dind files.
* Where as s_offy and d_offy used for set offset in sind and dind files.
*/

int read_file_info_file ()
{
  int i, fd, count = 0, choice;
//int index,hole,num;
//unsigned long indirect_block;
  char fname[50];

  fd = open (FILE_INFO_FILE, 0);
  if (fd == -1)
  {
    perror ("open");
    return -1;
  }
  printf ("\n\t\tPress 1: All files");
  printf ("\n\t\tPress 2: Specific file");
  printf ("\n\t\tType in:");
  scanf ("%d", &choice);
  if (choice == 2)
  {
    printf ("\n\t\t Enter filename :");
    scanf ("%s", fname);
  }
  else
    choice = 1;


  i = read (fd, giis_f.buffer, GIIS_FSIZE);
  while (i > 0)
  {
    count++;
    printf ("\n ============================================");
    printf ("\nFile name   :%s", giis_f.info.name);
    printf ("\tInode :%lu", giis_f.info.inode_number);
    printf ("  sfragment_flags:%d ", giis_f.info.sfragment_flag);
    printf ("  dfragment_flags:%d ", giis_f.info.dfragment_flag);
    printf ("  is_offset %lu ", giis_f.info.is_offset);
    printf ("  id_offset %lu ", giis_f.info.id_offset);

    printf ("\nRecorded on :%s", giis_f.info.date_time);
    printf ("\t\tFile name   :%s", giis_f.info.name);
    printf ("\nInode :%lu", giis_f.info.inode_number);
    printf ("\tParent :%lu", giis_f.info.parent_inode_number);
    printf ("\tSearch Flag: %d", giis_f.info.search_flag);
    printf ("\tMode:%d", giis_f.info.mode);
    printf ("\nOwner: %d", giis_f.info.owner);
    printf ("\tBlocks count:%lu", giis_f.info.file_blocks_count);
    printf ("\tSize: %lu", giis_f.info.file_size);

    printf ("\nData blocks :");
    for (i = 0; i < EXT3_N_BLOCKS; i++)
      printf ("\n[%d]=%lu", i, giis_f.info.data_block[i]);
    printf ("\nsingles");
    printf ("\nis_offset=%lu\n id=%lu", giis_f.info.is_offset, giis_f.info.id_offset);
    printf ("\ns_offy=%lu \t d_offy=%lu", s_offy, d_offy);
    printf ("Sample Data :");
    read_sample_info_file (giis_f.info.inode_number);



    if (giis_f.info.is_offset)
    {
      read_sind_info_file ();
      s_offy += giis_f.info.is_offset;
    }

    if (giis_f.info.id_offset)
    {
      read_dind_info_file ();
      d_offy += giis_f.info.id_offset;
    }
    printf ("\nis_offset=%lu\n id=%lu", giis_f.info.is_offset, giis_f.info.id_offset);
    printf ("\ns_offy=%lu \t d_offy=%lu", s_offy, d_offy);
    printf ("\nLast :%lu", giis_f.info.last_data_block);

    if (strcmp (fname, giis_f.info.name) == 0 && choice == 2)
    {
      printf ("\n\t\tPress 0:Try next file\n\t\tPress 1:Exit  \n\t\tType in:");
      scanf ("%d", &i);
      if (i != 0)
        return 1;

    }
    if (choice == 1)
    {
      printf ("\n\t\tPress 0:Continue\n\t\tPress 1 : Stop ");
      printf ("\n\t\tEnter your choice:");
      scanf ("%d", &i);
      if (i == 1)
        return 1;
    }


    i = read (fd, giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  printf ("\n Total No.of Files : %d", count);
  close (fd);
  return 1;
}


/*
* read_sind_info_file() read data from sind and displays. 
* introduced for giis2.
* s_offy specifies offset at sind file.
* is_offset gives total no.of entries of this record.
*/

int read_sind_info_file ()
{
  int i, fp;
  unsigned long iblock[1024];

  fp = open (SIND_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    close (fp);
    return -1;
  }
  lseek (fp, s_offy * sizeof (unsigned long), 0);
  i = (int) read (fp, iblock, giis_f.info.is_offset * sizeof (unsigned long));
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  i = 0;
  while (i < giis_f.info.is_offset)
  {

    //printf("\n%d==>%u",i,iblock[i]);
    i++;

  }
  printf ("\n SIND No.of records :%d", i);
  close (fp);
  return 1;
}

/*
* read_dind_info_file() read data from dind and displays. 
* introduced for giis2.
* d_offy specifies offset at dind file.
* id_offset gives total no.of entries of this record.
*/

int read_dind_info_file ()
{
  int i, fp;
  unsigned long iblock[1024];

  fp = open (DIND_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    close (fp);
    return -1;
  }
  lseek (fp, d_offy * sizeof (unsigned long), 0);
  i = (int) read (fp, iblock, giis_f.info.id_offset * sizeof (unsigned long));
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  i = 0;

  while (i < giis_f.info.id_offset)
  {
    //printf("\n%d==>%u",i,iblock[i]);
    i++;

  }

  printf ("\n DIND No.of records :%d", i);
  close (fp);
  return 1;
}

/*
* introduced for giis3
* read_sample_info_file() reads sample  file.  
*/

int read_sample_info_file (unsigned long inode_number)
{
  int i, sfp;

  sfp = open (SAMPLE_DATA_FILE, 0);
  if (sfp == -1)
  {
    perror ("open");
    close (sfp);
    return -1;
  }
  i = read (sfp, giis_s.buffer, GIIS_SSIZE);
  while (i > 0)
  {
    if (giis_s.info.inode_number == inode_number)
    {
      show_data (giis_s.info.sdata);
      close (sfp);
      return 1;
    }
    i = read (sfp, giis_s.buffer, GIIS_SSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  close (sfp);
  return 1;
}

/*
* Introduced for giis4
* show_data()- displays data meaningful to read.
* Updates for giis4.1 :
* giis analyser is include to detect the file formats and inform users :-)
*/
void show_data (char buffer[32])
{
  int i;
  int yes = 0;
  printf ("\n\t\t-------------- giis analyser : Begins -------------------");
/*Uncomment to Debug
		for(i=0;i<32;i++){
		printf("[%d]=%x %c %d\n",i,buffer[i],buffer[i],buffer[i]);
		}
*/


/* Graphics Files */


  if ((buffer[startbyte] == 0xffffffff && buffer[startbyte + 1] == 0xffffffd8 &&
       buffer[startbyte + 2] == 0xffffffff && buffer[startbyte + 3] == 0xffffffe0))
  {
    printf ("\n\t\t giis : It's Jpeg");
    yes = 444;
  }

  if (buffer[startbyte] == 0x47 && buffer[startbyte + 1] == 0x49
      && buffer[startbyte + 2] == 0x46 && buffer[startbyte + 3] == 0x38)
  {
    printf ("fname,giis_f.info.name\n\t\t giis : It's GIF");
    yes = 444;
  }
  if (buffer[startbyte] == 0xffffff89 && buffer[startbyte + 1] == 0x50
      && buffer[startbyte + 2] == 0x4e && buffer[startbyte + 3] == 0x47)
  {
    printf ("\n\t\t giis : It's PNG");
    yes = 444;
  }
  if (buffer[startbyte] == 0x49 && buffer[startbyte + 1] == 0x49
      && buffer[startbyte + 2] == 0x2a && buffer[startbyte + 3] == 0x00)
  {
    printf ("\n\t\t giis : It's TIF");
    yes = 444;
  }
  if (buffer[startbyte] == 'B' && buffer[startbyte + 1] == 'M'
      && buffer[startbyte + 2] == '?' && buffer[startbyte + 3] == '?'
      && buffer[startbyte + 4] == 0x00)
  {
    printf ("\n\t\t giis : It's BMP ");
    yes = 444;
  }


/*Video - Animated files */

  if (buffer[startbyte] == 'R' && buffer[startbyte + 1] == 'I'
      && buffer[startbyte + 2] == 'F' && buffer[startbyte + 3] == 'F'
      && buffer[startbyte + 4] == '?' && buffer[startbyte + 5] == '?'
      && buffer[startbyte + 6] == '?' && buffer[startbyte + 7] == '?'
      && buffer[startbyte + 8] == 'A' && buffer[startbyte + 9] == 'V'
      && buffer[startbyte + 10] == 'I')
  {
    printf ("\n\t\t giis : It's AVI ");
    yes = 444;
  }


  if (buffer[startbyte] == '?' && buffer[startbyte + 1] == '?'
      && buffer[startbyte + 2] == '?' && buffer[startbyte + 3] == '?'
      && buffer[startbyte + 4] == '?' && buffer[startbyte + 5] == '?'
      && buffer[startbyte + 6] == '?' && buffer[startbyte + 7] == '?'
      && buffer[startbyte + 8] == 0x6d)
  {
    printf ("\n\t\t giis : It's Apple Quicktime ");
    yes = 444;
  }

  if ((buffer[startbyte] == 0x00 && buffer[startbyte + 1] == 0x00
       && buffer[startbyte + 2] == 0x01 && buffer[startbyte + 3] == 0xffffffba)
      || (buffer[startbyte] == 0x00 && buffer[startbyte + 1] == 0x00
          && buffer[startbyte + 2] == 0x01 && buffer[startbyte + 3] == 0xffffffb3))
  {
    printf ("\n\t\t giis : It's MPEG");
    yes = 444;
  }

  if (buffer[startbyte] == 'm' && buffer[startbyte + 1] == 'p'
      && buffer[startbyte + 2] == 'g')
  {
    printf ("\n\t\t giis : It's MPEG");
    yes = 444;
  }

  if (buffer[startbyte] == 'F' && buffer[startbyte + 1] == 'W'
      && buffer[startbyte + 2] == 'S')
  {
    printf ("\n\t\t giis : It's Flash Files");
    yes = 444;
  }


/* */

  if (buffer[startbyte] == 0x1f && buffer[startbyte + 1] == 0xffffff8b)
  {
    printf ("\n\t\t giis : It's GZip");
    yes = 444;
  }
  if (buffer[startbyte] == 0x7f && buffer[startbyte + 1] == 0x45
      && buffer[startbyte + 2] == 0x4c && buffer[startbyte + 3] == 0x46)
  {
    printf ("\n\t\t giis : It's Unix ELF");
    yes = 444;
  }
  if (buffer[startbyte] == 0x50 && buffer[startbyte + 1] == 0x4b
      && buffer[startbyte + 2] == 0x03 && buffer[startbyte + 3] == 0x04)
  {
    printf ("\n\t\t giis : It's Zip");
    yes = 444;
  }
  if (buffer[startbyte] == 0xffffffd0 && buffer[startbyte + 1] == 0xffffffcf
      && buffer[startbyte + 2] == 0x11 && buffer[startbyte + 3] == 0xffffffe0
      && buffer[startbyte + 4] == 0xffffffa1 && buffer[startbyte + 5] == 0xffffffb1)
  {
    printf ("\n\t\t giis : It's DOC");
    yes = 444;
  }

  if ((buffer[startbyte] == 0x2e && buffer[startbyte + 1] == 0x72
       && buffer[startbyte + 2] == 0x61 && buffer[startbyte + 3] == 0xfffffffd)
      || (buffer[startbyte] == '.' && buffer[startbyte + 1] == 'R'
          && buffer[startbyte + 2] == 'M' && buffer[startbyte + 3] == 'F'))
  {
    printf ("\n\t\t giis : It's Real Audio Files");
    yes = 444;
  }

  if ( buffer[startbyte] == 0x30 && buffer[startbyte + 1] == 0x26
      && buffer[startbyte + 2] == 0xB2 && buffer[startbyte + 3] == 0x75 )
  {
    printf ("\n\t\t giis : It's WMA Files");
    yes = 444;
  }


  if ((buffer[startbyte] == 0xffffffff && buffer[startbyte + 1] == 0xfffffffb
       && buffer[startbyte + 2] == 0xffffff90 && buffer[startbyte + 3] == 0x44)
      || (buffer[startbyte] == 0xffffffff && buffer[startbyte + 1] == 0xfffffffb
          && buffer[startbyte + 2] == 0xffffff90 && buffer[startbyte + 3] == 0x00)
      || (buffer[startbyte] == 0x57 && buffer[startbyte + 1] == 0x41
          && buffer[startbyte + 2] == 0x56 && buffer[startbyte + 3] == 0x45)
      || (buffer[startbyte] == 0xffffffff && buffer[startbyte + 1] == 0xfffffffb
          && buffer[startbyte + 2] == 0xffffffD0) || (buffer[startbyte] == 0x49
                                                      && buffer[startbyte + 1] == 0x44
                                                      && buffer[startbyte + 2] == 0x33)
      || (buffer[startbyte] == 0x4C && buffer[startbyte + 1] == 0x41
          && buffer[startbyte + 2] == 0x4D && buffer[startbyte + 3] == 0x45))
  {
    printf ("\n\t\t giis : It's Mp3");
    yes = 444;
  }

  if (buffer[startbyte] == '%' && buffer[startbyte + 1] == 'P'
      && buffer[startbyte + 2] == 'D' && buffer[startbyte + 3] == 'F')
  {
    printf ("\n\t\t giis : It's a PDF File");
    yes = 444;
  }

  if (buffer[startbyte] == 0xED && buffer[startbyte + 1] == 0xAB)
  {
    printf ("\n\t\t giis : It's a Linux rpm File");
    yes = 444;
  }


  if (buffer[startbyte] == 'R' && buffer[startbyte + 1] == 'I'
      && buffer[startbyte + 2] == 'F' && buffer[startbyte + 3] == 'F'
      && buffer[startbyte + 4] == '?' && buffer[startbyte + 5] == '?'
      && buffer[startbyte + 6] == '?' && buffer[startbyte + 7] == '?'
      && buffer[startbyte + 8] == 'W' && buffer[startbyte + 9] == 'A'
      && buffer[startbyte + 10] == 'V' && buffer[startbyte + 11] == 'E')
  {
    printf ("\n\t\t giis : It's WAV files ");
    yes = 444;
  }
  if (buffer[startbyte] == 0x30 && buffer[startbyte + 1] == 0x26
      && buffer[startbyte + 2] == 0xB2 && buffer[startbyte + 3] == 0x75
      && buffer[startbyte + 4] == 0x8E && buffer[startbyte + 5] == 0x66
      && buffer[startbyte + 6] == 0xCF && buffer[startbyte + 7] == 0x11
      && buffer[startbyte + 8] == 0xA6 && buffer[startbyte + 9] == 0xD9
      && buffer[startbyte + 10] == 0x00 && buffer[startbyte + 11] == 0xAA
      && buffer[startbyte + 12] == 0x00 && buffer[startbyte + 13] == 0x62
      && buffer[startbyte + 14] == 0xCE)
  {

    printf ("\n\t\t giis : It's WMV files ");
    yes = 444;
  }
  if (buffer[startbyte] == 'R' && buffer[startbyte + 1] == 'a'
      && buffer[startbyte + 2] == 'r' && buffer[startbyte + 3] == '!')
  {
    printf ("\n\t\t giis : It's a Rar File");
    yes = 444;
  }

  if (buffer[startbyte] == 0xCA && buffer[startbyte + 1] == 0xFE
      && buffer[startbyte + 2] == 0xBA && buffer[startbyte + 3] == 0xBE)
  {
    printf ("\n\t\t giis : It's a Java File");
    yes = 444;
  }
  if (buffer[startbyte] == '<' && buffer[startbyte + 1] == 'h'
      && buffer[startbyte + 2] == 't' && buffer[startbyte + 3] == 'm'
      && buffer[startbyte + 1] == 'l')
  {
    printf ("\n\t\t giis : It's a html File");
    yes = 444;
  }

  if (yes == 0)
  {
    printf ("\n\t\t Unable to Detect File Type - Found Data : \n\t\t");
    for (i = 0; i < 32; i++)
    {
      if ((buffer[i] >= 32) && (buffer[i] < 128))
      {
        if (buffer[i] != 10)
          printf ("%c", buffer[i]);
        else
          printf ("%x", buffer[i]);
      }
    }

  }
  printf ("\n\t\t--------------Ends --------------\n\n");

}

/*
* Introduced in giis v.4.1:
* file_repeated() : It returns number of times this file entry is available in giis system 
* files. 
*/
int file_repeated (char fname[50])
{
  int i, fd, exists = 0;
  union u_file_recover_info temp_giis_f;

  fd = open (FILE_INFO_FILE, 0);
  if (fd == -1)
  {
    perror ("open");
    return -1;
  }
  i = read (fd, temp_giis_f.buffer, GIIS_FSIZE);
  while (i > 0)
  {
    if (strcmp (fname, temp_giis_f.info.name) == 0)
    {
      exists++;
    }
    i = read (fd, temp_giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
//printf("\n Total No.of Times Files repeated : %d",exists);
  close (fd);
  return exists;
}
/*
Introduced to giis 4.3
display_deleted_files:Displays deleted files and status
*/
int display_deleted_files(){

 int i, fd, total=0,count = 0, deleted,temp=0;
 extern union u_s_rere_info giis_r;

  fd = open (FILE_INFO_FILE, 0);
  if (fd == -1)
  {
    perror ("open");
    return -1;
  }
 
  i = read (fd, giis_f.buffer, GIIS_FSIZE);
  printf("\n\t\tList of Deleted files :");
 printf("\n\t\t======================");
  

  while (i > 0)
  {
 
    deleted=0;
    
	//is it deleted?
	deleted = verify_inode (giis_f.info.inode_number);

	if(deleted){
	count++;
	for(i=0;i<count;i++)
		{//if already exists -path is already generated so skip this
		if(giis_r.info.list_of_deleted_inodes[i]==giis_f.info.inode_number){
		goto next; //first goto in giis :-)
		}
		}
	
	printf ("\n\n\t\t%s", giis_f.info.name);
	total++;


	giis_r.info.list_of_deleted_inodes[total-1]=giis_f.info.inode_number;
	
	//memset -clear 
	memset(pathname,'\0',150);
	strcat(pathname,"/");
	parent_found=0;
	lookup(giis_f.info.parent_inode_number);
	//place restore path 
	temp=total-1;
 	strcpy(giis_r.info.list_of_restorepaths[temp],pathname);
	next:
	;
    	}
	
   i = read (fd, giis_f.buffer, GIIS_FSIZE);
   }

printf("\n\t\t======================");
printf("\n\nTotal files:%d",total);
printf("\n\nNote:If required deleted file is not listed here,\nthen try \"force giis - giis dump\" option.\n\n");

return total;
}

/* Introduced for giis.4.3
lookup:Get parent name of given inode and construct absolute path name 
*/
int lookup (unsigned long parent)
{
  int fp, i;
  extern int parent_found;
  extern char pathname[150];

 
  giis_d.info.inode_number=0;
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
	if(parent_found==1)
	{
	strcat(pathname,"\/");
	strcat(pathname,giis_d.info.name);
	return 1;
	}

	

	if((parent_found==0) && (giis_d.info.parent_inode_number!=ROOT_INODE))
		{
		lookup(giis_d.info.parent_inode_number);
		}
	else{
	  close (fp);
	  strcat(pathname,giis_d.info.name);
	  parent_found=1;
      	  return 1;
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

  close (fp);
  return -1;
}

/*
Introduced in giis 4.4:
read_config_file: Reads Configuration file details
giis 4.5:
added : get TIME value from config file.
*/

int read_config_file(char file[100]){

char b[]="$DIR = ";
char buf[4096];
char directory[100][100];
char d[]="$DEVICE = ";
char e[]="$TIME = ";
char f[]="$LEVEL_VALUE = ";
int i,j,k,tfp;
int row,col,c=0;
i=0;

//printf("\n passed file~~~~~~>%s",file);
tfp=open(GIIS_CONF_FILE,0);
if(tfp ==  -1)
{
printf("\n Unable to load config file");
exit(0);
}
//read the file
i=read(tfp,buf,4096);
close(tfp);
//Search for device name 
if(strcmp(d,file)==0){
//printf("\n Checking Device name.. ");
memset(device_name,'\0',100);

//printf("\n First Device Name:%s\n",device_name);
i=0;c=0;
while(buf[i]!='\0'){
j=0;
k=i;

	while((d[j]==buf[k]) && (d[j]!='\0')){
		j++;
		k++;
		}
	
	if(d[j]=='\0'){
		while(buf[k]!='$'){
			device_name[c++] = buf[k++];
			}
	device_name[c] = '\0';
	return 1;
	i=k;
	//directory[row][col]='\0';
	//row++;col=0;
	}else
	i++;
	
}
if(strcmp(d,file)==0)
return -1;

}
//Search Ends

//Search for Time 
if(strcmp(e,file)==0){
//printf("\n Checking Time.. ");
memset(auto_time,'\0',5);

i=0;c=0;
while(buf[i]!='\0'){
j=0;
k=i;

	while((e[j]==buf[k]) && (e[j]!='\0')){
		j++;
		k++;
		}
	
	if(e[j]=='\0'){
		while(buf[k]!='$'){
			auto_time[c++] = buf[k++];
			}
	auto_time[c] = '\0';
	return 1;
	i=k;
	}else
	i++;
	
}
}
//Search for time ends


//Search for Level-value 
if(strcmp(f,file)==0){
//printf("\n Checking Level-value.. ");
memset(s_level_value,'\0',5);

i=0;c=0;
while(buf[i]!='\0'){
j=0;
k=i;

	while((f[j]==buf[k]) && (f[j]!='\0')){
		j++;
		k++;
		}
	
	if(f[j]=='\0'){
		while(buf[k]!='$'){
			s_level_value[c++] = buf[k++];
			}
	s_level_value[c] = '\0';
	return 1;
	i=k;
	}else
	i++;
	
}
}
//Search for Level-value ends

row=0;
col=0;
i=0;
while(buf[i]!='\0'){
j=0;
k=i;

	while((b[j]==buf[k]) && (b[j]!='\0')){
		j++;
		k++;
		}
	
	if(b[j]=='\0'){
//	printf("\nPattern exists");
	
		while(buf[k]!='$'){
			directory[row][col++]=buf[k++];
			//printf("\n%d %d==>%c",row,col-1,directory[row][col-1]);
				}
	i=k;
	directory[row][col]='\0';
	row++;col=0;
	}else
	i++;
	
}
if(row == 0)
//printf("\n pattern  not exists");
;
else
for(i=0,col=0;i<row;i++){
//printf("\n=====>%s",directory[i]);
//chk given name exists or not.
if(strcmp(directory[i],file)==0)
//printf("\n======*>%d %s ",i,file);
return 444;

}
return 1;
}

/*
Introduced in giis 4.4:
read_config_file2: Reads Configuration file details
returns directory names 
*/

int read_config_file2(){

char b[]="$DIR = ";
char buf[4096];
extern char g_directory[100][100];
int i,j,k,tfp,col;
extern int g_row;
i=0;

//printf("\n passed file~~~~~~>%s",file);
tfp=open(GIIS_CONF_FILE,0);
if(tfp ==  -1)
{
printf("\n Unable to load config file");
exit(0);
}
//read the file
i=read(tfp,buf,4096);
close(tfp);

g_row=0;
col=0;
i=0;
while(buf[i]!='\0'){
j=0;
k=i;

	while((b[j]==buf[k]) && (b[j]!='\0')){
		j++;
		k++;
		}
	
	if(b[j]=='\0'){
//	printf("\nPattern exists");
	
		while(buf[k]!='$'){
			g_directory[g_row][col++]=buf[k++];
			//printf("\n%d %d==>%c",row,col-1,directory[row][col-1]);
				}
	i=k;
	g_directory[g_row][col]='\0';
	g_row++;col=0;
	}else
	i++;
	
}
return 1;
}



/*
* read_conf_file_inum(): read data from dir_info_file and return inode number
input:directory from giis.conf
output:inode number
*/

int read_conf_file_inum ()
{
  int i, count = 0;
  int j,tfd;	
  extern new_dir_added;
  new_dir_added=0;

  tfd = open (DIR_INFO_FILE, 2);
  if (tfd == -1)
  {
    perror ("open");
    return -1;
  }
  //printf ("\n\tName\tinode\tParent-inode\tSearchFlag\n");
  i = read (tfd, giis_d.buffer, GIIS_DSIZE);
  while (i > 0)
  {
	  count++;
	if(giis_d.info.parent_inode_number==2){
	for(j=0;j<g_row;j++){
	if ((strcmp(giis_d.info.name,g_directory[j])==0)&&(giis_d.info.search_flag==0))
    	{
      	//printf ("\n\t%s\t%lu\t", giis_d.info.name, giis_d.info.inode_number);
    	 // printf ("%lu\t%d", giis_d.info.parent_inode_number, giis_d.info.search_flag);
	//reset the flag here.
	//printf("\n count:%d",count);
	lseek(tfd,-(GIIS_DSIZE),1);
	giis_d.info.search_flag=1;
	//new directory is included in giis sys
	new_dir_added=1;
	write (tfd, giis_d.buffer, GIIS_DSIZE);goto out;
	 }
	}
	}
	out:
    i = read (tfd, giis_d.buffer, GIIS_DSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }

  //printf ("\n Total No.of Dir : %d", count);
  close (tfd);
  return 1;
}