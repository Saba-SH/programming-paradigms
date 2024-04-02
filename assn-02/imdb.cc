using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
    const string actorFileName = directory + "/" + kActorFileName;
    const string movieFileName = directory + "/" + kMovieFileName;

    actorFile = acquireFileMap(actorFileName, actorInfo);
    movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
    return !( (actorInfo.fd == -1) || 
        (movieInfo.fd == -1) ); 
}

/**compares two names
 * @param one: pointer to actorPair struct
 * @param two: pointer to the offset of the second actor's name in the actor file
 * @return the result of string comparison
*/
int imdb::namesCmp(const void* one, const void* two)
{
    actorPair *pr = (actorPair*)one;
    const char* name0 = pr->name;
    int offset = *(int*)two;
    char* name1 = (char*)pr->filePtr + offset;

    return strcmp(name0, name1);
}

/**compares two films
 * @param one: pointer to moviePair struct
 * @param two: pointer to the offset of the second film info in the film file
 * @return the result of comparison of films based on their name and year
*/
int imdb::filmsCmp(const void* one, const void* two)
{
    filmPair *fp = (filmPair*)one;
    int offset = *(int*)two;
    char *secondFilmInfo = (char*)fp->filePtr + offset;
    
    film film1;
    film1.title = string(secondFilmInfo);
    film1.year = 1900 + (int)*((char*)secondFilmInfo + film1.title.length() + 1);

    if(*fp->movie == film1)
        return 0;
    if(*fp->movie < film1)
        return -1;
    return 1;
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
    int actorAmount = *(int*)actorFile;

    void* startOfOffsets = (int*)actorFile + 1;
    //create an actorPair struct for comparison
    const char *cstr = player.c_str();
    actorPair searchPair;
    searchPair.name = cstr;
    searchPair.filePtr = actorFile;

    void *pointerToOffset = bsearch(&searchPair, startOfOffsets, actorAmount, sizeof(int), namesCmp);
    //if the actor couldn't be found
    if(pointerToOffset == NULL)
        return false;
    
    const void *startOfInfo = (char*)actorFile + *(int*)pointerToOffset;
    //advance through the actor info
    char *positionInActorFile = (char*)startOfInfo;
    positionInActorFile += player.length() + 1;
    if(player.length() % 2 == 0)               //skip the extra \0 after the name if needed
        positionInActorFile++;

    int filmAmount = (int)*(short*)positionInActorFile;
    positionInActorFile += 2;
    if((positionInActorFile - (char*)startOfInfo) % 4)      //skip two \0's after the amount of films if needed
        positionInActorFile += 2;

    //iterate over the films of the actor, inserting them in the vector
    for(int i = 0; i < filmAmount; i++)
    {
        int offsetInMovieFile = *(int*)positionInActorFile;
        film currFilm;
        char* startOfFilmInfo = (char*)movieFile + offsetInMovieFile;
        currFilm.title = string(startOfFilmInfo);
        currFilm.year = 1900 + *(startOfFilmInfo + currFilm.title.length() + 1);
        films.push_back(currFilm);
        positionInActorFile += 4;
    }

    return true;
}

bool imdb::getCast(const film& movie, vector<string>& players) const {
    int filmAmount = *(int*)movieFile;
    void *startOfOffsets = (int*)movieFile + 1;
    
    //create a filmPair struct for searching
    filmPair searchPair;
    searchPair.movie = &movie;
    searchPair.filePtr = movieFile;

    void *pointerToOffset = bsearch(&searchPair, startOfOffsets, filmAmount, sizeof(int), filmsCmp);
    //if the film can't be found
    if(pointerToOffset == NULL)
        return false;

    const char *startOfInfo = (char*)movieFile + *(int*)pointerToOffset;
    char *positionInInfo = (char*)startOfInfo + movie.title.length() + 2;   //2 bytes _ \0 at the end and the year
    if(movie.title.length() % 2)        //extra \0 maybe
        positionInInfo++;

    int actorsAmount = (int)*(short*)positionInInfo;
    positionInInfo += 2;
    if((positionInInfo - startOfInfo) % 4)
        positionInInfo += 2;

    //iterate over the actors and insert them in the vector
    for(int i = 0; i < actorsAmount; i++)
    {
        int currActorOffset = *(int*)positionInInfo;
        players.push_back(string((char*)actorFile + currActorOffset));
        positionInInfo += 4;
    }

    return true;
}

imdb::~imdb()
{
    releaseFileMap(actorInfo);
    releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
    struct stat stats;
    stat(fileName.c_str(), &stats);
    info.fileSize = stats.st_size;
    info.fd = open(fileName.c_str(), O_RDONLY);
    return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
    if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
    if (info.fd != -1) close(info.fd);
}
