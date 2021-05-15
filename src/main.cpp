#include <iostream>
#include <SDL.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct File {
    std::string name;
    TTF_Font *font;
    SDL_Texture *text;
    SDL_Rect location;
    bool isDirectory;
    bool isExec;
    std::string extension;
    std::string path;
    std::string permission;
    int filesize;
} File;

void textRefresh(SDL_Renderer *renderer, std::vector<File> *fileObjects, int offset);
void initialize(SDL_Renderer *renderer);
void render(SDL_Renderer *renderer);
void storeDirectory(std::string dirname, std::vector<File> *fileObjects,bool isRecursive,int level);

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    printf("HOME: %s\n", home);

    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize and perform rendering loop
    initialize(renderer);
    render(renderer);
    SDL_Event event;
    SDL_WaitEvent(&event);
    std::vector<File> fileObjects;
    std::string currentDir = home;
    storeDirectory(home,&fileObjects,false,0);

    //Andy, 16 lines of text can fit into the window (save 2 for up and down arrow)
    int offset = 0;
    textRefresh(renderer,&fileObjects, offset);
    bool expand = false;
    while (event.type != SDL_QUIT)
    {
        
        
        //render(renderer);

        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_MOUSEWHEEL:{
                if (event.wheel.y > 0){
                    if (offset > 0){
                        offset = offset-1;
                        textRefresh(renderer,&fileObjects, offset);
                    }
                }
                else if (event.wheel.y < 0){
                    if (offset < fileObjects.size()-16 && fileObjects.size() > 16){
                        offset = offset+1;
                        textRefresh(renderer,&fileObjects, offset);
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:{
                if (event.button.y >= 564 &&
                    event.button.y <= 580 &&
                    event.button.x >= 392 &&
                    event.button.x <= 408){
                    if (!expand){
                        fileObjects.clear();
                        storeDirectory(currentDir,&fileObjects,true,0);
                        textRefresh(renderer,&fileObjects, 0);
                        expand = true;
                        break;
                    }
                    else {
                        fileObjects.clear();
                        storeDirectory(currentDir,&fileObjects,false,0);
                        textRefresh(renderer,&fileObjects, 0);
                        expand = false;
                        break;
                    }
                    
                }
                //printf("mouse: %d, %d\n", event.motion.x,event.motion.y);
                for (int i=offset; i < fileObjects.size() && i < 16+offset; i++){
                    //std::cout<<fileObjects[i].location.y + fileObjects[i].location.h<<std::endl;
                    if (event.button.y >= fileObjects[i].location.y &&
                        event.button.y <= fileObjects[i].location.y + fileObjects[i].location.h)
                    {
                        if (fileObjects[i].isDirectory){
                            
                            currentDir = fileObjects[i].path;
                            fileObjects.clear();
                            storeDirectory(currentDir,&fileObjects,false,0);
                            //std::cout<<fileObjects[i].path<<std::endl;
                            offset = 0;
                            textRefresh(renderer,&fileObjects, offset);
                        }
                        else {
                            //std::cout<<fileObjects[i].extension.compare("png")<<std::endl;
                            int pid = fork();
                            if (pid==0){
                                
                                char* pathChar = const_cast<char*>(fileObjects[i].path.c_str());
                                execl("/usr/bin/xdg-open","xdg-open ",pathChar,(char *)0);
                                exit(1);
                            }
                        }
                    }

                }

                //fileObjects.clear();
                //home + "\""
                break;
            }
        }
    }

    // clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void textRefresh(SDL_Renderer *renderer, std::vector<File> *fileObjects, int offset){
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    SDL_Color color = { 0, 0, 0 };
    int prevY = 33;
    for (int i=offset; i < fileObjects->size() && i < 16+offset; i++){
        fileObjects->at(i).font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 24);
        char * fileNameChar = const_cast<char*>(fileObjects->at(i).name.c_str());
        SDL_Surface *text_surf = TTF_RenderText_Solid(fileObjects->at(i).font, fileNameChar, color);
        fileObjects->at(i).text = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);
        fileObjects->at(i).location.x = 40;
        fileObjects->at(i).location.y = prevY;
        SDL_QueryTexture(fileObjects->at(i).text, NULL, NULL, &(fileObjects->at(i).location.w),&(fileObjects->at(i).location.h));
        prevY = fileObjects->at(i).location.h + prevY;

        



        SDL_Surface *icon_Surface;
        if (fileObjects->at(i).isDirectory){
            icon_Surface = IMG_Load("resrc/images/directory.png");
        }
        else if (
                fileObjects->at(i).extension.compare("jpg")==0 ||
                fileObjects->at(i).extension.compare("jpeg")==0 ||
                fileObjects->at(i).extension.compare("png")==0 ||
                fileObjects->at(i).extension.compare("tif")==0 ||
                fileObjects->at(i).extension.compare("tiff")==0 ||
                fileObjects->at(i).extension.compare("gif")==0) {
            icon_Surface = IMG_Load("resrc/images/image.png");
        }
        else if (
                fileObjects->at(i).extension.compare("mp4")==0 ||
                fileObjects->at(i).extension.compare("mov")==0 ||
                fileObjects->at(i).extension.compare("mkv")==0 ||
                fileObjects->at(i).extension.compare("avi")==0 ||
                fileObjects->at(i).extension.compare("webm")==0) {
            icon_Surface = IMG_Load("resrc/images/video.png");
        }
        else if (
                fileObjects->at(i).extension.compare("h")==0 ||
                fileObjects->at(i).extension.compare("c")==0 ||
                fileObjects->at(i).extension.compare("cpp")==0 ||
                fileObjects->at(i).extension.compare("py")==0 ||
                fileObjects->at(i).extension.compare("java")==0 ||
                fileObjects->at(i).extension.compare("js")==0) {
            icon_Surface = IMG_Load("resrc/images/code.png");
        }
        else if (fileObjects->at(i).isExec){
            icon_Surface = IMG_Load("resrc/images/executable.png");
        }
        else {
            icon_Surface = IMG_Load("resrc/images/unknown.png");
        }
        SDL_Texture *icon_Texture;
        icon_Texture = SDL_CreateTextureFromSurface(renderer, icon_Surface);
        SDL_FreeSurface(icon_Surface);
        SDL_Rect icon_Location;
        icon_Location.x = 3;
        icon_Location.y = fileObjects->at(i).location.y;
        icon_Location.w = 32;
        icon_Location.h = 32;

        if (!fileObjects->at(i).isDirectory){
            int filesize = fileObjects->at(i).filesize;
            std::string filesizeString;
            if (filesize >= 1024){
                filesize = filesize / 1024;
                if (filesize >= 1024){
                    filesize = filesize / 1024;
                    if (filesize >= 1024){
                        filesize = filesize / 1024;
                        filesizeString = std::to_string(filesize) + " GB";
                    } else {
                        filesizeString = std::to_string(filesize) + " MB";
                    }
                } else {
                    filesizeString = std::to_string(filesize) + " KB";
                }
            } else {
                filesizeString = std::to_string(filesize) + " B";
            }
            char * fileSizeChar = const_cast<char*>(filesizeString.c_str());
            SDL_Surface *size_surf = TTF_RenderText_Solid(fileObjects->at(i).font, fileSizeChar, color);
            SDL_Texture *size_Texture;
            size_Texture = SDL_CreateTextureFromSurface(renderer, size_surf);
            SDL_FreeSurface(size_surf);
            SDL_Rect size_Location;
            size_Location.x = 720;
            size_Location.y = fileObjects->at(i).location.y;
            SDL_QueryTexture(size_Texture, NULL, NULL, &(size_Location.w),&(size_Location.h));
            SDL_RenderCopy(renderer, size_Texture, NULL, &size_Location);

            char * permChar = const_cast<char*>(fileObjects->at(i).permission.c_str());
            SDL_Surface *perm_surf = TTF_RenderText_Solid(fileObjects->at(i).font, permChar, color);
            SDL_Texture *perm_Texture;
            perm_Texture = SDL_CreateTextureFromSurface(renderer, perm_surf);
            SDL_FreeSurface(perm_surf);
            SDL_Rect perm_Location;
            perm_Location.x = 600;
            perm_Location.y = fileObjects->at(i).location.y;
            SDL_QueryTexture(perm_Texture, NULL, NULL, &(perm_Location.w),&(perm_Location.h));
            SDL_RenderCopy(renderer, perm_Texture, NULL, &perm_Location);


        }

        SDL_RenderCopy(renderer, fileObjects->at(i).text, NULL, &(fileObjects->at(i).location));
        SDL_RenderCopy(renderer, icon_Texture, NULL, &icon_Location);
        

        SDL_Surface *recur_Surface;
        recur_Surface = IMG_Load("resrc/images/recursive.png");
        SDL_Texture *recur_Texture;
        recur_Texture = SDL_CreateTextureFromSurface(renderer, recur_Surface);
        SDL_FreeSurface(recur_Surface);
        SDL_Rect recur_Location;
        recur_Location.x = 392;
        recur_Location.y = 568;
        recur_Location.w = 16;
        recur_Location.h = 16;

        SDL_RenderCopy(renderer, recur_Texture, NULL, &recur_Location);

        SDL_RenderPresent(renderer);

        //std::cout<<fileObjects[i].name<<std::endl;
    }
}

