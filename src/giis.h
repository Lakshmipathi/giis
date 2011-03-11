/*
* /giis/giis.h - The header file.
*
* gET iT i sAY-giis - EXT3/EXT2 File Undelete Tool. Version 4.4
* 
* Copyright (C) 2009. G.Lakshmipathi.<lakshmipathi.g@giis.co.in> <lakshmipathi.g@gmail.com>
*
*
*/
#ifndef GIIS_H
#define GIIS_H

        /* List of files to be included */
#define _LARGEFILE64_SOURCE     /* lseek64() required this */
#define _GNU_SOURCE

#include <sys/types.h>
#include <linux/ext3_fs.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

	/* Pre-defined macro */
	
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	
#define O_EXCL		   0200	
#define O_TRUNC		  01000	


		/* Some macro's used for reading clarity */

#define JSJ_NO_RETURN 0    /* for routine that need a return code - default to zero */

/* giis-4.6 LEVEL_VALUE from /etc/giis.conf will be used.
#define LEVEL_VALUE  15	 //Supports upto 15 levels of sub-dir.(dir1/dir2/dir3.../dir15) 
*/
#define INSTALL_DIR1   "/giis"
#define INSTALL_DIR2   "/giis/sys"
#define INSTALL_DIR3   "/giis/got_it"			/* Recovered files */
#define INSTALL_DIR4	"/giis/unrecovered" 		/* Unrecovered Files List */

#define DIR_INFO_FILE  "/giis/sys/dir"			/* Directory informations*/
#define FILE_INFO_FILE "/giis/sys/file"			/* File informations */
#define SIND_INFO_FILE "/giis/sys/sind"			/* single indirect blocks */
#define DIND_INFO_FILE "/giis/sys/dind"			/* Double indirect blocks */
#define SAMPLE_DATA_FILE "/giis/sys/sample"		/* Sample Data File */
#define GIIS_RERE_FILE	"/giis/restore_details.txt"	/* File contains info about restore paths */
#define GIIS_CONF_FILE  "/etc/giis.conf"		/* config file*/

#define DEFAULT_OFFSET_OF_SUPER_BLOCK 1024 		/* Default offset */
#define ACCESS 0 					/* Read  only */
#define SUPER_BLOCK_SIZE sizeof(struct ext3_super_block)
/* #define INODE_SIZE 128 */ // Get from SB
#define GROUP_DESC_SIZE sizeof(struct ext3_group_desc)
#define DIR_SIZE sizeof(struct ext3_dir_entry_2)
#define GIIS_DSIZE sizeof(struct s_dir_recover_info)
#define GIIS_FSIZE sizeof(struct s_file_recover_info)
#define GIIS_SSIZE sizeof(struct s_sample_file_data)

#define ROOT_INODE 2
#define RANGE 4294967295UL	
#define MAX_FILE_LENGTH 64


	/* Buffers which will be loaded with corresponding structures */

union get_ext_super_block{
	struct ext3_super_block sb;
	char buffer[sizeof(struct ext3_super_block)];
	}isb;

union get_ext_group_desc{
	struct ext3_group_desc gd;
	char buffer[sizeof(struct ext3_group_desc)];
	}igd;

union get_ext_inode{
	struct ext3_inode in;
	char buffer[sizeof(struct ext3_inode)];
	}iin;
union get_ext_dir{
	struct ext3_dir_entry_2 de;
	char buffer[sizeof(struct ext3_dir_entry_2)];
	}ide;

				/* Contains file system Infomations */
struct file_system{
	
