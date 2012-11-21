/*
 * state.cpp
 *
 *  Created on: 2012-11-19
 *      Author: Travis Redpath
 */

#include "config.h"

#include <SDL.h>
#include <SDL_endian.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "memory/memory.h"
#include "state.h"
#include "neo4all.h"

#if 0

static ST_REG *reglist;
static ST_MODULE st_mod[ST_MODULE_END];
static SDL_Rect buf_rect = { 24, 16, 320, 224 };
static SDL_Rect screen_rect = { 0, 0, 320, 224 };
SDL_Surface *state_img_tmp;


void create_state_register(ST_MODULE_TYPE module, const char* reg_name, Uint8 num, void *data, int size, ST_DATA_TYPE type)
{
	ST_REG *t = (ST_REG*)calloc(1, sizeof(ST_REG));
	t->next = st_mod[module].reglist;
	st_mode[module].reglist = t;
	t->reg_name = strdup(reg_name);
	t->data = data;
	t->size = size;
	t->type = type;
	t->num = num;
}

void set_pre_save_function(ST_MODULE_TYPE module, void (*func)(void))
{
	st_mod[module].pre_save_state = func;
}

void set_post_load_function(ST_MODULE_TYPE module, void (*func)(void))
{
	st_mod[module].post_load_state = func;
}

static void* find_data_by_name(ST_MODULE_TYPE module, Uint8 num, char* name)
{
	ST_REG* t = st_mod[module].reglist;
	while (t)
	{
		if ((!strcmp(name, t->reg_name)) && (t->num == num))
		{
			return t->data;
		}
		t = t->next;
	}
	return NULL;
}

static int sizeof_st_type(ST_DATA_TYPE type)
{
	switch(type)
	{
	case REG_UINT8:
	case REG_INT8:
		return 1;
	case REG_UINT16:
	case REG_INT16:
		return 2;
	case REG_UINT32:
	case REG_INT32:
		return 4;
	}
	return 0;
}

void swap_buf16_if_need(Uint8 src_endian, Uint16* buf, Uint32 size)
{
	int i;
#ifdef WORDS_BIGENDIAN
	Uint8 my_endian = 1;
#else
	Uint8 my_endian = 0;
#endif
	if (my_endia != src_endian)
	{
		for (i = 0; i < size; i++)
			SDL_Swap16(buf[i]);
	}
}

void swap_buf32_if_need(Uint8 src_endian,Uint32* buf,Uint32 size)
{
    int i;
#ifdef WORDS_BIGENDIAN
    Uint8  my_endian=1;
#else
    Uint8  my_endian=0;
#endif
    if (my_endian!=src_endian)
    {
        for (i = 0; i < size; i++)
            buf[i] = SDL_Swap32(buf[i]);
    }
}

Uint32 how_many_slot(char *game)
{
	char *st_name;
	FILE *f;
	char *dir="shared/misc/NEOCD/save/";
	Uint32 slot=0;
	st_name=(char*)alloca(strlen(dir)+strlen(game)+5);
	while (1)
	{
		sprintf(st_name,"%s%s.%03d",dir,game,slot);
		if (st_name && (f=fopen(st_name,"rb")))
		{
			fclose(f);
			slot++;
		}
		else
			return slot;
	}
}



