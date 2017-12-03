//TODOKISHAN delete this
/*C program to read and print the N student 
details using structure and Dynamic Memory Allocatoin.*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
/*structure declaratoin*/
struct student
{
    char name[30];
    int roll;
    int perc;
};
 
int main(int argc, char* argv[], char* envp[])
{
    struct student *pstd;
    int n,i;
    char* buf = (void*)malloc(256);
    printf("Enter total number of elements: ");
    gets(buf);
    atoi(buf,&n);
     
    /*Allocate memory dynamically for n objetcs*/
    pstd=(struct student*)malloc(n*sizeof(struct student));
     
    if(pstd==NULL)
    {
        printf("Insufficient Memory, Exiting... \n");
        return 0;
    }
     
    /*read and print details*/
    for(i=0; i<n; i++)
    {
        printf("\nEnter detail of student [%d]:\n",i+1);
        printf("Enter name: ");
        //scanf(" "); /*clear input buffer*/
        gets((pstd+i)->name);
        printf("Enter roll number: ");
        gets(buf);
        atoi(buf, &(pstd+i)->roll);
        //scanf("%d",&(pstd+i)->roll);
        printf("Enter percentage: ");
        gets(buf);
        atoi(buf, &(pstd+i)->perc);
        //scanf("%f",&(pstd+i)->perc);
    }
     
    printf("\nEntered details are:\n");
    for(i=0; i<n; i++)
    {
        printf("%s   %d %d\n",(pstd+i)->name,(pstd+i)->roll,(pstd+i)->perc);
    }
     
    free(pstd); 
    free(buf); 
    return 0;
}
