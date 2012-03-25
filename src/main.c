/*
* /giis/main.c The main procedure call.
*
* Copyright (C) 2005. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/

#include "giis.h"

/*
* main() Responds to user inputs and calls appropriate procedure.
*/

int main (int argc, char *argv[])
{
  int i, ans = -1, fp, line = 1;
  char a, x;
  char sumkey[] = "Enter Any key to Continue :-)";
  extern int rere;
  extern int install_mode;/* 0 for prompting user and 1 for using config file */
  extern int date_mode,day,month,year; /* Time based recovery */
  date_mode=-1;
  
  do
  {
    if (argc > 2)
    {
      printf ("giis : invalid option \n");
      printf ("Type 'giis -h' for help \n");
      return -1;
    }

    /* process command line */

    if (argc == 2)
    {
      if (strcmp (argv[1], "-m") == 0)  /* menu */
        ans = 444;
      if (strcmp (argv[1], "-i") == 0) {/* -i will prompt user during installation ,-c will use config file */
        ans = 0; 
	install_mode=0;
	}
      if (strcmp (argv[1], "-c") == 0){
	ans = 0;
	install_mode=1;
	}if (strcmp (argv[1], "-u") == 0)  /* update */
        ans = 1;
      if (strcmp (argv[1], "-g") == 0)  /* recover */
        ans = 2;
      if (strcmp (argv[1], "-d") == 0)  /* System info. */
        ans = 3;
      if (strcmp (argv[1], "-f") == 0)  /* giis info. */
        ans = 4;
      if (strcmp (argv[1], "-r") == 0)  /* uninstall */
        ans = 5;
      if (strcmp (argv[1], "-a") == 0)  /* author */
        ans = 6;
      if (strcmp (argv[1], "-t") == 0)  /* quotes */
        ans = 7;
      if (strcmp (argv[1], "-q") == 0)  /* quits */
        ans = 8;

      if (strcmp (argv[1], "-h") == 0)
      {                         /* help */
	printf ("giis Version 4.6.1,  25 March 2012");
	printf ("\nBy Lakshmipathi.G (www.giis.co.in)");
	printf("\nThis software comes with ABSOLUTELY NO WARRANTY.");
        printf ("\nUsage: giis [OPTION]\n");
        printf ("\nPossible options available with giis :");
        printf ("\n  -a\t\t Info. about the author.");
        printf ("\n  -d\t\t Displays your file system informations.");
        printf ("\n  -f\t\t Displays giis system files.");
        printf ("\n  -g\t\t Recovers the Given or all deleted files.");
        printf ("\n  -h\t\t Prints this menu.");
        printf ("\n  -i\t\t Installation of giis in Interactive mode.");
	printf ("\n  -c\t\t Installation of giis using Configuation file.");
        printf ("\n  -m\t\t Menu based user interaction.");
        printf ("\n  -q\t\t Exit giis");
        printf ("\n  -t\t\t Some quotes and thoughts.");
        printf ("\n  -u\t\t Updates giis to reflect current status.");
        printf ("\n  -r\t\t Uninstalls giis.\n");
        printf ("\nPlease report bugs to http://groups.google.com/group/giis-users\n");
        return 0;
      }
      if ((ans != 444) && (ans < 0 || ans > 8))
      {
        printf ("giis : invalid option \n");
        printf ("Type 'giis -h' for help.\n");
        return -1;
      }


    }
    if (argc == 1)              /* default is menu. */
      ans = 444;

    if (ans == 444)
    {
      system ("clear");
      printf ("\n\t\t ===================================================");
      printf ("\n\t\t  gET iT i sAY. giis - Ext3/Ext2 File Undelete Tool ");
      printf ("\n\t\t ===================================================\n");
      printf ("\n\tPress 0 : Install \n\tPress 1 : Update Now\n\tPress 2 : File Recovery");
      printf ("\n\tPress 3 : Your File System Details");
      printf ("\n\tPress 4 : giis File Details \n\tPress 5 : Uninstall");
      printf ("\n\tPress 6 : About giis ");
      printf ("\n\tPress 7 : Quotes \n\tPress 8 : Exit");
      printf ("\n\n\t\tEnter Your choice : ");
      scanf ("%d", &ans);
    }

    if (ans == 4)
    {
      fp = open (INSTALL_DIR1, 0);
      if (errno == ENOENT)
      {
        printf ("\n\t\tgiis Not installed...\n");
        printf ("\t\t\t...So install giis and then try this..\n\n ");
        close (fp);
        return -1;
      }
      close (fp);

      i = read_dir_info_file ();
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      printf ("\n File Info File");
      i = read_file_info_file ();
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }

      return 0;
    }


    if (ans == 8)
    {
      printf ("\n\t Quiting giis....");
      printf ("\n\n\t Bye...Bye...!!!\n\n");
      return 0;
    }

    if (ans == 5)
    {
      i = remove_giis ();
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      return 0;
    }

    if (ans == 6)
    {

      system ("clear");
      fp = open ("/usr/share/giis/hai", 0);
      if (fp == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      i = read (fp, &a, 1);
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      while (i > 0)
      {
        write (1, &a, 1);
        i = read (fp, &a, 1);
      }
      close (fp);
      return 0;
    }
    if (ans == 7)
    {

      system ("clear");
      fp = open ("/usr/share/giis/quotes", 0);
      if (fp == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      i = read (fp, &a, 1);
      if (i == -1)
      {
        perror ("");
        printf ("Error Number:%d", errno);
        return -1;
      }
      while (i > 0)
      {
        if (a == '\n')
          line++;
        write (1, &a, 1);
        i = read (fp, &a, 1);
        if ((line % 24) == 0)
        {
          line++;
          write (1, sumkey, strlen (sumkey));
          read (0, &x, 1);
        }
      }
      close (fp);
      return 0;
    }




  }
  while (!(ans >= 0 && ans <= 8));

  /* Check for installation */
  if (ans == 0)
  {
    install = 1;
    fp = open (INSTALL_DIR1, 0);
    if (fp != -1)
    {
      printf ("\n\t\tgiis already installed in your system...\n");
      printf ("\t\tTry giis -r or manually delete /usr/share/giis to install freshly...\n ");
      close (fp);
      return -1;
    }
  }

  if (ans == 1 || ans == 2 || ans == 3)
  {
    fp = open (INSTALL_DIR1, 0);
    if (errno == ENOENT)
    {
      printf ("\n\t\tgiis Not installed...\n");
      printf ("\t\t\t...So install giis and then try this ;)\n ");
      close (fp);
      return -1;
    }
    close (fp);
  }

  printf ("\nSearching for file system....");
  if (init () == -1)
  {
    printf ("\n\nFile System not is Initialized....Aborting....\n");
    exit (1);
  }

  if (ans == 0)
  {
    install = 1;
    update_file = 0;            /* Used to call update_dir_info_file() */
    update_dir = 0;
    fs.inode_number = ROOT_INODE;       /* Get Root inode offset to fs.root_inode_offset.First. */
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
    {                           /* If inode is a directory then Show its content  */
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
    else
      {
        printf("\n **giis panic ** root inode not detected..exiting\n");
	exit(0);
      }
	return 0;
  }

  /* Display File System Details */

  if (ans == 3)
  {
    fs.inode_number = ROOT_INODE;       /* Get Root inode offset */
    i = find_inode_offset ();
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    i = fs_display ();
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    return 0;
  }


  if (ans == 0 || ans == 1)
  {
    if (ans == 1)
    {
	printf("\n\tgiis update :Please wait this process may take some time to complete... \n");
      install = 0;
      update_file = 1;
      update_dir = 1;
    }

    i = search4dir ();          /* searches for directory in DIR_INFO_FILE */
    if (i == -1)
    {
      perror ("");
      printf ("Error Number:%d", errno);
      return -1;
    }
    if (ans == 0)
    {
	//Set device name :
      search4fs();
      printf("\n\t\t Extracting Sample data...");
      get_sample_data ();
	
      printf ("\n\n\t\tgiis - Installation completed...");
    }
  }


  if (ans == 2)
  {

  printf ("\n\n\t *** Note : 'Recover and Restore Files' option will place  recovered files into it's original\ parent directory and creates  link  under /usr/share/giis/got_it directory.If the original directory path not\ exists,then those files can be found under /usr/share/giis/got_it.See /usr/share/giis/restore_details.txt for restored files.");
  printf("\n\n\t==>Recover and Restore  Files : Press 1(Recommended) Else Press 0 : ");
	
  scanf("%d",&rere);


    printf ("\n\nHow do you want to recover files?");
    printf ("\n\t Press 0 : Get All User files ");
    printf ("\n\t Press 1 : Get Specific User files");
    printf ("\n\t Press 2 : force giis - giis dump");
    printf ("\n\t Press 3 : Get By File Format");
    printf ("\n\t Press 4 : List Deleted files");
    printf ("\n\t Press 5 : Get files by Deleted date\n");
    printf ("\n\t\tEnter Your Choice : ");
    scanf ("%d", &ans);
    if(ans == 4){
	display_deleted_files();
	exit(0);
	}
/* Get files by date */
    if(ans == 5){
	up:
	printf("\n\nGet Files by Deleted Date:\n\tPress 0 : Deleted on\n\tPress 1 : Deleted After\n\tPress 2 : Deleted Before \n\tPress 3 : Deleted Between");
	printf("\n\n\t\tEnter Your Choice :");
	scanf("%d",&date_mode);

	if((date_mode<0) || (date_mode>3)){
	printf("\n Please Enter Valid Choice.");
	goto up;
	}

	if(date_mode == 0 || date_mode == 1 || date_mode == 2 ){
	printf("\n Enter the day (1..31) :");
	scanf("%d",&day);
	printf("\n Enter the month (1..12) : ");
	scanf("%d",&month);
	printf("\n Enter the year (YYYY) :");
	scanf("%d",&year);
	}else if(date_mode == 3){
	printf("\n Enter From-Date Details :");
	printf("\n Enter the day (1..31) :");
	scanf("%d",&day);
	printf("\n Enter the month (1..12) : ");
	scanf("%d",&month);
	printf("\n Enter the year (YYYY) :");
	scanf("%d",&year);

	printf("\n Enter To-Date Details :");
	printf("\n Enter the day (1..31) :");
	scanf("%d",&day1);
	printf("\n Enter the month (1..12) : ");
	scanf("%d",&month1);
	printf("\n Enter the year (YYYY) :");
	scanf("%d",&year1);
	
	}
	// date validation
	if(((day<=0 || day>31)||(month<=0 || month>12)||(year<1000))||((date_mode == 3)&&(day1<=0 || day1>31 || month1<=0 || month1>12|| year1<1000 )))
	{
	printf("\n Please Enter Valid Date.");
	goto up;
	}
	//Now recover all files 
	ans=0;
	}

    if (ans >= 0 && ans <= 3)
      use_uid = ans;
    else
    {
      printf ("\n\n\t\tPlease Enter correct Choice");
      return -1;
    }

    install = 0;
    get_it ();                  /* Search for deleted file */

    /*giis 4.3 :Recover and restore */
   if(rere==1)
	recover_restore();
  }
  return 0;
}
