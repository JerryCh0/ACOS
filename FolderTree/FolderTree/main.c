//
//  main.c
//  FolderTree
//
//  Created by Дмитрий Ткаченко on 09/03/16.
//  Copyright © 2016 Dmitry Tkachenko. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


int dir_flag, tabs;
const char* dot = ".";
const char* two_dots = "..";
const char* slash = "/";

//Вывод директории.
void read_dir(char* dirname) {
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(dirname);
    
    if(!dir) {
        tabs--;
        //perror("diropen");
        return;
    }
    
    while ( (entry = readdir(dir)) != NULL) {
        if (strcmp(entry -> d_name, dot) == 0 || strcmp(entry -> d_name, two_dots) == 0) {
            continue;
        }

        if (entry -> d_type == dir_flag) {
            
            for (int i = 0; i < tabs; i++) {
                printf(" ");
            }
            
            if (strcmp(entry -> d_name, dot) != 0 && strcmp(entry -> d_name, two_dots) != 0) {
                printf("%s\n", entry -> d_name);
            }
            
            if ((strcmp(entry -> d_name, dot) != 0) && (strcmp(entry -> d_name, two_dots) != 0) ) {
                if (strcmp(dirname, slash) == 0) {
                    tabs++;
                    read_dir( strcat( dirname, entry -> d_name) );
                }
                else{
                    tabs++;
                    read_dir( strcat( strcat( dirname, "/"), entry -> d_name) );
                }
            }
            
        }
    };
    
    tabs--;
    int t = 0;
    while ( strchr( (dirname + strlen(dirname) - t), '/') == 0 ){
        t++;
    }
    *(dirname + strlen(dirname) - t) = '\0';
    closedir(dir);
    
}

int main() {
    
    //Определяем номер типа директории. На любой системе первая директория - "." и это папка.
    DIR *dirr;
    struct dirent *en;
    dirr = opendir("/");
    en = readdir(dirr);
    dir_flag = en -> d_type;
    
    //Считываем имя требуемой директории.
    char name[256];
    scanf("%s", name);
    
    //Обнуляем счетчик отступов.
    tabs = 0;
    
    read_dir(name);
}
