#include <iostream>
#include <SDL.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <SDL_image.h>
#include <SDL_ttf.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct File {
    std::string name;
    TTF_Font *font;
    SDL_Texture *text;
    SDL_Rect location;
} File;

void textRefresh(SDL_Renderer *renderer, std::vector<File> *fileObjects, int offset);
void initialize(SDL_Renderer *renderer);
void render(SDL_Renderer *renderer);
void storeDirectory(std::string dirname, std::vector<File> *fileObjects);

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
    storeDirectory(home,&fileObjects);
    
    //Andy, 16 lines of text can fit into the window (save 2 for up and down arrow)
    int offset = 0;
    textRefresh(renderer,&fileObjects, offset);

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
                    if (offset < fileObjects.size()){
                        offset = offset+1;
                        textRefresh(renderer,&fileObjects, offset);
                        
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:{
                //printf("mouse: %d, %d\n", event.motion.x,event.motion.y);
                for (int i=offset; i < fileObjects.size() && i < 16+offset; i++){
                    //std::cout<<fileObjects[i].location.y + fileObjects[i].location.h<<std::endl;
                    if (event.button.y >= fileObjects[i].location.y &&
                        event.button.y <= fileObjects[i].location.y + fileObjects[i].location.h)
                    {
                        //std::cout<<fileObjects[i].name<<std::endl;
                        
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
        fileObjects->at(i).location.x = 20;
        fileObjects->at(i).location.y = prevY;
        SDL_QueryTexture(fileObjects->at(i).text, NULL, NULL, &(fileObjects->at(i).location.w),&(fileObjects->at(i).location.h));
        prevY = fileObjects->at(i).location.h + prevY;

        SDL_RenderCopy(renderer, fileObjects->at(i).text, NULL, &(fileObjects->at(i).location));
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
void storeDirectory(std::string dirname, std::vector<File> *fileObjects)
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
            else if (files[i].compare(".")==0){
                //do nothing
            }
            else
            {
                if (S_ISDIR(file_info.st_mode))
                {
                    //printf("%s (directory)\n",files[i].c_str());
                    /*
                    if (files[i] != "." && files[i] != "..")
                    {
                        listDirectory(dirname + "/" + files[i]);
                    }
                    */
                    //listDirector(dirname + "/" + files[i]);
                }
                else
                {
                    //printf("%s (%ld bytes)\n",files[i].c_str(), file_info.st_size);
                }
                File newFileObject;
                newFileObject.name = files[i];
                //newFileObject.font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 24);
                fileObjects->push_back(newFileObject);
                
            }
            //printf("%s\n", files[i].c_str());
        }

    }
    else
    {
        fprintf(stderr, "Error: directory '%s' not found\n", dirname.c_str());
    }
}