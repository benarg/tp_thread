#include <pthread.h>
#include "synchro.h"
#include "ensitheora.h"


bool fini;

/* les variables pour la synchro, ici */

int textureDispo = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c, c2, c3;
int tailleFenetre = 0;
int texturePrete = 0;

/* l'implantation des fonctions de synchro ici */
void envoiTailleFenetre(th_ycbcr_buffer b) {
  //pthread_mutex_init(&m2,NULL);
  pthread_mutex_lock(&m2);
  windowsx = b[0].width;
  windowsy = b[0].height;
  tailleFenetre = 1;
  pthread_cond_signal(&c2);
  pthread_mutex_unlock(&m2);
}

void attendreTailleFenetre() {
  pthread_mutex_lock(&m2);
  //pthread_cond_init(c,NULL)
  while (tailleFenetre == 0){
      pthread_cond_wait(&c2,&m2);
  }
  //pthread_cond_destroy(c);
  pthread_mutex_unlock(&m2);
  //pthread_mutex_destroy(&m2);
}

void signalerFenetreEtTexturePrete() {
  //pthread_mutex_init(&m3,NULL);
  pthread_mutex_lock(&m3);
  texturePrete = 1;
  pthread_cond_signal(&c3);
  pthread_mutex_unlock(&m3);
}

void attendreFenetreTexture() {
  pthread_mutex_lock(&m3);
  while (texturePrete == 0){
    printf("pas de texture prete\n");
    pthread_cond_wait(&c3,&m3);
  }
  pthread_mutex_unlock(&m3);
  //pthread_mutex_destroy(&m3);
}

void debutConsommerTexture() {
  pthread_mutex_lock(&m);
  while (textureDispo <= 0){
    pthread_cond_wait(&c,&m);
  }
}

void finConsommerTexture() {
  textureDispo -=1;
  printf("texture dispo %i\n",textureDispo );
  pthread_mutex_unlock(&m);
    pthread_cond_signal(&c);
}


void debutDeposerTexture() {

  pthread_mutex_lock(&m);
  while (textureDispo >= NBTEX){
    pthread_cond_wait(&c,&m);
  }
  printf("texture dispo %i\n",textureDispo );

}

void finDeposerTexture() {
  textureDispo +=1;
  printf("texture dispo %i\n",textureDispo );
  pthread_mutex_unlock(&m);
  pthread_cond_signal(&c);
}
