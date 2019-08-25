#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <ctype.h> /* isalnum, isdigit */ 

int hash(char* word, int cache_size)
{
   int ascii_sum=0;
   char *p;
   for (p = word; *p != '\0'; p++) 
   {
      ascii_sum += *p;
      //printf("pointing to %d, %d\n", *p, (1 + *p));
   }
   
   return (ascii_sum % cache_size);
}

int main( int argc, char** argv ) 
{

   setvbuf( stdout, NULL, _IONBF, 0 );
   
   /* Two additional command line arguments: size of the cache and the text file*/
   if ( argc != 3 )
   {
      fprintf( stderr, "ERROR: Invalid arguments\n" );
      fprintf( stderr, "USAGE: %s <cache size> <filename>\n", *(argv) );
      return EXIT_FAILURE;
   }
   /* Verify that cache size argument is an integer */
   for (int i=0; i < strlen(*(argv+1)); i++) 
   {
   	if (!isdigit(*(*(argv+1)+i)))
   	{
   	   fprintf( stderr, "ERROR: Invalid arguments\n" );
   	   fprintf( stderr, "USAGE: %s <cache size> <filename>\n", *(argv) );
   	   return EXIT_FAILURE;
   	}
   }
   
   int cache_size = atoi(*(argv+1));
   
   char ** cache;   /* array of char* */

   /* dynamically allocate an array of cache_size char* elements */
   cache = calloc( cache_size, sizeof( char * ) );

   if ( cache == NULL )
   {
      fprintf( stderr, "ERROR: calloc() failed.\n" );
      return EXIT_FAILURE;
   }
   
   /* open the text file */
   FILE *fp;                  /* file pointer*/
   fp = fopen(*(argv+2), "r");
   
   if ( fp == NULL )
   {
      fprintf( stderr, "ERROR: fopen() failed.\n");
      return EXIT_FAILURE;
   }
   
   char c;
   char* word = calloc(128, sizeof(char));
   int windex;
   int cindex;
   while (1)
   {
      windex = strlen(word);
      c = fgetc ( fp );
      //printf("%d\n", isalnum(c)); 
      
      /* if at end of file, break out of while loop*/
      if (c == EOF) 
      {
         free(word);
         break;
      }
      
      /*if read character is valid, append it to current word*/
      if (isalnum(c)) 
      {
         *(word+windex) = c;

      }
      /*if character is invalid then it is either not a valid word or a finished word*/
      /*-->if word is less than 3 characters long, it is considered not a valid word, so reset word to nothing*/
      else if (strlen(word) < 3)
      {
         memset(word, 0, 128);
      }
      /*-->if word is greater than 3 characters long, then we have a finished word, so put it into cache*/
      else 
      {
         
         cindex = hash(word, cache_size);
         if (*(cache+cindex) == NULL) 
         {
            *(cache+cindex) = calloc( strlen (word) + 1, sizeof( char ) );
            strcpy(*(cache+cindex), word);
            printf("Word \"%s\" ==> %d (calloc)\n", word, cindex);
         }
         else 
         {
            *(cache+cindex) = realloc(*(cache+cindex), strlen(word)+1);
            strcpy(*(cache+cindex), word);
            printf("Word \"%s\" ==> %d (realloc)\n", word, cindex);
         }
         //printf("%s\n", word);
         memset(word, 0, 128);
      }
   }
   
   fclose(fp);
   
   for (int i = 0; i < cache_size; i++) 
   {
      if (*(cache + i) != NULL) 
      {
         printf("Cache index %d ==> \"%s\"\n", i, *(cache+i));
      }
   } 
   
   /* Free all memory allocated for cache*/
   for (int i = 0; i < cache_size; i++) 
   {
      free(*(cache+i));
   } 
   free(cache);
   
   return EXIT_SUCCESS;
}

