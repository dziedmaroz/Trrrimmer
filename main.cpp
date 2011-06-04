#include "main.h"

#include <stdio.h>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <fstream>

using namespace std;

    void trim (FILE* fin, FILE* fout);
    void shiftLeft (char* str);
    char* safefgets (FILE* fin);

    void findNReplace (char* finName, char* foutName, char* fVocName);
    char* decToHex (int x);
    bool isInSet (char, char*);
    char** readTypes (char* filename, int* size);
    void addVoc (char** &voc,char *entry, int &vocSize);
    void cleanVoc (char** voc, int &vocSize);

int main(int argc, char *argv[])
{
   FILE* fin = fopen (argv[1],"r");
   FILE* fout = fopen ("tmp.trm","w");
   trim (fin,fout);
   fclose (fin);
   fclose (fout);
   char* tmp = decToHex (255);
   printf ("TEST     \"   TEST SPACE TEST   %s\n",tmp);
   delete [] tmp;
   findNReplace ("tmp.trm",argv[2],"voc.voc");
   return 0;
}


void shiftLeft (char* str)
{
    int i=0;
    while (str[i]!='\0')
    {
        str[i]=str[++i];
    }
}

char* safefgets (FILE *fin)
{
    const int BLOCK_SIZE = 8;
    char* tmpStr = (char*) malloc (BLOCK_SIZE*sizeof(char));
    int len = 0;
    tmpStr[len]=fgetc(fin);
    len++;
    int allocated = BLOCK_SIZE;
    while (tmpStr[len-1]!='\n' && tmpStr[len-1]!='\0' && tmpStr[len-1]!=-1)
    {
        if (len==allocated)
        {
            allocated+=BLOCK_SIZE;
            tmpStr = (char*) realloc (tmpStr,allocated*sizeof(char));
        }
        fscanf (fin,"%c",&tmpStr[len++]);
    }
    tmpStr[len-1]='\0';
    return tmpStr;
}

void trim (FILE *fin, FILE *fout)
{
    bool comment = false;
    bool nothingToChange = false;
    while (!feof (fin))
    {
        bool directive = false;
        char* tmpStr = safefgets (fin);
        int sLength = strlen(tmpStr);
        if (tmpStr[0]=='#') directive = true;
        for (int i=0;i<sLength;i++)
        {
            if (tmpStr[i]=='\"')
            {
                if (i>0)
                {
                    if (tmpStr[i-1]!='\\') nothingToChange = !nothingToChange;
                }
                else nothingToChange = !nothingToChange;
                continue;
            }
            if (!nothingToChange)
            {
                if (tmpStr[i]=='/' && tmpStr[i+1]=='/')
                {
                    tmpStr[i]='\0';
                    sLength=i;
                    break;
                }
                if (tmpStr[i]==' ' && tmpStr[i+1]==' ')
                {
                    shiftLeft(tmpStr+i);
                    i--;
                    sLength--;
                }
                if (tmpStr[i]=='\t')
                {
                    shiftLeft (tmpStr+i);
                    i--;
                    sLength--;
                }
                if (tmpStr[i]==' ' && (i!=0) && (tmpStr[i-1]=='{' || tmpStr[i-1]=='}'|| tmpStr[i-1]==';'))
                {
                    shiftLeft (tmpStr+i);
                    sLength--;
                }
            }
        }
        for (int i=0;i<sLength;i++)
        {
            if (tmpStr[i]=='/' && tmpStr[i+1]=='*') comment = true;
            if (tmpStr[i]=='*' && tmpStr[i+1]=='/') comment = false;
            if (!comment)
            {
                fprintf (fout,"%c",tmpStr[i]);
            }
        }
        if (directive)
        {
            fprintf (fout,"\n");
            directive = false;
        }
        free(tmpStr);
    }
    fprintf (fout,"\n");
}

char* decToHex (int x)
{
    const int SIZE = 10;
    char* tmpStr = new char [SIZE];
    for (int i=0;i<SIZE;i++)
        tmpStr[i]='0';
    tmpStr[0]='O';
    tmpStr[1]='x';

    int count =0;

    while (x!=0)
    {
        if (x%16<10)
        {
            tmpStr[SIZE-count-1]='0'+x%16;
        }
        else
        {
            tmpStr[SIZE-count-1]='a'+x%16-10;
        }
        x/=16;
        count++;
    }
    return tmpStr;
}