					/* Group Specific */
unsigned long group_number;
unsigned long block_bitmap;
unsigned long inode_bitmap;
unsigned long inode_table;
unsigned long free_blocks_count;
unsigned long free_inodes_count;
					 /*General info*/
unsigned long inode_number;
unsigned long block_number;
unsigned long super_block_offset;
unsigned long  group_offset;
unsigned long long inode_offset;	/* Consist of current inodes offset */
unsigned long long root_inode_offset;
unsigned long long device_offset;
unsigned long long content_offset;
unsigned long index_number;		/* Index number in Inode Table of Group Descriptor */
					/*Obtained info */
unsigned long inodes_per_group;
unsigned long blocks_per_group;
int groups_count;			/* Total Number of Group Descriptor */
unsigned long long inodes_count;	/* Total Number of inodes in Group Descriptor */
unsigned long long blocks_count;	/* Total Number of blocks in Group Descriptor */
unsigned long long fs_size; 		/* File system size */
int inode_table_size;
int inodes_per_block;
int  block_size;
int first_data_block;
int first_group_desc;
int inode_size;				/* Get it from SB */
}fs;
				
				/* information for recovery */

union u_file_recover_info{
struct s_file_recover_info{
	char            date_time[30];		  /* When this entry recorded */	
	char            name[64];		  /* filename */		
	unsigned long	inode_number;		  /* inode number */
	unsigned long   parent_inode_number;	  /* Directory inode number */
	int             search_flag;		  /* Search flags*/	
	uid_t        mode;			  /* File type */
	uid_t        owner;		  	  /* owner's uid */	
	unsigned long	file_blocks_count;	  /* Blocks count */
	unsigned long	file_size;		
	unsigned long	file_flags;	
	unsigned long	data_block[EXT3_N_BLOCKS];/* Contents pointer */
	unsigned long   is_offset; /* Single Indirect Block offset */	
	unsigned long   id_offset; /* Double indirect Block offset */
	int             sfragment_flag;		  /* Which to use */
	int 		dfragment_flag;
	unsigned long   last_data_block;/* Block number where last data remains */
	}info;
char buffer[sizeof(struct s_file_recover_info)];
}giis_f;


union u_dir_recover_info{
struct s_dir_recover_info{
	char  		date_time[30];		  /* When this entry recorded */	
	char            name[64];		  /* Directory	name */
	unsigned long	inode_number;		  /* inode number */
	unsigned long   parent_inode_number;	  /* Directory inode number */
	int 		search_flag;		  /* Indicates entry search or not */	
	}info;
char buffer[sizeof(struct s_dir_recover_info)];
}giis_d,giis_dt;	


union u_sample_file_data{
struct s_sample_file_data{
	unsigned long	inode_number;		  /* inode number */
	char sdata[32];			  /* Sample content of file */
	}info;
char buffer[sizeof(struct s_sample_file_data)];
}giis_s;

/* Recover and Restore */
union u_s_rere_info{
struct s_rere_info{
unsigned long list_of_deleted_inodes[2048];		/* For Recover and Restore */
char list_of_restorepaths[150][150];				/* For Recover and Restore */
	}info;
char buffer[sizeof(struct s_rere_info)];
}giis_r;


		/* Single or Double Indirect */
unsigned long idata_block[1024];

struct passwd *pwfile;  			 /* Used for uid verfication */



			/*Global Variables */
int gi_round,hole,gd;
char device_name[100],user[25],search_file[30];
char auto_time[5];				/* Time value from conf file */
int auto_time2;					/* Converted time value */
char s_level_value[5];				/* Level value from conf file */
int level_value;				/* Converted Level value */

int dir,use_uid,install,fd;
int install_file;
int update_file,update_dir;			/* Required during updates */
int entry,found;				/* Used in dir.c while recording data */
time_t result;					/* time n date  used to ? */
char date_time[25];				/* specific when entries recorded */
unsigned long long deleted_inode_offset;	/* Deleted inode's offset */
unsigned long size,err_size;			/* File size used to read a file */
unsigned long home_inode;			/* inode of /home */
unsigned long root_inode;			/* inode of /root */
unsigned long s_offy;				/* SIND offset */
unsigned long d_offy;				/* DIND offset */
int dir_level;					/* Directory depth */
int fileoverwritten;				/* File modified or overwritten */
int search_format;				/* Get files using its format */
int startbyte;					/* Used in file.c */
int get_version;				/* version */
int repeat;              			/* For check */
int parent_found;				/* Used while displaying list of deleted files */
char pathname[150];				/* Same as above */
int rere;					/* Recover and Restore */
char rere_filenames[200][150];			/* To hold file names */
int rere_count;					/* To hold the count */
unsigned long rere_inodes[1024];		/* To hold inodes */
char g_directory[100][100];
int g_row;					/* directories count from config file*/
int new_dir_added;				/* To be used in update to include all sub-dir*/
int large_dir;					/* For checking large directories */
int dir_seq;
int large_dir_count;
int install_mode;				/* supports config file setup */
int day,month,year,day1,month1,year1;		/* Get by date */
int date_mode;
time_t result1;					/* time n date  used for Time based recovery*/
int test_count1,test_count2;


			/* Function protocols */
		
				/* dir.c */
			
