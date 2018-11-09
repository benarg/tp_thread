#include <stdbool.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include "ensitheora.h"
#include "synchro.h"
#include "stream_common.h"

int windowsx = 0;
int windowsy = 0;

int tex_iaff= 0;
int tex_iwri= 0;


static SDL_Window *screen = NULL;
static SDL_Renderer *renderer = NULL;
struct TextureDate texturedate[NBTEX] = {};

struct streamstate *theorastrstate=NULL;

struct threadParam2{
  long long int serial;
  pthread_mutex_t m;
};

void *draw2SDL(void *arg) {
//    struct threadParam2 * a = reinterpret_cast< struct threadParam2 *>(p);
    struct threadParam2 *a = arg;
    int serial = (int) (*a).serial;
    struct streamstate *s= NULL;

    printf("[draw]attendreTailleFenetre - 1\n");
    attendreTailleFenetre();
    printf("[draw]attendreTailleFenetre - 2\n");

    // create SDL window (if not done) and renderer
    screen = SDL_CreateWindow("Ensimag lecteur ogg/theora/vorbis",
			      SDL_WINDOWPOS_UNDEFINED,
			      SDL_WINDOWPOS_UNDEFINED,
			      windowsx,
			      windowsy,
			      0);
    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);


    assert(screen);
    assert(renderer);
    // affichage en noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);


    // la texture
    for(int i=0; i < NBTEX; i++) {
	texturedate[i].texture = SDL_CreateTexture(renderer,
						   SDL_PIXELFORMAT_YV12,
						   SDL_TEXTUREACCESS_STREAMING,
						   windowsx,
						   windowsy);
	texturedate[i].timems = 0.0;
	assert(texturedate[i].texture);
    }

    printf("[draw]signalerFenetreEtTexturePrete - 1\n");
    signalerFenetreEtTexturePrete();
    printf("[draw]signalerFenetreEtTexturePrete - 2\n");

    /* Protéger l'accès à la hashmap */

    printf("[draw]Devant mutex\n");
    pthread_mutex_lock(&((*a).m));
    printf("[draw]Entrée mutex\n");
    HASH_FIND_INT( theorastrstate, &serial, s );
    pthread_mutex_unlock(&((*a).m));
    printf("[draw]Sortie mutex\n");


    assert(s->strtype == TYPE_THEORA);

    while(! fini) {
	// récupérer les évenements de fin
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	  // handle your event here
	  if (event.type == SDL_QUIT) {
	    fini = true;
	    break;
	  }
	}
  printf("[draw]debutConsommerTexture - 1\n");
	debutConsommerTexture();
  printf("[draw]debutConsommerTexture - 1\n");

	// Copy the texture with the renderer
	SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texturedate[tex_iaff].texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	double timemsfromstart = msFromStart();

	int delaims = (int) (texturedate[tex_iaff].timems - timemsfromstart);

	tex_iaff = (tex_iaff + 1) % NBTEX;

  printf("[draw]finConsommerTexture - 1\n");
	finConsommerTexture();
  printf("[draw]finConsommerTexture - 2\n");

	if (delaims > 0.0)
	    SDL_Delay(delaims);
    }
    return 0;
}


void theora2SDL(struct streamstate *s) {
    assert(s->strtype == TYPE_THEORA);

    ogg_int64_t granulpos = -1;
    double framedate; // framedate in seconds
    int res = th_decode_packetin( s->th_dec.ctx,
				  & s->packet,
				  & granulpos);
    framedate = th_granule_time( s->th_dec.ctx, granulpos);
    if (res == TH_DUPFRAME) // 0 byte duplicated frame
	return;

    assert(res == 0);
    th_ycbcr_buffer buffer;

    res =  th_decode_ycbcr_out(s->th_dec.ctx, buffer);

    // Envoyer la taille de la fenêtre

    printf("[theora]envoiTailleFenetre - 1\n");
    envoiTailleFenetre(buffer);
    printf("[theora]envoiTailleFenetre - 2\n");

    printf("[theora]attendreFenetreTexture - 1\n");
    attendreFenetreTexture();
    printf("[theora]attendreFenetreTexture - 2\n");

    // copy the buffer
    SDL_Rect rect = {};
    rect.w = buffer[0].width;
    rect.h = buffer[0].height;

    // 1 seul producteur/un seul conso => synchro sur le nb seulement

    printf("[theora]debutDeposerTexture - 1\n");
    debutDeposerTexture();
    printf("[theora]debutDeposerTexture - 2\n");

    res = SDL_UpdateYUVTexture(texturedate[tex_iwri].texture, & rect,
			       buffer[0].data,
			       buffer[0].stride,
			       buffer[1].data,
			       buffer[1].stride,
			       buffer[2].data,
			       buffer[2].stride);
    texturedate[tex_iwri].timems = framedate * 1000;
    printf("res : %i\n", res);
    if(res == -1){
      printf("SDL_Init failed: %s\n", SDL_GetError());
    }
    assert(res == 0);
    tex_iwri = (tex_iwri + 1) % NBTEX;

    printf("[theora]finDeposerTexture - 1\n");
    finDeposerTexture();
    printf("[theora]finDeposerTexture - 2\n");
}
