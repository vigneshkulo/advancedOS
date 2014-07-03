#include<stdio.h>
#include<string.h>

int main ()
{
   FILE *fp;
   char str[10]; // = "This is tutorialspoint.com";
	int rand = 10;

   fp = fopen( "file.txt" , "w+" );
  // fwrite(&rand , 1 , sizeof(int) , fp );
  // fwrite(&rand , 1 , sizeof(int) , fp );
sprintf(str, "%d ", rand);
   fwrite(str , 1 , strlen(str) , fp );

	rand = 20;
sprintf(str, "%d ", rand);
   fwrite(str , 1 , strlen(str) , fp );
   fclose(fp);
  
   return(0);
}
