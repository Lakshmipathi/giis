/*
* /giis/get_it_i_say.c-Here we try to recover deleted files
*
* Copyright (C) 2007. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/


#include "giis.h"

/*
* get_it() This is will scan through the file entries with the help of verfiy_inode()
* Mofication for giis2:
* Keep track of s_offy and d_offy.
*/

int get_it ()
{
  int i, fd, no = 0;
  static int search_uid;
  search_uid = 0;
//Added for giis4.0
  char extension[10];
  int glength = 0, ulength = 0, pos, count, typefound;

  if (use_uid == 2)
  {
    i = force_giis ();
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    return 1;
  }

  if (use_uid == 1)
  {
    printf ("\n Enter the User Name....");
    scanf ("%s", user);
    pwfile = getpwnam (user);
    if (pwfile == NULL)
    {
      printf ("\nPlease Enter Correct user name ");
      return 1;
    }
    use_uid = pwfile->pw_uid;
    printf ("\nSearching For %s files with uid=%d", pwfile->pw_name, use_uid);
    search_uid = 1;             /* Search for specific users */
  }
  /* Get by File format */
  if (use_uid == 3)
  {
    printf ("\n Enter any file type:(txt,doc,mp3,c,cpp,dat--etc) :");
    scanf ("%s", extension);
    search_format = 1;
  }


  /* Reset offset of two files */

  d_offy = 0;
  s_offy = 0;


  fd = open (FILE_INFO_FILE, 2);
  if (fd == -1)
  {
    perror ("open");
    return -1;
  }

  i = read (fd, giis_f.buffer, GIIS_FSIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  while (i > 0)
  {
    /* Search for given file formats only */
    if (search_format == 1)
    {
      glength = 0;
      ulength = 0;
      glength = strlen (giis_f.info.name);
      ulength = strlen (extension);
      ulength++;                //get dot part too .txt /.cpp
      pos = glength - ulength;
      if (giis_f.info.name[pos] == '.')
      {
        pos++;
        count = 0;
        typefound = 1;
        for (; pos < glength; pos++, count++)
        {
          if (giis_f.info.name[pos] != extension[count])
          {
            typefound = 0;
            pos = glength;
          }
        }
        if (typefound)
        {

          i = verify_inode (giis_f.info.inode_number);
          if (i == -1)
          {
            perror ("");
            printf ("Error Number:%d", errno);
            return -1;
          }
        }
        else
          i = 0;
      }
    }

    /* Specific user is set and this is his entry then verfify else skip.
       If use_uid not set search for all files ignore uid */
    if (search_uid)
    {

      if (giis_f.info.owner == use_uid)
      {
        i = verify_inode (giis_f.info.inode_number);
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
      }
      else
        i = 0;
    }
    if ((search_format != 1) && (!search_uid))
    {
      i = verify_inode (giis_f.info.inode_number);
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
    }
    /* Update s_offy and d_offy here */

    if (giis_f.info.is_offset)
      s_offy += giis_f.info.is_offset;


    if (giis_f.info.id_offset)
      d_offy += giis_f.info.id_offset;


    if (i == 1)
    {

      get_it_i_say ();

      /* Finally update this entry in file_info_file() */

      giis_f.info.search_flag = 1;      /* Recovered or failed */
      lseek (fd, -GIIS_FSIZE, 1);
      i = write (fd, giis_f.buffer, GIIS_FSIZE);
      if (i != GIIS_FSIZE)
      {
        perror ("write");
        return -1;
      }
      no = 1;
    }
    i = read (fd, giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  if (no == 1)
  {
    printf ("\n\n giis process Completed...");
    printf ("\n\t Recovered files   : /giis/got_it");
    printf ("\n\t Unrecovered files : /giis/unrecovered");
  }
  else
    printf ("\n\n\t\tNothing Recovered...");
  close (fd);
  return 1;
}

/*
* verify_inode() -  This is a helper file to get_it_i_say() this one verifies inode
* is deleted or is being in use.
*/

int verify_inode (unsigned long inode)
{
  int i;
/* For time based recovery - giis4.4*/
  time_t rawtime;
  struct tm *mytm;

  fs.inode_number = inode;

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
  if (iin.in.i_links_count == 0)
  {
	
    deleted_inode_offset = fs.inode_offset;
    
/* Check whether it's time-based recovery - if so process it.*/
if(date_mode !=-1){	
	time(&rawtime);
	mytm=localtime(&rawtime);
	mytm->tm_mon=month-1;
	mytm->tm_mday=day;
	mytm->tm_year=year-1900;

	mytm->tm_sec=0;
	mytm->tm_min=0;
	mytm->tm_hour=0;
	result = mktime(mytm);

	//Deleted on 
	if(date_mode ==0){
	if ((iin.in.i_dtime >= result) && (iin.in.i_dtime<= (result+86400)))
	return 1;
	else
	return 0;
	}

	//Deleted before
	if(date_mode ==2){
	if (iin.in.i_dtime<result)
	return 1;
	else
	return 0;
	}	

	//Deleted after 

	if(date_mode==1){
	if (iin.in.i_dtime > (result+86400))
	return 1;
	else
	return 0;
	}

	//Deleted between 
	if(date_mode==3){
	//set to-date
	time(&rawtime);
	mytm=localtime(&rawtime);
	mytm->tm_mon=month1-1;
	mytm->tm_mday=day1;
	mytm->tm_year=year1-1900;

	mytm->tm_sec=0;
	mytm->tm_min=0;
	mytm->tm_hour=0;
	result1 = mktime(mytm);

	if ((iin.in.i_dtime > result) && (iin.in.i_dtime < (result1+86400)))
	return 1;
	else
	return 0;
	}
}
return 1; // for get all files
  }
  else
    return 0;

}

/*
* get_it_i_say()-This will try to retrieve the deleted file.
* Mofication for giis2:
* New Access methods for the following flags,
* sfragment_flag=0 files not uses single indirect at all.
* sfragment_flag=1 file uses single indirect but it's already in sequence.
* sfragment_flag=2 file uses single indirect with SIND FILE .
*
* dfragment_flag=0 files not uses double indirect at all.
* dfragment_flag=1 file uses double indirect but it's already in sequence.
* dfragment_flag=2 file uses double indirect with DIND FILE .
*
*/

int get_it_i_say ()
{
  int i, fp, fdes, fd, num, hole, eof, count, tfp;
  unsigned long indirect_block, iblock[1024];
  char name[75] = "/giis/got_it/";
  char over[75] = "/giis/unrecovered/";
  extern char rere_filenames[200][150];
  extern int rere_count;
  extern unsigned long rere_inodes[1024];


  /* just always create a  new file */
  strcat (name, giis_f.info.name);

  fp = open (name, 0);
  if (fp != -1)
  {
    /*Already retrieved */
    close (fp);
    return 1;
  }


  fp = open (name, O_CREAT | O_RDWR | O_TRUNC,S_IRWXU |S_IRWXG |S_IRWXO );
  if (fp == -1)
  {
    printf ("\nFile Not found %s", name);
    close (fp);
    return -1;
  }

//Store file names to temp. variable for rere
    if((rere==1)&&(rere_count<200)){
	strcpy(rere_filenames[rere_count],giis_f.info.name);
	rere_inodes[rere_count]=fs.inode_number;
	rere_count++;
  }


  fd = open (device_name, 0);
  if (fd == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }
  eof = 1;
  fileoverwritten = 0;
  size = giis_f.info.file_size; /*  Decrement afer every read to denote EOF  */
  err_size = 0;                 /* Increment after read for Error check */
  gd = 0;                       /* used in get_data_from_block */
	/* giis 4.4 - Set owner & permission mode */
	chmod(name,iin.in.i_mode);
	chown(name,iin.in.i_uid,iin.in.i_gid);

  for (i = 0; (i <= 14) && eof; i++)
  {


    /*Don't use fragment_flag Here */

    if (i < 12 && giis_f.info.data_block[i] != 0)
    {                           /* Direct blocks */
      eof = get_data_from_block (fp, fd, giis_f.info.data_block[i]);
      /* If Data Overwritten then Quit */
      if (fileoverwritten == 444)
      {
        close (fp);
        i = unlink (name);
        strcat (over, giis_f.info.name);
        tfp = open (over, O_CREAT | O_RDWR | O_TRUNC);
        if (tfp == -1)
        {
          printf ("\nFile Not found %s", over);
          return -1;
        }
        close (tfp);
        close (fd);
        return 1;
      }
      if (eof == 0)
      {
        close (fp);
        close (fd);
        return 1;
      }

      continue;
    }


    /* part 1: Access single indirect blocks  */

    if (i == 12 && giis_f.info.data_block[12] != 0)
    {

      indirect_block = giis_f.info.data_block[12];

      /* sfragment_flag=1 just read by incrementing number */


      if (giis_f.info.sfragment_flag == 1)
      {

        for (count = 0; count < 1024; count++)
        {                       /* Sequence */
          indirect_block++;
          eof = get_data_from_block (fp, fd, indirect_block);


          if (eof == 0)
          {
            close (fp);
            close (fd);
            return 1;
          }
        }
      }

      /* sfragment_flag=2 read from sind_info_file */



      if (giis_f.info.sfragment_flag == 2)
      {                         /* holes here n there */
        num = 0;

        fdes = open (SIND_INFO_FILE, 0);
        if (fdes == -1)
        {
          perror ("open");
          printf ("\nError No : %d", errno);
        }

        lseek (fdes, (s_offy - giis_f.info.is_offset) * sizeof (unsigned long), 0);
        read (fdes, iblock, giis_f.info.is_offset * sizeof (unsigned long));
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        hole = iblock[num];

        for (count = 0; count < 1024; count++)
        {

          if (indirect_block == hole)
          {
            indirect_block = iblock[num + 1];
            num += 2;
            hole = iblock[num];
          }
          else
            indirect_block++;

          eof = get_data_from_block (fp, fd, indirect_block);
          if (eof == 0)
          {
            close (fp);
            close (fd);
            return 1;
          }
        }
      }
      continue;
    }

    /* part 2: Access double indirect blocks */

    if (i == 13 && giis_f.info.data_block[13] != 0)
    {


      indirect_block = giis_f.info.data_block[13];
      indirect_block++;

      /* dfragment_flag=1 just read by incrementing number */

      if (giis_f.info.dfragment_flag == 1)
      {
        while (eof)
        {                       /* Sequence */
          indirect_block++;
          eof = get_data_from_block (fp, fd, indirect_block);
          if (eof == 0)
          {
            close (fp);
            close (fd);
            return 1;
          }
        }

      }

      /* dfragment_flag=2 read from file_info_file */

      if (giis_f.info.dfragment_flag == 2)
      {                         /* holes here n there */
        num = 0;
        fdes = open (DIND_INFO_FILE, 0);
        if (fdes == -1)
        {
          perror ("open");
          printf ("\nError No : %d", errno);
        }

        lseek (fdes, (d_offy - giis_f.info.id_offset) * sizeof (unsigned long), 0);
        read (fdes, iblock, giis_f.info.id_offset * sizeof (unsigned long));
        if (i == -1)
        {
          perror ("");
          printf ("Error Number:%d", errno);
          return -1;
        }
        hole = iblock[num];
        count = 0;
        /* 4 holes occur first say at [13] itself */
        if (hole == (indirect_block - 1))
          indirect_block = hole;

        /*
           For files more than 8.04 MB we need count to keep trace of 
           when to double jump from position 1022 to next.
           It's has taken few hours for me to find solution why the above                         if block doesn't reset count.The answer is, 
           In situations like we have a hole at indirect_block+1 and count=1023
           doesn't enter above block then count goes to beyond 1024 so in the 
           following iterations too we can't reset count so we have...
         */

        while (eof)
        {
          if (indirect_block == hole)
          {
            indirect_block = iblock[num + 1];
            num += 2;
            hole = iblock[num];
            if (count == 1024)
              count = 0;
          }
          else
            indirect_block++;

          if (count == 1024)
          {
            indirect_block++;
            count = 0;
          }


          eof = get_data_from_block (fp, fd, indirect_block);
          if (eof == 0)
          {
            close (fp);
            close (fd);
            return 1;
          }
          count++;


        }
      }
    }


  }
  close (fd);
  close (fp);
  return 1;
}

/*
* get_data_from_block() is a helper function to above procedure it computes the content 
* offset of given block number and access that data.
* Modification for giis2:
* Check introduced with help of last_data_block.
*/

int get_data_from_block (int fp, int fd, unsigned long data_block)
{
  int i = 0;
  unsigned long device_block_number;
/*
static int lg;
char *disp1="\ngiis.........Smile or Sad face?";
char *disp2="                 ";
char disp3[]="|\\-/|\\-/"; 
char disp4[]="               :-";
char disp5[]=")(";
*/



  /* just for fun */

  /* Some graphics output commented because it's affects speedy recovery 
     Removing following comments is not a risky opertions at all. */

/*		system("clear");
		write(0,disp1,strlen(disp1));
		write(0,disp2,strlen(disp2));
		write(0,&disp3[lg],1);
		write(0,disp4,strlen(disp4));
		write(0,&disp5[lg%2],1);
		lg++;
		if(lg==strlen(disp3))
		lg=0;
*/

  fs.block_number = data_block; /* blocks */

  if (fs.block_number != 0)
  {

    /* 
       First thought some big file are unable to recover since they might be using
       fragment blocks.So i let go.Just in final hours i found something you can
       call its as a bug  but for me it's big breakthough.

       Here we just can't go and get blocks sometimes blocks might be scattered around
       the disk.So make sure that you access  data from all groups.
       So Always keep a close eye on block number and it's Group Descriptor

     */

    if (gd == 0)
      i = eye_on_gd ();
    if (i == -1)
    {
      printf ("\n\tAbnormal : Group Descriptor not found ");
      return 0;
    }

    /* Calling eye_on_gd() affect performance so call when it's needed absoultely */

    if (fs.block_number > (fs.block_bitmap + fs.blocks_per_group) ||
        fs.block_number < (fs.block_bitmap))
      i = eye_on_gd ();
    if (i == -1)
    {
      printf ("\n\tAbnormal : Group Descriptor not found %lu", fs.block_number);
      return 0;
    }

    device_block_number =
      ((fs.block_number - 1) % fs.blocks_per_group) +
      fs.group_number * fs.blocks_per_group;
    device_block_number += fs.first_data_block;
    fs.content_offset =
      (unsigned long long) device_block_number *(unsigned long long) fs.block_size;
    fs.content_offset += fs.block_size;
    //set devicefile to appropriate address
    lseek64 (fd, fs.content_offset, 0);

    read_show_file (fd, fp);

    if (fileoverwritten == 444)
    {                           //Check for fileoverwritten ???
      return 0;
    }

    if (size == 0 || fs.block_number == giis_f.info.last_data_block)
    {


      if ((size == 0) && (fs.block_number == giis_f.info.last_data_block)
          && (err_size == giis_f.info.file_size))
      {
        return 0;               /* fine */
      }

      else
      {
        printf ("\n\n\tWarning : %s size differs\n\t", giis_f.info.name);
        printf ("Actual Size(Approx):%lu MB", ((giis_f.info.file_size / 1024) / 1024));
        printf ("\n\tRecovered Only(Approx):%lu MB ", ((err_size / 1024) / 1024));
        return 0;
      }
    }
    return 444;                 /*... nothing special. my lucky number */

  }

  return JSJ_NO_RETURN;
}



/*
* force_giis : Tries to recover the requested file. 
* giis4.1:
* concept of version is implemented.
*/


int force_giis ()
{
  int fp, i, exists;
  repeat = 0;                   /* If exists == repeat latest version of a file */
 

	  
  printf ("\n ->Get Latest Version or First Version of Deleted File ??");
  printf ("\n\tPress 0: Get Latest Version (Recommended)\n\tPress 1: First Version ");
  printf ("\n\t Enter your choice :");
  scanf ("%d", &get_version);
  if (get_version != 0 && get_version != 1)
  {
    get_version = 0;            //default version is latest .
  }

  printf ("\nForcing giis-Warning:Recovered file may be a corrupted one.");
  printf ("\nWitch/Which File ? : Enter Filename \t :");
  scanf ("%s", search_file);

  fp = open (FILE_INFO_FILE, 0);
  if (fp == -1)
  {
    perror ("open");
    return -1;
  }

  i = read (fp, giis_f.buffer, GIIS_FSIZE);
  s_offy = d_offy = 0;

  while (i > 0)
  {

    /* update s_offy & d_offy */

    if (giis_f.info.is_offset)
      s_offy += giis_f.info.is_offset;

    if (giis_f.info.id_offset)
      d_offy += giis_f.info.id_offset;


    if (strcmp (search_file, giis_f.info.name) == 0)
    {
      /* Check how many entries are available in giis system file */
      exists = file_repeated (giis_f.info.name);

      repeat++;                 /* We created version. 'repeat' of this file */
      /* Get Lastest Version */
      if (get_version == 0)
      {
        if (repeat == exists)
        {
          /* Fetch the last updated version of deleted file */
          get_it_i_say ();      /*Never mind about others go and get */
          close (fp);
          printf ("\n\n\tRecovered       :/giis/got_it");
          printf ("\n\tUnrecovered Files :/giis/unrecovered");
          return 1;
        }
      }
      if (get_version == 1)
      {                         /*Get First version */
        get_it_i_say ();        /*Never mind about others go and get */
        close (fp);
        printf ("\n\n\tRecovered       :/giis/got_it");
        printf ("\n\tUnrecovered Files :/giis/unrecovered");
        return 1;
      }

    }

    i = read (fp, giis_f.buffer, GIIS_FSIZE);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
  }
  printf ("\n\t%s File not found", search_file);
  close (fp);
  return 1;
}

/*
* recover_restore : Tries to recover the requested file and copies recovered files to it's original 
* directories and creates symbolic link under /giis/got_it . If unable to find original parent path,
* files will be placed under /giis/got_it.
*/


int recover_restore ()
{
int i=0,j=0,retval=0,total=0,fp;
 extern union u_s_rere_info giis_r;
char homepath[250]="/giis/got_it/";
char *disp1="Recovered File Name \t\t Restore Directory\n";
char *disp2=" \t\t\t";
char *disp3="\n";
if(rere_count==0) //Nothing is recovered
return 1;


/* Load inodes and files into global structure.*/
total=display_deleted_files();


/* File which contains details about where the files are restored back to */

fp = open (GIIS_RERE_FILE, 2);
  if (fp == -1)
  {
    perror ("open:");
    printf ("\nError No:%d", errno);
    close (fp);
    return -1;
  }
write(fp,disp1,strlen(disp1));

/* Restore */
for(i=0;i<rere_count;i++){

		for(j=0;j<total;j++){
			if(rere_inodes[i]==giis_r.info.list_of_deleted_inodes[j]){

			strcat(strcat(giis_r.info.list_of_restorepaths[j],"/"),rere_filenames[i]);
			memset(homepath,'\0',250);
			strcat(homepath,"/giis/got_it/");

			retval=link(strcat(homepath,rere_filenames[i]),giis_r.info.list_of_restorepaths[j]);
			if(retval<0){
			//printf("\nretval=%d",retval);
			//perror("");
			}else{
			/* Store file and restore paths */
			write(fp,rere_filenames[i],strlen(rere_filenames[i]));
			write(fp,disp2,strlen(disp2));
		write(fp,giis_r.info.list_of_restorepaths[j],strlen(giis_r.info.list_of_restorepaths[j]));
			write(fp,disp3,strlen(disp3));
			}
			
			}
		
		}
}
printf("\n See  /giis/restore_details.txt for Successfully restored files\n");
close(fp);
printf ("\n\n\tCheck /giis/got_it For all Recovered Files.\n");
return 1;
  
}
