/**************************************
****   CDAUDIO.C  -  CD-DA Player  ****
**************************************/

#ifdef ENABLE_CDDA
//-- Include files -----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <SDL.h>

#include "../neo4all.h"
#include "cdaudio.h"

#include "../video/console.h"

#ifdef USE_THREAD_CDDA
#include<SDL_thread.h>
#define MAX_COLA 128
static int cola[MAX_COLA];
static int iw_cola=0, ir_cola=0;
static SDL_sem *sem_atiende=NULL;
static SDL_mutex *mtx_procesando=NULL;
#endif

#ifdef __QNXNTO__
#include <stdio.h>
#include <unistd.h>
#include <mm/renderer.h>
#include <sys/stat.h>

const char* PLAYER_NAME = "cdda_player";
const char* AUDIO_OUT = "audio:default";
const char* INPUT_TYPE = "track";

static mmr_connection_t* connection;
static mmr_context_t *ctxt;
#endif


//-- Private Variables -------------------------------------------------------
static int			cdda_min_track;
static int			cdda_max_track;
static int			cdda_disk_length;
static int			cdda_track_end;
static int			cdda_loop_counter;
static SDL_CD			*cdrom=NULL;

//-- Public Variables --------------------------------------------------------
int			cdda_first_drive=0;
int			cdda_current_drive=0;
int			cdda_current_track=0;
int			cdda_current_frame=0;
int			cdda_playing=0;
int			cdda_autoloop=0;
int			cdda_volume=0;
int			cdda_disabled=0;



#ifdef __QNXNTO__

#define ERR( code ) #code

static const char *errlist[] = {
	ERR( MMR_ERROR_NONE ),
	ERR( MMR_ERROR_UNKNOWN ),
	ERR( MMR_ERROR_INVALID_PARAMETER ),
	ERR( MMR_ERROR_INVALID_STATE ),
	ERR( MMR_ERROR_UNSUPPORTED_VALUE ),
	ERR( MMR_ERROR_UNSUPPORTED_MEDIA_TYPE ),
	ERR( MMR_ERROR_MEDIA_PROTECTED ),
	ERR( MMR_ERROR_UNSUPPORTED_OPERATION ),
	ERR( MMR_ERROR_READ ),
	ERR( MMR_ERROR_WRITE ),
	ERR( MMR_ERROR_MEDIA_UNAVAILABLE ),
	ERR( MMR_ERROR_MEDIA_CORRUPTED ),
	ERR( MMR_ERROR_OUTPUT_UNAVAILABLE ),
	ERR( MMR_ERROR_NO_MEMORY ),
	ERR( MMR_ERROR_RESOURCE_UNAVAILABLE )
};
#undef ERR
#define NERRS ( sizeof(errlist) / sizeof(errlist[0]) )

static void mmrerror( mmr_context_t *ctxt, const char *errmsg )
{
	const mmr_error_info_t *err = mmr_error_info( ctxt );
	unsigned errcode = err->error_code;
	const char *name;
	if ( errcode >= NERRS || ( name = errlist[ errcode ] ) == NULL )
	{
		name = "bad error code";
	}
	fprintf(stderr, "%s: error %d (%s)\n", errmsg, errcode, name);
}

#endif

//----------------------------------------------------------------------------
int cdda_get_disk_info()
{
#ifdef ENABLE_CDDA
    if(cdda_disabled) return 1;

#ifdef USE_MP3_CDDA
    cdda_min_track = 0;
    cdda_max_track = cdda_num_tracks();
    cdda_disk_length = cdda_max_track;
    return 1;
#else
    if( CD_INDRIVE(SDL_CDStatus(cdrom)) ) {
        cdda_min_track = 0;
        cdda_max_track = cdrom->numtracks;
        cdda_disk_length = cdrom->numtracks;
        return 1;
    }
    else
    {
        console_printf("Error: No Disc in drive\n");
        cdda_disabled=1;
        return 1;
    }
#endif
#endif
    return 0;
}