SDL_bool load_state(char *game,int slot)
{
    char *st_name;
#ifdef __QNXNTO__
     char *gngeo_dir = "shared/misc/gngeo/save/";
#else
     char *gngeo_dir=get_gngeo_dir();
#endif

#ifdef WORDS_BIGENDIAN
    Uint8  my_endian=1;
#else
    Uint8  my_endian=0;
#endif

    int i;
    gzFile *gzf;
    char string[20];
    Uint8 a,num;
    ST_DATA_TYPE type;
    void *data;
    Uint32 len;

    Uint8  endian;
    Uint32 rate;

    st_name=(char*)alloca(strlen(gngeo_dir)+strlen(game)+5);
    sprintf(st_name,"%s%s.%03d",gngeo_dir,game,slot);

    if ((gzf=gzopen(st_name,"rb"))==NULL)
    {
		printf("%s not found\n", st_name);
		return SDL_FALSE;
    }

    memset(string, 0, 20);
    gzread(gzf, string, 6);

    if (strcmp(string,"GNGST1"))
    {
		printf("%s is not a valid Neo Geo CD st file\n",st_name);
		gzclose(gzf);
		return SDL_FALSE;
    }

    gzread(gzf,&endian,1);

    if (my_endian!=endian)
    {
		printf("This save state comme from a different endian architecture.\n"
			   "This is not currently supported :(\n");
		return SDL_FALSE;
    }

	/* enable sound */
#ifdef Z80_EMULATED
	init_sdl_audio();
#ifndef AES
	_z80_init();
#endif
#endif
	SDL_PauseAudio(0);

    gzread(gzf, state_img->pixels, 304*224*2);
    swap_buf16_if_need(endian, state_img->pixels, 304*224);

    while(!gzeof(gzf))
    {
		gzread(gzf, &a, 1); /* name size */
		memset(string, 0, 20);
		gzread(gzf, string, a); /* regname */
		gzread(gzf, &num, 1); /* regname num */
		gzread(gzf, &a, 1); /* module id */
		gzread(gzf, &len, 4);
		gzread(gzf, &type, 1);
		data = find_data_by_name(a, num, string);
		if (data)
		{
			gzread(gzf, data, len);
			switch(type)
			{
			case REG_UINT16:
			case REG_INT16:
				swap_buf16_if_need(endian, data, len >> 1);
				break;
			case REG_UINT32:
			case REG_INT32:
				swap_buf32_if_need(endian, data, len >> 2);
				break;
			case REG_INT8:
			case REG_UINT8:
				/* nothing */
				break;
			}
		}
		else
		{
			/* unknow reg, ignore it*/
			printf("skipping unknown reg %s\n", string);
			gzseek(gzf, len, SEEK_CUR);
		}
    }
    gzclose(gzf);

    for(i = 0; i < ST_MODULE_END; i++)
    {
    	if (st_mod[i].post_load_state)
    		st_mod[i].post_load_state();
    }

    return SDL_TRUE;
}

SDL_bool save_state(char *game,int slot) {
     char *st_name;
#ifdef __QNXNTO__
     char *gngeo_dir = "shared/misc/gngeo/save/";
#else
     char *gngeo_dir=get_gngeo_dir();
#endif

    Uint8 i;
    gzFile *gzf;
    char string[20];
    Uint8 a,num;
    ST_DATA_TYPE type;
    void *data;
    Uint32 len;
#ifdef WORDS_BIGENDIAN
    Uint8  endian=1;
#else
    Uint8  endian=0;
#endif

    st_name = (char*)alloca(strlen(gngeo_dir) + strlen(game) + 5);
    sprintf(st_name, "%s%s.%03d", gngeo_dir, game, slot);

    if ((gzf = gzopen(st_name, "wb")) == NULL)
    {
		printf("can't write to %s\n", st_name);
		return SDL_FALSE;
    }

    SDL_BlitSurface(buffer, &buf_rect, state_img, &screen_rect);

    gzwrite(gzf, "GNGST1", 6);
    gzwrite(gzf, &endian, 1);
    gzwrite(gzf, state_img->pixels, 304*224*2);
    for(i=0;i<ST_MODULE_END;i++)
    {
		ST_REG *t = st_mod[i].reglist;
		if (st_mod[i].pre_save_state)
			st_mod[i].pre_save_state();
		while(t)
		{
			a=strlen(t->reg_name);
			gzwrite(gzf, &a, 1); /* strlen(regname) */
			gzwrite(gzf, t->reg_name, strlen(t->reg_name)); /* regname */
			gzwrite(gzf, &t->num, 1); /* regname num */
			gzwrite(gzf, &i, 1); /* module id */
			gzwrite(gzf, &t->size, 4);
			gzwrite(gzf, &t->type, 1);
			gzwrite(gzf, t->data, t->size);

			t = t->next;
		}
    }
    gzclose(gzf);
    return SDL_TRUE;
}

/* neogeo state register */
static Uint8 st_current_pal, st_current_fix;

static void neogeo_pre_save_state(void)
{
    st_current_pal = (current_pal == memory.pal1 ? 0 : 1);
    st_current_fix = (current_fix == memory.sfix_board ? 0 : 1);
}

static void neogeo_post_load_state(void)
{
    current_pal=(st_current_pal==0?memory.pal1:memory.pal2);
    current_pc_pal=(st_current_pal==0?memory.pal_pc1:memory.pal_pc2);
    current_fix=(st_current_fix==0?memory.sfix_board:memory.sfix_game);
    update_all_pal();
}

#endif









