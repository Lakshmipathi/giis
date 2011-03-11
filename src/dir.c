/*
* /giis/dir.c This files deals with gathering vital infomations need for recovery.
*
* Copyright (C) 2006. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include "giis.h"

/*
* set_content_offset() will first set offset of directory and then read it into buffer 
*/

int set_content_offset ()
{
int i;
  fs.block_number = iin.in.i_block[0];
  fs.content_offset = (unsigned long long) fs.block_number * fs.block_size;
  lseek64 (fd, fs.content_offset, 0);
  //giis4.4
  dir_seq=1;large_dir=0;
  if(iin.in.i_size > fs.block_size){
	
        large_dir=1;
        large_dir_count=iin.in.i_size/fs.block_size;
        for(i=0;i<(large_dir_count-1);i++){
        if((iin.in.i_block[i+1]-iin.in.i_block[i]) == 1){
        dir_seq++;
	}
        else
        break;
	}
        }else{
        large_dir=0;
        }
  return 1;

}

/*
* read_dir() Loads directory into buffer 
*/

int read_dir ()
{
  int i;
  i = read (fd, ide.buffer, DIR_SIZE);
  if (i == -1)
  {
    perror ("");
    printf ("Error Number:%d", errno);
    return -1;
  }

  return 1;
}

/*
* record_dir() enters directory informations into a file 
*
* Modification for giis2.1:
*
* Makes a call to check4dir_dups().
*
* Modification for giis4.2:
* Provides user an option to choose directory .
*
*/

int record_dir (unsigned long parent)
{
  int fp, i;
  int add_dir_option=-1; 
  int config_file=0;
	
  fp = open (DIR_INFO_FILE, 2);
  if (fp == -1)
  {
    perror ("open:");
    printf ("\nError No:%d", errno);
    close (fp);
    return -1;
  }

  lseek (fp, 0, 2);             /* Set file pointer to end */

  /*  Don't Call during installation. */

  if (install == 0)
  {
    i = check4dir_dups (parent);
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);close (fp);
      return -1;
    }
    if (i == 0)
    {       
                    /* Already we have it */
	
      close (fp);
      return 1;
    }
	
  }

  /* date n time */
  result = time (NULL);
  strcpy (date_time, asctime (localtime (&result)));
  strcpy (giis_d.info.date_time, date_time);

  if (entry == 0)
  {                             /* entry and found are Global */
    giis_d.info.inode_number = ide.de.inode;    /* current inode */
    for (i = 0; i < ide.de.name_len; i++)
      giis_d.info.name[i] = ide.de.name[i];
    giis_d.info.name[i] = '\0';
    close (fp);
    return 1;
  }
  if (entry == 1)
  {
    giis_d.info.parent_inode_number = ide.de.inode;
    giis_d.info.search_flag = 0;
    i = write (fp, giis_d.buffer, GIIS_DSIZE);
    if (i != GIIS_DSIZE)
    {
      perror ("write():");
      printf ("\nError No:%d", errno);
      close (fp);
      return -1;
    }
    close (fp);
    return 1;
  }
  if (entry >= 2 && !found)
    giis_d.info.parent_inode_number = giis_d.info.inode_number;
  found = 1;
  for (i = 0; i < ide.de.name_len; i++)
    giis_d.info.name[i] = ide.de.name[i];
  giis_d.info.name[i] = '\0';

  giis_d.info.inode_number = ide.de.inode;
  if (giis_d.info.name[0] == '.')
  {                             /* don't save system files except . and .. */
    close (fp);
    return 1;
  }
  giis_d.info.search_flag = 0;

  if ((strcmp (giis_d.info.name, "root") == 0))
  {
    giis_d.info.search_flag = 1;
    root_inode = giis_d.info.inode_number;
  }

  if ((strcmp (giis_d.info.name, "home") == 0))
  {
    giis_d.info.search_flag = 1;
    home_inode = giis_d.info.inode_number;
  }

