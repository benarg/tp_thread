#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <pthread.h>

#include "stream_common.h"
#include "oggstream.h"

struct threadParam1{
  char* nomFichier;
  pthread_mutex_t m;
};

int main(int argc, char *argv[]) {
    int res;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s FILE", argv[0]);
	exit(EXIT_FAILURE);
    }
    assert(argc == 2);


    // Initialisation de la SDL
    res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    atexit(SDL_Quit);
    assert(res == 0);

    // start the two stream readers
    pthread_t tid[2];
    struct threadParam1 param;

    param.nomFichier = argv[1];
    pthread_mutex_init(&(param.m),NULL);
    pthread_create(&tid[0],NULL,theoraStreamReader,&param);
    pthread_create(&tid[1],NULL,vorbisStreamReader,&param);



    // wait audio thread

    //pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);


    // 1 seconde de garde pour le son,
    sleep(1);

    // tuer les deux threads videos si ils sont bloqués
    pthread_cancel(tid[0]);
    //pthread_cancel(tid[1]);

    pthread_mutex_destroy(&(param.m));
    // attendre les 2 threads videos


    exit(EXIT_SUCCESS);
}