int set_content_offset();			/* Sets offset to content of inode */	
int read_dir();					/* Load  dirctory  into buffer*/
int record_dir(unsigned long);			/* Writes data into DIR_INFO_FILE */
int record_file(unsigned long);			/* Writes data into FILE_INFO_FILE */
int show_dir();					/* Display directory */

				/* file.c */
			
int read_show_file(int,int);		        /* Read a file  */
int read_dir_info_file();			/* Reads data from DIR_INFO FILE */
int read_file_info_file();			/* Reads data from FILE_INFO_FILE */
int read_sind_info_file();			/* Reads SIND_INFO_FILE */
int read_dind_info_file();			/* Reads DIND_INFO_FILE */
int read_sample_info_file(unsigned long);       /* Reads SAMPLE_DATA_FILE */
void show_data(char []);			/* Displays Given Buffer */
int file_repeated(char []);			/* No.of times entry exists in sys file */
int display_deleted_files();			/* List deleted file names */
int lookup(unsigned long);			/* Lookup parent of given inode */
int read_config_file(char []);			/* Get data from giis.conf file */
int read_config_file2();			/* Get data from conf file and store in global variable*/
int read_conf_file_inum();			/* Get's inode number of config files */
		
			/* get_it_i_say.c */
int get_it(); 					/* Search for all deleted file and recover */
int verify_inode(unsigned long);        	/* Verifies given inode is free or not */
int get_it_i_say();				/* Recovers the deleted  file */
int get_data_from_block(int,int,unsigned long);	/* Get data from block */
int force_giis();				/* Get specific file */
int recover_restore();				/* Restore the recovered files */


			/* group.c */
int get_group_desc();				/* Get request Group Desccriptor */
int eye_on_gd();				/* Keep track of group */
		
			/* init.c */
		
int search4fs();				/* Searches for root file system */
int search4super_block();     			/* Searches for superblock */
void set_file_system();				/* Set file_system for processing */
int search4group_desc();   			/* Searches for group descriptor */
int init();					/* Initialize Procedure */
int init_files();				/* Creates or opens files */
int fs_display();				/* Display FS info */

			/* inode.c */
		
int find_inode_offset();			/* Initialize offset of inode */
int read_inode();				/* Get inode from offset*/
int show_inode();				/* Display inode*/


			/* searchnupdate.c */
int search4dir();		         	/* searches for directory  */	
int update_dir_info_file();			/* Updates DIR_INFO_FILE */
int get_parent_inode_flag(unsigned long);	/* Get Parent of given inode */
int search4sequence12();			/* Searches for continuous blocks */
int search4sequence13();			/* Searches for continuous blocks */
int avoid_dups();				/* Removes duplicates */
int call2remove(int);				/* Helper to above */
int check4file_dups(unsigned long);		/* Check for duplicates */
int check4dir_dups(unsigned long);		/* Check for duplicates */
int dir_depth(unsigned long );			/* Get directory depth */
int get_sample_data();				/* Get file content */ 	
int check4samplefile_dups(unsigned long);	/* Avoids duplication of sample data */

			/* uninit.c */
int remove_giis();	  			/* Uninstalls giis */


#endif /* GIIS_H */