void findNReplace (char *finName, char *foutName, char *fVocName)
{
     FILE* fin = fopen (finName,"r");
     fseek(fin,0,SEEK_END);
     int size =ftell (fin);
     rewind (fin);
     char* buff = new char [size];
     bool* pattern = new bool [size];

// избавляемся от всего лишнего
     for (int i=0;i<size;i++)
     {
         buff[i]=fgetc(fin);
         pattern[i]=true;
     }
     fclose (fin);
     for (int i=0;i<size;i++)
     {
         if (buff[i]=='#' && (i==0 || buff[i-1]=='\n'))
         {

             while (buff[i]!='\n'&& i<size)
             {
                 pattern[i]=false;
                 i++;

             }
             pattern[i]=false;
             continue;
         }
         if (isInSet (buff[i],"!#%&*-+/|^{}"))  { pattern[i]=false; continue;}
     }

     bool strFlag = false;
     int patternSize = 0;
     for (int i=0;i<size;i++)
     {
         if (buff[i]=='\"')
         {
             if (i>0)
             {
                 if (buff[i-1]!='\\')
                 {
                     strFlag = !strFlag;
                     pattern [i]=false;
                 }
             }
             else
             {
                 strFlag = !strFlag;
                 pattern [i]=false;
             }
         }
         if (strFlag) pattern [i]=false;
         if (pattern[i]) patternSize++;
     }


     FILE* fvoc = fopen (fVocName,"w") ;
     char* pattBuff = new char [patternSize];
     patternSize=0;
     for (int i=0;i<size;i++)
     {
         if (pattern[i])
         {              
             pattBuff[patternSize++]=buff[i];
         }
     }
//************************************************

     int vocTop =0;
     char** vocabulary=NULL;

     int typesSize =0;
     char** types = readTypes ("types.tp",&typesSize);

// поиск переменных
     for (int i=0;i<typesSize;i++)
     {
         char* pattBuffPTR = pattBuff;
         while (pattBuffPTR)
         {
             pattBuffPTR = strstr(pattBuffPTR,types[i]);
             if (!pattBuffPTR) break;
             if (pattBuffPTR[strlen(types[i])]!=' ')
             {
                 pattBuffPTR+=strlen(types[i]);
                 continue;
             }

             pattBuffPTR+=strlen(types[i]);
             while (pattBuffPTR && isInSet (*(pattBuffPTR)," ") )
             {
                 pattBuffPTR++;
             }

             char* candidate = new char [128];
             int len =0;
             while (!isInSet(*(pattBuffPTR)," =[](),.:"))
             {
                 candidate[len++]=*(pattBuffPTR++);
             }
             candidate[len]='\0';
             if (strcmp(candidate,"main") && candidate[0]!=NULL)
             {
                 addVoc (vocabulary,candidate,vocTop);
             }
             delete [] candidate;
         }
     }

     cleanVoc (vocabulary,vocTop);
     for (int i=0;i<vocTop;i++)
     {
          fprintf (fin,"%s\n",vocabulary[i]);
     }
     delete [] pattBuff;
     for (int i=0;i<typesSize;i++)
     {
         delete [] types[i];
     }
     delete [] types;
//************************************************

// расставляем хуки
     int* hooks = new int [size];
     for (int i=0;i<size;i++)
     {
         hooks[i]=-1;
     }

     for (int i=0;i<vocTop;i++)
     {
         char* buffPTR = strstr(buff,vocabulary[i]);
         while (buffPTR && buffPTR-buff <= size)
         {
             if (pattern[buffPTR-buff])
             {
                 if (!isInSet (*(buffPTR-1),"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"))
                 {
                    if (!isInSet(*(buffPTR+strlen(vocabulary[i])),"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"))
                    {
                           hooks[buffPTR-buff]=i;
                    }
                 }
             }
             buffPTR+=strlen(vocabulary[i]);
             buffPTR = strstr (buffPTR,vocabulary[i]);

         }

     }
//************************************************

    FILE* fout = fopen (foutName,"w");
    for (int i=0;i<size;i++)
    {
        if (hooks[i]!=-1)
        {
           char* replace = decToHex (hooks[i]);
           fprintf (fout,"%s",replace) ;
           delete [] replace;
           i+=strlen(vocabulary[hooks[i]]);
        }
        fprintf (fout,"%c",buff[i]);
    }

     delete [] buff;
     for (int i=0;i<vocTop;i++)
     {
         delete [] vocabulary[i];
     }
     delete [] vocabulary;
     fclose (fvoc);
     delete [] pattern;
     delete [] hooks;
}

bool isInSet (char ch, char * set)
{
    for (int i=0;i<strlen(set);i++)
    {
        if (ch==set[i]) return true;
    }

    return false;
}

char** readTypes (char *filename, int *size)
{
    FILE* fin= fopen (filename,"r");
    int lines =0;
    while(!feof(fin))
    {
        char* tmpStr = new char [100];
        tmpStr[0]='\0';
        fgets(tmpStr,100,fin);
        if (*tmpStr!=NULL) lines++;
        delete[] tmpStr;
    }
    rewind (fin);
    char** types = new char* [lines];
    int typesTop=0;
    while (!feof(fin))
    {
        char* tmpStr= safefgets (fin);
        if (*tmpStr!=NULL)
        {
            types[typesTop]=new char [strlen(tmpStr)+1];
            for (int i=0;i<strlen(tmpStr);i++)
                types[typesTop][i]=tmpStr[i];
            types[typesTop][strlen(tmpStr)]='\0';
            typesTop++;
        }
        free (tmpStr);
    }
    fclose (fin);
    *size = typesTop;
    return types;
}


void addVoc (char** &voc,char *entry, int &vocSize)
{
    char** tmpVoc = new char* [vocSize+1];
    for (int i=0;i<vocSize;i++)
        tmpVoc[i]=voc[i];
    delete [] voc;
    tmpVoc[vocSize]=new char [strlen(entry)+1];
    for (int i=0;i<strlen(entry);i++)
    {
        tmpVoc [vocSize][i]=entry[i];
    }
    tmpVoc [vocSize][strlen(entry)]='\0';
    vocSize++;
    voc = tmpVoc;
}

void cleanVoc (char **voc, int &vocSize)
{
    for (int i=0;i<vocSize;i++)
    {
        for (int j=i+1;j<vocSize;j++)
        {
            if (strcmp(voc[i],voc[j])<0)
            {
                char* tmpPtr= voc[i];
                voc[i]=voc[j];
                voc[j]=tmpPtr;
            }
            if (!strcmp (voc[i],voc[j]))
            {
                delete [] voc [j];
                for (int ii=j;ii<vocSize-1;ii++)
                {
                    voc[ii]=voc[ii+1];
                }
                vocSize--;
            }
        }
    }
}