//----------------------------------------------------------------------------
#ifdef USE_THREAD_CDDA
static int real_cdda_play(int track)
#else
int cdda_play(int track)
#endif
{
#ifndef SHOW_MENU
	track++;
#endif
#ifdef ENABLE_CDDA
    if(cdda_disabled) return 1;
    
    if(cdda_playing && cdda_current_track==track) return 1;

#ifdef USE_MP3_CDDA
    if (track > 1 && track <= cdda_num_tracks())
    {
    	char fname[256];
    	if (cdda_get_track_name(track, fname))
    	{
    		char fname2[256];
    		memset(fname2, 0, 256);
    		strcpy(fname2, "file://accounts/1000/shared/misc/NEOCD/iso/");
    		strcat(fname2, fname);
    		if (cdda_playing)
    		{
    			mmr_stop(ctxt);
    			mmr_input_detach(ctxt);
    		}
			if (mmr_input_attach(ctxt, fname2, INPUT_TYPE) < 0)
				mmrerror(ctxt, fname2);
			else if (mmr_play(ctxt))
				mmrerror(ctxt, "mmr_play");
			cdda_current_track = track;
			cdda_loop_counter = 0;
			cdda_track_end = cdda_get_track_end(track);
			cdda_playing = 1;
			return 1;
    	}
    }
#else

    if( CD_INDRIVE(SDL_CDStatus(cdrom)) ) {
    	SDL_CDPlayTracks(cdrom, track-1, 0, 1, 0);
    	cdda_current_track = track;
    	cdda_loop_counter=0;
    	cdda_track_end=(cdrom->track[track-1].length*60)/CD_FPS;//Length in 1/60s of second
    	cdda_playing = 1;
#ifndef USE_THREAD_CDDA
        init_autoframeskip();
#endif
    	return 1;
    } 
    else
    { 
        cdda_disabled = 1;
        return 1;
    }
#endif
#endif
    return 0;
}

//----------------------------------------------------------------------------
void cdda_pause()
{
	if(cdda_disabled) return;
#ifdef ENABLE_CDDA
#ifdef USE_MP3_CDDA
#ifdef __QNXNTO__
	mmr_speed_set(ctxt, 0);
#endif
#else
	SDL_CDPause(cdrom);
#endif
#endif
	cdda_playing = 0;
}


void cdda_stop()
{
	if(cdda_disabled) return;
#ifdef ENABLE_CDDA
#ifdef USE_MP3_CDDA
#ifdef __QNXNTO__
	mmr_stop(ctxt);
#endif
#else
	SDL_CDStop(cdrom);
#endif
#endif
	cdda_playing = 0;
}

//----------------------------------------------------------------------------
void cdda_resume()
{
	if(cdda_disabled || cdda_playing) return;
#ifdef ENABLE_CDDA
#ifdef USE_MP3_CDDA
#ifdef __QNXNTO__
	mmr_speed_set(ctxt, 1000);
#endif
#else
	SDL_CDResume(cdrom);
#endif
#endif
	cdda_playing = 1;
}

//----------------------------------------------------------------------------
void cdda_loop_check()
{
#ifdef ENABLE_CDDA
	if(cdda_disabled) return;
	if (cdda_playing==1) {
		cdda_loop_counter++;
		if (cdda_loop_counter>=cdda_track_end) {
			if (cdda_autoloop)
				cdda_play(cdda_current_track);
			else
				cdda_stop();
		}
	}
#endif
}

#ifdef USE_THREAD_CDDA

static __inline__ int datospendientes(void)
{
	int ret;
	SDL_mutexP(mtx_procesando);
	ret=(iw_cola!=ir_cola);
	SDL_mutexV(mtx_procesando);
	return ret;
}

static __inline__ int leecola(void)
{
	int valor;
	SDL_mutexP(mtx_procesando);
	ir_cola=(ir_cola+1)%MAX_COLA;
	valor=cola[ir_cola];
	SDL_mutexV(mtx_procesando);
	return valor;
}

int cdda_play(int track)
{
	if (track>=0)
	{
		SDL_mutexP(mtx_procesando);
		iw_cola=(iw_cola+1)%MAX_COLA;
		cola[iw_cola]=track;
		SDL_mutexV(mtx_procesando);
		SDL_SemPost(sem_atiende);
		return 1;
	}
	return 0;
}

