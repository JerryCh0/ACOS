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

int tabs;

void read_dir(char* dirname) {
    
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirname);
    
    if(!dir) {
        tabs--;
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry -> d_type == 4) {
            for (int i = 0; i < tabs; i++) {
                printf("-");
            }
            
            printf("%s\n", entry -> d_name);
            tabs++;
            read_dir( strcat( strcat(dirname, "/"), entry -> d_name) );
        }
    };
        
    tabs--;
    closedir(dir);
    
}

int main() {
    
    char name[256];
    scanf("%s", name);
    tabs = 0;
    
    read_dir(name);
}
