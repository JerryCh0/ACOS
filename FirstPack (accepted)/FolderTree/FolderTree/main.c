#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


int tabs;
const char* dot = ".";
const char* two_dots = "..";
const char* slash = "/";

//Вывод директории.
void read_dir(char* dirname, int max_dirname_len) {
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(dirname);
    
    if(!dir) {
        tabs--;
        perror("diropen");
        return;
    }
    
    while ( (entry = readdir(dir)) != NULL) {
        
        if (strcmp(entry -> d_name, dot) == 0 || strcmp(entry -> d_name, two_dots) == 0) {
            continue;
        }
        
        if (entry -> d_type == DT_DIR) {
            
            for (int i = 0; i < tabs; i++) {
                printf(" ");
            }
            
            if (strcmp(entry -> d_name, dot) != 0 && strcmp(entry -> d_name, two_dots) != 0) {
                printf("%s\n", entry -> d_name);
            }
            
            if (strcmp(dirname, slash) == 0) {
                tabs++;
                if(strlen(dirname) + strlen(entry->d_name) >= max_dirname_len) {//если path стал слишком длинным, реаллокируем память
                    char* new_dirname;
                    new_dirname = (char*) malloc(2 * sizeof(char) * max_dirname_len);
                    strcpy(new_dirname, dirname);
                    strcat(new_dirname, entry -> d_name);
                    read_dir( new_dirname, 2 * max_dirname_len );
                }else {
                    strcat(dirname, entry -> d_name);
                    read_dir( dirname, max_dirname_len );
                }
            }
            
            else{
                tabs++;
                if(strlen(dirname) + 1 + strlen(entry->d_name) >= max_dirname_len) {//если path стал слишком длинным, реаллокируем память
                    char* new_dirname;
                    new_dirname = (char*) malloc(2 * sizeof(char) * max_dirname_len);
                    strcpy(new_dirname, dirname);
                    strcat(new_dirname, "/");
                    strcat(new_dirname, entry->d_name);
                    read_dir( new_dirname, 2 * max_dirname_len );
                }else {
                    strcat(dirname, "/");
                    strcat(dirname, entry->d_name);
                    read_dir( dirname, max_dirname_len );
                }
            }
            
        }
    };
    
    if (strcmp(dirname, slash) != 0) {
        tabs--;
        int t = 0;
        while ( strchr( (dirname + strlen(dirname) - t), '/') == 0){
            t++;
        }
        if (t == strlen(dirname) - 1) {
            *(dirname + strlen(dirname) - t + 1) = '\0';
        }
        else {
            *(dirname + strlen(dirname) - t) = '\0';
        }
    }
    
    closedir(dir);
    
}

int main() {
    
    //Считываем имя требуемой директории.
    char *name = (char*) malloc(sizeof(char) * 100);
    scanf("%s", name);
    
    //Обнуляем счетчик отступов.
    tabs = 0;
    
    read_dir(name, 100);
    
}
