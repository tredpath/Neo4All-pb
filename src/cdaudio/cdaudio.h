/**************************************
****   CDAUDIO.H  -  CD-DA Player  ****
****         Header File           ****
**************************************/

#ifndef	CDAUDIO_H
#define CDAUDIO_H

#ifdef ENABLE_CDDA

//-- Exported Variables ------------------------------------------------------
extern int	cdda_first_drive;
extern int	cdda_nb_of_drives;
extern int	cdda_current_drive;
extern int	cdda_current_track;
extern int	cdda_playing;
extern char	drive_list[32];
extern int	nb_of_drives;
extern int	cdda_autoloop;

//-- Exported Functions ------------------------------------------------------
int	cdda_init();
int	cdda_play(int);
void	cdda_pause();
void	cdda_stop();
void	cdda_resume();
void	cdda_shutdown();
void	cdda_loop_check();
int 	cdda_get_disk_info();
void	cdda_build_drive_list();
int	cdda_get_volume();
void	cdda_set_volume(int volume);

#ifdef USE_MP3_CDDA
int cdda_num_tracks();
int cdda_get_track_name(int index, char* name);
int cdda_get_track_end(int track);
#endif

#else

#define cdda_init() (0)
#define cdda_play(A) (0)
#define cdda_pause()
#define cdda_stop()
#define cdda_resume()
#define cdda_shutdown()
#define cdda_loop_check()
#define cdda_get_disk_info() (0)
#define cdda_build_drive_list()
#define cdda_get_volume() (0)
#define cdda_set_volume()

#endif

#endif /* CDAUDIO_H */