// giis4.2 : Newly added directories 

	//Record everything under / on users choice 
	//Fix: use update_dir giis 4.3  to avaoid user being prompted during giis updation.
	//giis 4.4 install mode supports config file.
	if((install_mode == 0) && (update_dir == 0)&&(giis_d.info.parent_inode_number == ROOT_INODE)&&(strcmp (giis_d.info.name, "giis") != 0)&&(strcmp (giis_d.info.name, "home") != 0) && (strcmp (giis_d.info.name, "root") != 0)){
	printf("\n\n\t\t ==================");
	printf("\n\t\t directory : /%s ",giis_d.info.name);
	printf("\n\t\t ====================");
        printf("\n\t To Add this directory press  : 1 ");
	printf("\n\t To skip this directory press : 0\n");
	printf("\n\t Please Enter you choice :");
	scanf("%d",&add_dir_option);

	if(add_dir_option == 1){
	printf("\n\t\t /%s => Added",giis_d.info.name);
        giis_d.info.search_flag = 1;
	}
	else{
	printf("\n\t\t /%s => Skipped",giis_d.info.name);
	 giis_d.info.search_flag = 0;
	}
	}
/// giis-4.2 
  if (update_dir)
  {                             /* Record all dir found during updates */
    giis_d.info.search_flag = 1;
    giis_d.info.parent_inode_number = parent;

  }
	//Check directory List to added new
	config_file=read_config_file(giis_d.info.name);
	if((config_file==444)&&(giis_d.info.parent_inode_number==2)){
	printf("\n %s available in /etc/giis.conf ",giis_d.info.name);
	giis_d.info.search_flag=1;config_file=0;
	}

  i = write (fp, giis_d.buffer, GIIS_DSIZE);
  if (i != GIIS_DSIZE)
  {
    perror (" write()");
    printf ("\nError No:%d \t fp=%d", errno, fp);
    close (fp);
    return -1;
  }
  close (fp);
  return 1;
}

/*
* record_file() records  file informations necessary for recovery.
* file is system file which often begins with "." then don't record it for obvious reason.
* Since that's not created by any users.
*
* Modification for giis2.1:
*
* Makes a call to check4file_dups().
*/

