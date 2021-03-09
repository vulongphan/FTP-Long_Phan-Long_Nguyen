#include <stdio.h>
#include <dirent.h>
int main(void){
    // Pointer for directory entry 
   struct dirent *files;

   // opendir() returns a pointer of DIR type
   DIR *dir = opendir(".");

   // opendir returns NULL if couldn't open directory 
   if (dir == NULL){
      printf("Directory cannot be opened!" );
      return 0;
   }
   while ((files = readdir(dir)) != NULL)
   printf("%s\n", files->d_name);
   closedir(dir);
   return 0;
}