void initialize(SDL_Renderer *renderer)
{
    // set color of background when erasing frame
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
}

void render(SDL_Renderer *renderer)
{
    // erase renderer content
    SDL_RenderClear(renderer);
    
    // TODO: draw!

    // show rendered frame
    SDL_RenderPresent(renderer);
}
void storeDirectory(std::string dirname, std::vector<File> *fileObjects, bool isRecursive, int level)
{
    struct stat info;
    int err = stat(dirname.c_str(), &info);
    if (err == 0 && S_ISDIR(info.st_mode))
    {
        std::vector<std::string> files;
        DIR* dir = opendir(dirname.c_str());
        // TODO: modify to be able to print all entries in directory in alphabetical order
        //       in addition to file name, also print file size (or 'directory' if entry is a folder)
        //       Example output:
        //         ls.cpp (693 bytes
        //         my_file.txt (62 bytes)
        //         OS Files (directory)
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            //printf("%s\n", entry->d_name);
            files.push_back(entry->d_name);
        }
        closedir(dir);

        std::sort(files.begin(), files.end());

        int i;
        struct stat file_info;
        for (i=0; i < files.size(); i++)
        {
            err = stat((dirname+ "/" + files[i]).c_str(), &file_info);
            if (err)
            {
                //fprintf(stderr, "Uh oh, shouldn't have gotten here!\n");
            }
            else if (files[i].compare(".")==0 || (files[i].compare("..")==0 && level > 0)){
                //do nothing
            }
            else
            {
                File newFileObject;
                newFileObject.name = files[i];
                newFileObject.path = dirname+ "/" + files[i].c_str();
                //std::cout<<newFileObject.path<<std::endl;
                char* pathChar = const_cast<char*>(newFileObject.path.c_str());
                char permissionChar[] = "---------";

                if (S_ISDIR(file_info.st_mode))
                {
                    newFileObject.isDirectory = true;
                }
                else
                {
                    newFileObject.isDirectory = false;
                    int j = newFileObject.name.rfind('.',newFileObject.name.length());
                    if (j != std::string::npos){
                        newFileObject.extension = (newFileObject.name.substr(j+1,newFileObject.name.length()-j));
                    } 
                    else {
                        newFileObject.extension = "";
                    }
                    //printf("%s (%ld bytes)\n",files[i].c_str(), file_info.st_size);
                    newFileObject.filesize = file_info.st_size;


                    mode_t filePerm = file_info.st_mode;
                    if (filePerm & S_IRUSR){
                        newFileObject.permission = newFileObject.permission + "r";
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IWUSR){
                        newFileObject.permission = newFileObject.permission + "w";
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IXUSR){
                        newFileObject.permission = newFileObject.permission + "x";
                        newFileObject.isExec = true;
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IRGRP){
                        newFileObject.permission = newFileObject.permission + "r";
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IWGRP){
                        newFileObject.permission = newFileObject.permission + "w";
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IXGRP){
                        newFileObject.permission = newFileObject.permission + "x";
                        newFileObject.isExec = true;
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IROTH){
                        newFileObject.permission = newFileObject.permission + "r";

                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IWOTH){
                        newFileObject.permission = newFileObject.permission + "w";
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    if (filePerm & S_IXOTH){
                        newFileObject.permission = newFileObject.permission + "x";
                        newFileObject.isExec = true;
                    } else {
                        newFileObject.permission = newFileObject.permission + "-";
                    }
                    /*
                    struct stat fileattrib;
                    int fileMode;       

                    printf("-");
                    fileMode = fileattrib.st_mode;
                    if ((fileMode & S_IRUSR) && (fileMode & S_IREAD))
                        printf("r");
                    else
                        printf("-");
                    if ((fileMode & S_IWUSR) && (fileMode & S_IWRITE)) 
                        printf("w");
                    else
                        printf("-");
                    if ((fileMode & S_IXUSR) && (fileMode & S_IEXEC))
                        printf("x");
                    else
                        printf("-");
                    if ((fileMode & S_IRGRP) && (fileMode & S_IREAD))
                        printf("r");
                    else
                        printf("-");
                    if ((fileMode & S_IWGRP) && (fileMode & S_IWRITE))
                        printf("w");
                    else
                        printf("-");
                    if ((fileMode & S_IXGRP) && (fileMode & S_IEXEC))
                        printf("x");
                    else
                        printf("-");
                    if ((fileMode & S_IROTH) && (fileMode & S_IREAD))
                        printf("r");
                    else
                        printf("-");
                    if ((fileMode & S_IWOTH) && (fileMode & S_IWRITE))
                        printf("w");
                    else
                        printf("-");
                    if ((fileMode & S_IXOTH) && (fileMode & S_IEXEC))
                        printf("x   ");
                    */
                }

                for (int i=0; i < level; i++){
                    newFileObject.name = "    " + newFileObject.name;
                }

                fileObjects->push_back(newFileObject);
                
                if (S_ISDIR(file_info.st_mode)&& isRecursive)
                {
                    if (files[i] != "..")
                    {
                        storeDirectory(newFileObject.path,fileObjects,isRecursive,level+1);
                    }
                }
            }
            //printf("%s\n", files[i].c_str());
        }

    }
    else
    {
        fprintf(stderr, "Error: directory '%s' not found\n", dirname.c_str());
    }
}