int record_file (unsigned long parent)
{
  int fp, i;
  int len=0;
  static unsigned long parent_original=0;
/* giis 4.3 : Store system/hidden files too */	
  /*
    if (ide.de.name[0] == '.')
    return 1;
  */
if(ide.de.file_type==2){
parent_original=ide.de.inode;
}
  fp = open (FILE_INFO_FILE, 1);
  if (fp == -1)
  {
    perror ("open:");
    printf ("\nError No:%d", errno);
    close (fp);
    return -1;
  }
  lseek (fp, 0, 2);             /* Set file pointer to end */
  /* date n time */
  result = time (NULL);
  strcpy (date_time, asctime (localtime (&result)));
  strcpy (giis_f.info.date_time, date_time);


  /* Get  inode  details */

  fs.inode_number = ide.de.inode;
  i = find_inode_offset ();
  if (i == -1)
  {
    perror ("");
    printf ("dir.c:record_file()-find_inode_offset:Error Number:%d", errno);close (fp);
    return -1;
  }

  i = read_inode ();
  if (i == -1)
  {
    perror ("");
    printf ("dir.c:record_file()-read_inode:Error Number:%d", errno);close (fp);
    return -1;
  }

  /* Store files iff it's a new one or it's old one with some contents modified and
     Don't Called during installation. */
  if (install == 0)
  {
	/* if mtime not within last 20 mintues then ingore -no need to call avoid dups..
	auto_time2 compute from TIME value in giis.conf*/
	
	if(iin.in.i_mtime < (time(0)-auto_time2))
	{
	close (fp);
      	return 1;
	}
	
    i = check4file_dups (parent_original);
    if (i == -1)
    {
      perror ("");
      printf ("dir.c:record_file()-check4file_dups:Error Number:%d", errno);close (fp);
      return -1;
    }
    if (i == 0)
    {                           /* Already we have it */
      close (fp);
      return 1;
    }
  }

  for (i = 0; i < ide.de.name_len; i++)
    giis_f.info.name[i] = ide.de.name[i];
    giis_f.info.name[i] = '\0';

     len=i;
     if(len > MAX_FILE_LENGTH ){
	close (fp);
	return 1; //File name is too large-Just ignore these files.
	}
	

  giis_f.info.inode_number = ide.de.inode;
  giis_f.info.parent_inode_number = parent_original;     /*Same parent 4 all files in dir--but set it correctly :) */
  giis_f.info.mode = iin.in.i_mode;
  giis_f.info.owner = iin.in.i_uid;
  giis_f.info.file_blocks_count = iin.in.i_blocks;
  giis_f.info.file_size = iin.in.i_size;
  giis_f.info.file_flags = iin.in.i_flags;
  giis_f.info.search_flag = 0;
//reset recorded  time correctly
strcpy (giis_f.info.date_time, date_time); 


  giis_f.info.last_data_block = 0;
  for (i = 0; i < 15; i++)
  {
    giis_f.info.data_block[i] = iin.in.i_block[i];
    /* Store block number of files<48KB size */
    if (iin.in.i_block[i] == 0 && i <= 12 && !giis_f.info.last_data_block)
      giis_f.info.last_data_block = iin.in.i_block[i - 1];
  }

  /* Error check 4 some raw error check details */

  if (iin.in.i_block[0] == iin.in.i_block[1]){
   close(fp);
  return 1;
  }
  if (iin.in.i_size > 49152 && (iin.in.i_block[12] == 0)){
  close(fp);
  return 1;
  }
  if (iin.in.i_size < 49152 && (iin.in.i_block[12] != 0)){
  close(fp);
  return 1;
  }

  if (iin.in.i_block[14] != 0)  /* Not supported :-) */
  {
  close(fp);
  return 1;
  }



  /* Set flags */

  giis_f.info.sfragment_flag = 0;       /* Fragment level for single indirect */
  giis_f.info.dfragment_flag = 0;       /* Do the Same for double */
  giis_f.info.is_offset = 0;
  giis_f.info.id_offset = 0;


  if (iin.in.i_size >= 49152)
  {

    if (giis_f.info.data_block[12] != 0)
    {
      i = search4sequence12 ();
      if (i == -1)
      {
        perror ("");
        printf ("dir.c:record_file()-search4sequence12:Error Number:%d", errno);close (fp);
        return -1;
      }
    }

    if (giis_f.info.data_block[13] != 0)
    {

      gi_round = 0;
      hole = 0;                 /* used in search4sequence13 */
      giis_f.info.last_data_block = 0;
      i = search4sequence13 ();
      if (i == -1)
      {
        perror ("");
        printf ("dir.c:record_file()-search4sequence13:Error Number:%d", errno);close (fp);
        return -1;
      }
    }

  }
  
   i = write (fp, giis_f.buffer, GIIS_FSIZE);
  if (i != GIIS_FSIZE)
  {
    perror (" write():");
    printf ("\nError No:%d", errno);
    close (fp);
    return -1;
  }



  close (fp);
  return 1;
}

/*
* show_dir() displays the content of the requested  directory inode.
* And during installation stores file and directory entries.
*
* Modification for giis2:
*
* Introduce new variable  install_file with it's help we first store all directory 
* informations first. Once all directories are recorded then we start to record files
* from these directories.
*
* Modification for giis2.1:
*
* update_dir variable to update directories.
*/

