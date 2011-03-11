/*
* /giis/uninit.c This file performs uninstall opertions.
*
* Copyright (C) 2005. G.Lakshmipathi.<lakshmipathi.g@gmail.com>
*
*/


#include "giis.h"


/*
* remove_giis() This procedure just unlinks files.
*/


int remove_giis ()
{
  int i;

  i = open (INSTALL_DIR1, 0);
  if (i == -1)
  {
    printf ("\n\t\t Install giis...and then try this :-)\n\n");
    return 0;
  }
  printf ("\n\t\tPress 1 : Continue Uninstall\n\t\tPress 2 : Abort");
  printf ("\n\t\tType in :");
  scanf ("%d", &i);
  if (i != 1)
    return 0;


  /* Remove  files */

  i = unlink (DIR_INFO_FILE);
  if (i == 0)
    printf ("\n\t%s Removed", DIR_INFO_FILE);
  else
    printf ("\n\t%s not Deleted...", DIR_INFO_FILE);

  i = unlink (FILE_INFO_FILE);
  if (i == 0)
    printf ("\n\t%s Removed", FILE_INFO_FILE);
  else
    printf ("\n\t%s not Deleted...", FILE_INFO_FILE);

  i = unlink (SIND_INFO_FILE);
  if (i == 0)
    printf ("\n\t%s Removed", SIND_INFO_FILE);
  else
    printf ("\n\t%s not Deleted...", SIND_INFO_FILE);

  i = unlink (DIND_INFO_FILE);

  if (i == 0)
    printf ("\n\t%s Removed", DIND_INFO_FILE);
  else
    printf ("\n\t%s not Deleted...", DIND_INFO_FILE);


  i = unlink (SAMPLE_DATA_FILE);

  if (i == 0)
    printf ("\n\t%s Removed", SAMPLE_DATA_FILE);
  else
    printf ("\n\t%s not Deleted...", SAMPLE_DATA_FILE);

  //Remove the executable file.

  i = unlink ("/usr/bin/giis");
  if (i == 0)
    printf ("\n\t/usr/bin/giis Removed");
  else
    printf ("\n\t/usr/bin/giis not Deleted...");



  i = unlink ("/giis/hai");
  if (i == 0)
    printf ("\n\t/giis/hai Removed");
  else
    printf ("\n\t/giis/hai not Deleted...");


  i = unlink ("/giis/quotes");
  if (i == 0)
    printf ("\n\t/giis/quotes Removed");
  else
    printf ("\n\t/giis/quotes not Deleted...");



  i = unlink ("/giis/restore_details.txt");
  if (i == 0)
    printf ("\n\t/giis/restore_details.txt Removed");
  else
    printf ("\n\t/giis/restore_details.txt not Deleted...");

  rmdir (INSTALL_DIR3);
  rmdir (INSTALL_DIR2);
  rmdir (INSTALL_DIR4);
  i = rmdir (INSTALL_DIR1);
  if (i != 0)
    printf ("\n\t **Unable to Delete %s\n", INSTALL_DIR1);
  else
    printf ("\n\t **All Files Removed...\n\t***gET iT i sAY.giis-Uninstalled Successfully***\n");
  return 1;
}