unsigned char cpy_neogeo_memorycard[NEO4ALL_MEMCARD_SIZE];
extern int real_save_savestate(void);
int save_savestate(void)
{
	memcpy(cpy_neogeo_memorycard,neogeo_memorycard,NEO4ALL_MEMCARD_SIZE);
	SDL_mutexP(mtx_procesando);
	iw_cola=(iw_cola+1)%MAX_COLA;
	cola[iw_cola]=-1;
	SDL_mutexV(mtx_procesando);
	SDL_SemPost(sem_atiende);
	return 0;
}

static int mithread_alive=0;

static int mithread(void *data)
{
	mithread_alive=1;
	while(mithread_alive)
	{
		SDL_SemWait(sem_atiende);
		if (datospendientes())
		{
			int valor=leecola();
			if (valor>=0)
				real_cdda_play(valor);
			else
				real_save_savestate();
		}
	}
	return 0;
}

#endif


//----------------------------------------------------------------------------
int	cdda_init()
{
#ifdef ENABLE_CDDA
	cdda_min_track = cdda_max_track = 0;
	cdda_current_track = 0;
	cdda_playing = 0;
	cdda_loop_counter = 0;
	if (cdda_num_tracks() == 0)
		cdda_disabled = 1;
	else
		cdda_disabled = 0;

#ifdef USE_MP3_CDDA
#ifdef __QNXNTO__
	mode_t mode = S_IRUSR | S_IXUSR;
	strm_dict_t *aoparams = NULL;
	int audio_oid;
	if ((connection = mmr_connect(NULL)) == NULL)
		fprintf(stderr, "mmr_connect: %s\n", strerror(errno));
	else if ((ctxt = mmr_context_create(connection, PLAYER_NAME, 0, mode)) == NULL)
		fprintf(stderr, "%s %s\n", PLAYER_NAME, strerror(errno));
	else if ((audio_oid = mmr_output_attach(ctxt, AUDIO_OUT, "audio")) < 0)
		mmrerror(ctxt, AUDIO_OUT);
	else if (aoparams && mmr_output_parameters(ctxt, audio_oid, aoparams))
		mmrerror(ctxt, "Output parameters (audio)");
	cdda_disabled=0;
#endif
#else
	/* Open the default drive */
	cdrom=SDL_CDOpen(cdda_current_drive);

	/* Did if open? Check if cdrom is NULL */
	if(cdrom == NULL){
		console_printf("Couldn't open drive %s for audio.  %s\n", SDL_CDName(cdda_current_drive), SDL_GetError());
		cdda_disabled=1;
		return 1;
	} else {
		cdda_disabled=0;
#endif
#ifdef USE_THREAD_CDDA
		if (!sem_atiende)
			sem_atiende=SDL_CreateSemaphore(0);
		if (!mtx_procesando)
			mtx_procesando=SDL_CreateMutex();
		if (!mithread_alive)
			SDL_CreateThread(mithread,0);
#endif
#ifdef USE_MP3_CDDA
#else
		console_printf("CD Audio OK!\n");
	}
#endif

	cdda_get_disk_info();
#else
	cdda_disabled=1;
#endif
	return 0;
}

//----------------------------------------------------------------------------
void cdda_shutdown()
{
#ifdef ENABLE_CDDA
	if(cdda_disabled) return;
#ifdef USE_MP3_CDDA
#ifdef __QNXNTO__
	if (cdda_playing)
	{
		mmr_stop(ctxt);
		mmr_input_detach(ctxt);
	}
	mmr_context_destroy(ctxt);
	mmr_disconnect(connection);
#endif
#else
	if (cdrom)
	{
		SDL_CDStop(cdrom);
		SDL_CDClose(cdrom);
	}
#endif
	cdrom=NULL;
	cdda_disabled=1;
#ifdef USE_THREAD_CDDA
	mithread_alive=0;
	SDL_Delay(234);
	if (sem_atiende)
		SDL_DestroySemaphore(sem_atiende);
	if (mtx_procesando)
		SDL_DestroyMutex(mtx_procesando);
#endif
#endif
}

#else
int	cdda_disabled=1; // DISABLED CDDA
#endif