int show_dir ()
{
  int i;
  unsigned long long temp_content_offset = 0, parent = 0;       /* So that we don't mess up original offset */
  temp_content_offset = fs.content_offset;
  int total_rec_len=0;//giis 4.3
 int process_block1=0,process_block2=0,process_block3=0,process_block4=0;
  int process_block5=0,process_block6=0,process_block7=0;
int process_block8=0,process_block9=0,process_block10=0;
  int process_block11=0;


if(large_dir==1 && dir_seq==1){

 process_block1=iin.in.i_block[1]-iin.in.i_block[0];
if(iin.in.i_block[2])
 process_block2=iin.in.i_block[2]-iin.in.i_block[1];
if(iin.in.i_block[3])
 process_block3=iin.in.i_block[3]-iin.in.i_block[2];
if(iin.in.i_block[4])
 process_block4=iin.in.i_block[4]-iin.in.i_block[3];

if(iin.in.i_block[5])
 process_block5=iin.in.i_block[5]-iin.in.i_block[4];
if(iin.in.i_block[6])
 process_block6=iin.in.i_block[6]-iin.in.i_block[5];
if(iin.in.i_block[7])
 process_block7=iin.in.i_block[7]-iin.in.i_block[6];

if(iin.in.i_block[8])
 process_block8=iin.in.i_block[8]-iin.in.i_block[7];
if(iin.in.i_block[9])
 process_block9=iin.in.i_block[9]-iin.in.i_block[8];
if(iin.in.i_block[10])
 process_block10=iin.in.i_block[10]-iin.in.i_block[9];
if(iin.in.i_block[11])
 process_block11=iin.in.i_block[11]-iin.in.i_block[10];

}

 
top:
  while ((ide.de.rec_len != 0) && (ide.de.rec_len % 4 == 0)&&total_rec_len<(fs.block_size*dir_seq))
  {
	 total_rec_len+=ide.de.rec_len;
	
    /*First entry is current directory inode */

    if ((entry == 0) && (ide.de.file_type == 2))
    {
      parent = ide.de.inode;
    }

    if (update_dir && (ide.de.file_type == 2))
    {
      record_dir (parent);
      entry++;
    }
    if (update_dir == 0)
    {

      if ((update_file == 0) && (install_file == 0) && (ide.de.file_type == 2))
      {
        record_dir (parent);    /*Install:Record data in DIR_INFO_FILE */
        entry++;
      }
	//giis 4.3 : Igonore sockets,symbolic links etc..except regular file and directory.

      if ((update_file || install_file) && (ide.de.file_type == 1 ||( ide.de.name_len==1 && ide.de.file_type == 2 && ide.de.name[0]=='.')))
      {
        /*Record data in FILE_INFO_FILE */
        record_file (parent);
      }
    }
    temp_content_offset += ide.de.rec_len;      /* Next entry */
    i = lseek64 (fd, temp_content_offset, 0);
    i = read (fd, ide.buffer, DIR_SIZE);
  }

	//if large_dir=1 and dir_seq=1 then it refers it's large directory but doesn't have data block in sequence
//only first data block is processed.to process others use process_block1,process_block2..
if((large_dir==1) && (dir_seq==1) &&( (process_block1!=0)||(process_block2!=0)||(process_block3!=0)||(process_block4!=0)||(process_block5!=0)
    ||(process_block6!=0)||(process_block7!=0)||(process_block8!=0)||(process_block9!=0)||(process_block10!=0)
    ||(process_block11!=0))){
total_rec_len=0;//reset

if(process_block1!=0){
temp_content_offset=temp_content_offset+(process_block1*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block1=0;
goto top;
}
if(process_block2!=0){
temp_content_offset=temp_content_offset+(process_block2*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block2=0;
goto top;
}
if(process_block3!=0){
temp_content_offset=temp_content_offset+(process_block3*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block3=0;
goto top;
}
if(process_block4!=0){
temp_content_offset=temp_content_offset+(process_block4*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block4=0;
goto top;
}


if(process_block5!=0){
temp_content_offset=temp_content_offset+(process_block5*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block5=0;
goto top;
}
if(process_block6!=0){
temp_content_offset=temp_content_offset+(process_block6*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block6=0;
goto top;
}
if(process_block7!=0){
temp_content_offset=temp_content_offset+(process_block7*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block7=0;
goto top;
}
if(process_block8!=0){
temp_content_offset=temp_content_offset+(process_block8*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block8=0;
goto top;
}
if(process_block9!=0){
temp_content_offset=temp_content_offset+(process_block9*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block9=0;
goto top;
}
if(process_block10!=0){
temp_content_offset=temp_content_offset+(process_block10*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block10=0;
goto top;
}
if(process_block11!=0){
temp_content_offset=temp_content_offset+(process_block11*fs.block_size);
i = lseek64 (fd, temp_content_offset, 0);
i = read (fd, ide.buffer, DIR_SIZE);process_block11=0;
goto top;
}



}//ends

  /* reset for next directories */
  entry = 0;
  found = 0;
  return 1;
}
