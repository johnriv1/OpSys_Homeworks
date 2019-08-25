#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <linux/limits.h> /* PATH_MAX */
#include <unistd.h> /* getcwd */ /*lstat*/
#include <sys/types.h> /*lstat*/
#include <sys/stat.h> /*lstat*/
#include <sys/types.h> /*waitpid*/
#include <sys/wait.h> /*waitpid*/

void remove_element(int* array, int index, int *array_size)
{
	for (int i = index; i < ((*array_size) - 1); i++)
   { 
   	array[i] = array[i + 1];
   }
  	(*array_size) --;
}

void print_array(int* array,int array_size) {
	for (int i = 0; i < array_size; i++) {
		printf("Array value is %d\n", array[i]);
	}
}

int main (){
#if 0
	char** array = NULL;
	array = calloc(3, sizeof(char*));
	
	array[0] = calloc(2, sizeof(char));
	array[1] = calloc(2, sizeof(char));
	array[2] = calloc(2, sizeof(char));
	
	strcpy(array[0], "a");
	strcpy(array[1], "b");
	strcpy(array[2], "c");
	
	printf("BEFORE NULL");
	for (int i = 0; i < 3; i++) {
		printf("Value is %s\n", array[i]);
	}
	
	free(array[2]);
	array[2] = NULL;

	/*array is {a,b,NULL} */
	
	for (int i = 0; i < 3; i++) {
		free(array[i]);
	}
	free(array);
#endif

	int array[3];
	array[0] = 10;
	array[1] = 11;
	array[2] = 12;
	int array_size = 3;
	remove_element(array, 1, &array_size);
	printf("Array size is %d\n", array_size);
	print_array(array, array_size);
	
	
	
}
