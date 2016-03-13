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
const char* etalon = "Logic";
const char* etalon2 = "MIPT";
const char* slash = "/";
const char* null_char = "\0";
const char* dot = ".";
const char* two_dots = "..";

void read_dir_test(char* dirname) {
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(dirname);
    
    char* work_name = dirname;
    
    if(!dir) {
        tabs--;
        perror("diropen");
        return;
    }
    
    while ( (entry = readdir(dir)) != NULL) {
        if (entry -> d_type == dir_flag) {
            
            for (int i = 0; i < tabs; i++) {
                printf("-");
            }
            
            printf("%s\n", entry -> d_name);
            if ( (entry -> d_type == 4) && (strcmp(entry -> d_type, dot))) {
                tabs++;
                read_dir_test( strcat( strcat(work_name, "/"), entry -> d_name) );
            }
            
        }
    };
    
    tabs--;
    int t = 0;
    while ( strchr( (work_name + strlen(work_name) - t), '/') == 0 ){
        t++;
    }
    *(work_name + strlen(work_name) - t) = '\0';
    closedir(dir);
    
}

//Вывод требуемой директории.
void read_dir(char* dirname) {
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(dirname);
    
    if(!dir) {
        perror("diropen");
        return;
    }

    while ( (entry = readdir(dir)) != NULL) {
        if (entry -> d_type == dir_flag)
        printf("%s\n", entry -> d_name);
    };
    
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
    
    read_dir_test(name);
}
