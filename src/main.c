/*  
 * MySQL dump parser
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compile_date_time.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#define     MAX_CHARS       255
#define		BUILD_DAY		COMPILE_DAY
#define		BUILD_MONTH		COMPILE_MONTH
#define		BUILD_YEAR		COMPILE_YEAR

void print_help(FILE* f)
{
  fprintf(f, "MySQL dump parser - v.%s\n",VERSION);
  fprintf(f, "Build date:%02d.%02d.%04d\n",BUILD_DAY,BUILD_MONTH,BUILD_YEAR);
  fprintf(f, "\n");
  fprintf(f, "Using:\n");
  fprintf(f, "  %s [ -i include_tables] [-x exclude_tables] \n", PACKAGE);
  fprintf(f, "  Parameters include_tables and exclude_tables can be a regular expressions.\n");
  fprintf(f, "  Dump file is read from stdin and then filtred dump file will be written to the stdout.\n");
  fprintf(f, "  Dump file should have the same charset as the current locale.\n");
  fprintf(f, "\n");
  fprintf(f, "Example:\n");
  fprintf(f, "  mysql_dump_parser -i pma* -x pma_bookmarks \n");
  fprintf(f, "\n");
}



int check_string(char* str, char* regexp)
{
    regex_t regex;
    int reti;
    int ret_val;
    char msgbuf[100];

    /* Compile regular expression */
    reti = regcomp(&regex, regexp, 0);
    if( reti )
    { 
        fprintf(stderr, "Wrong regex (%s)\n", regexp); 
        exit(1); 
    }

    /* Execute regular expression */
    reti = regexec(&regex, str, 0, NULL, 0);
    if( !reti )
    {
        /* match */
        ret_val = 1;
    }
    else if( reti == REG_NOMATCH )
    {
        /* no match */
        ret_val = 0;
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }

    /* Free compiled regular expression */ 
    regfree(&regex);
    return ret_val;
}


int check_table1(char* str)
{
    return check_string(str, "^--");
}


int check_table2(char* str)
{
    return check_string( str, "^-- Table structure for table");
}

int check_table3(char* str)
{
    return check_table1(str);
}


#define FIND_TABLE_END_OF_FILE 1
#define FIND_TABLE_OK 2
int find_table(char* name, int size, int show_content)
{
    char str[MAX_CHARS];
    char step0[MAX_CHARS];
    char step1[MAX_CHARS];
    int num_buf_str=0;
    int res = 0;
    int step = 0;

    while(!res)
    {
        if (fgets( str, MAX_CHARS, stdin)!=NULL)
        {
           switch(step)
           {
                case 0:
                    if (check_table1(str))
                    {
                        strncpy(step0, str, MAX_CHARS);
                        num_buf_str=1;
                        step++;
                    }
                    break;
                case 1:
                    if (check_table2(str))
                    {
                        char * pch; 
                        strtok (str,"`"); /* skip first substring */
                        pch = strtok (NULL,"`");
                        if (pch)
                        {
                            strncpy(name, pch, size);
                            step++;
                            strncpy(step1, str, MAX_CHARS);
                            num_buf_str = 2;
                        }
                        else
                            step =0;
                    }
                    else 
                        step = 0;
                    break;
                case 2:    
                    if (check_table3(str))
                    {
                        step=2;
                        res = FIND_TABLE_OK;
                    }
                    else step = 0;
                    break;
                default:
                    step = 0;
                    break;
           }
           if ((!step) && (show_content))
           {
               if (num_buf_str==2) printf( "%s", step1 );
               if (num_buf_str==1) printf( "%s", step0 );
               num_buf_str = 0;
               printf("%s", str);
           }
        }
        else
        {
           if (show_content)
           {
               if (num_buf_str==2) printf( "%s", step1 );
               if (num_buf_str==1) printf( "%s", step0 );
               num_buf_str = 0;
           }
           
           res = FIND_TABLE_END_OF_FILE;
        }
    }
    return res;
}

void    print_table_header(char* table)
{
    printf("--\n");
    printf("-- Table structure for table `%s`\n", table);
    printf("--\n");
}



int table_include(char* table, char* regexp)
{
    if (!regexp) return 1;
    return check_string( table, regexp );
}

int table_exclude(char* table, char* regexp)
{
    if (!regexp) return 0;
    return check_string( table, regexp );
}



void filter_db_dump(char* include_regexp, char* exclude_regexp)
{
    char table_name[MAX_CHARS];
    int  show = 1;

    for(;;)
    {
        if (find_table(table_name, MAX_CHARS, show)==FIND_TABLE_OK) 
        {
            if (table_include( table_name, include_regexp) && !table_exclude( table_name, exclude_regexp))
            {
                print_table_header(table_name);
                show = 1;
            }
            else show = 0;
        }
        else break;
    }
}



void    parse_command_line(char** include_regexp,char** exclude_regexp, int argc, char* argv[])
{
    int  res=0;

    /* parse incomming arguments */
	while ( (res = getopt(argc,argv,"i:x:h")) != -1)
    {
		switch (res)
        {
            case 'i': 
                      *include_regexp = optarg;
                      break;
            case 'x': 
                      *exclude_regexp = optarg;
                      break;
            case 'h': 
                      print_help(stdout); 
                      exit(0); 
                      break;
            case '?': 
                      printf("Error input argument '%s'\n\n", argv[optind]); 
                      print_help(stderr); 
                      exit(1); 
                      break;
        };
	};

    /* there is no input files. Only filter parameters */
    if ( argc != optind)
    {
        fprintf(stderr, "Error input argument: '%s'\n\n", argv[optind]);
        print_help(stderr);
        exit(1);
    };
}


int main(int argc, char* argv[])
{
    char*   include_regexp = NULL; 
    char*   exclude_regexp = NULL;

    parse_command_line(&include_regexp, &exclude_regexp, argc,  argv );
    filter_db_dump(     include_regexp,  exclude_regexp );
	return 0;
}

