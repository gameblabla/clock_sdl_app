#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include "font_drawing.h"

#ifndef SDL_TRIPLEBUF
#define SDL_TRIPLEBUF SDL_DOUBLEBUF
#endif

SDL_Surface* sdl_screen, *backbuffer;
uint32_t key_pressed = 0;

uint8_t tmp_str[64];
int buttons;

uint32_t date_selected = 1, month_selected = 1, year_selected = 1970, hour_selected = 0, minute_selected = 0, seconds_selected = 0;
uint32_t february_days = 28;
uint32_t update_clock = 0;

uint8_t final_long_string[512];

uint8_t string_tmp[2][512];

int select_cursor = 0;

/* I could probably make this smaller */
static void Dont_go_over_days()
{
	if (date_selected < 1)
	{
		date_selected = 1;
	}
	
	if (month_selected > 12)
	{
		month_selected = 12;
	}
	
	if (month_selected < 1)
	{
		month_selected = 1;
	}
	
	if (year_selected > 2100)
	{
		year_selected = 2100;
	}
	
	if (year_selected < 1970)
	{
		year_selected = 1970;
	}
	
	switch(month_selected)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		case 11:
			if (date_selected > 31) date_selected = 31;
		break;
		case 4:
		case 6:
		case 8:
		case 10:
		case 12:
			if (date_selected > 30) date_selected = 30;
		break;
		case 2:
			if (date_selected > february_days) date_selected = february_days;
		break;
	}
}

static void Check_leap_year()
{
	if (year_selected % 4 == 0 && year_selected%100 != 0 || year_selected % 400 == 0) february_days = 29;
	else february_days = 28;	
}

static void Dont_go_over_hour()
{
	if (hour_selected > 23)
	{
		hour_selected = 0;
	}
	
	if (minute_selected > 59)
	{
		minute_selected = 0;
	}
	
	if (seconds_selected > 59)
	{
		seconds_selected = 0;
	}
}

int main (int argc, char *argv[]) 
{
	SDL_Event event;
	int quit = 0;
	int i;
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_ShowCursor(0);
	sdl_screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_TRIPLEBUF);
	if (!sdl_screen)
	{
		printf("Can't set video mode\n");
		return 0;
	}
	backbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, sdl_screen->w, sdl_screen->h, sdl_screen->format->BytesPerPixel*8, 0, 0,0,0); 

	/* Actual time is retrieved */
	/* Here, we don't need to check if year is a leap year as it's being taken care 
	 * of by the system but when the user change the date, we do need to detect it.
	 * */

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	date_selected = tm.tm_mday;
	month_selected = tm.tm_mon + 1;
	year_selected = tm.tm_year + 1900;
	hour_selected = tm.tm_hour;
	minute_selected = tm.tm_min;
	seconds_selected = tm.tm_sec;
	
	SDL_EnableKeyRepeat(500, 50);
	
	while(!quit)
	{
		while (SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{
				case SDL_KEYUP:
					switch(event.key.keysym.sym) 
					{
						/* SDLK_HOME for OD needs the Power button to be detected upon release. */
						case SDLK_HOME:
							quit = 1;
						break;
						default:
						break;
					}
				break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) 
					{
						case SDLK_UP:
						switch(select_cursor)
						{
							case 0:
								date_selected++;
							break;
							case 1:
								month_selected++;
							break;
							case 2:
								year_selected++;
							break;
							case 3:
								hour_selected++;
							break;
							case 4:
								minute_selected++;
							break;
							case 5:
								seconds_selected++;
							break;
						}
						break;
						case SDLK_DOWN:
						switch(select_cursor)
						{
							case 0:
								date_selected--;
							break;
							case 1:
								month_selected--;
							break;
							case 2:
								year_selected--;
							break;
							case 3:
								hour_selected--;
							break;
							case 4:
								minute_selected--;
							break;
							case 5:
								seconds_selected--;
							break;
						}
						break;
						case SDLK_LEFT:
						select_cursor--;
						if (select_cursor < 0) select_cursor = 0;
						break;
						case SDLK_RIGHT:
						select_cursor++;
						if (select_cursor > 5) select_cursor = 5;
						break;
						case SDLK_RETURN:
							quit = 1;
							update_clock = 1;
						break;
						default:
						break;
					}
				break;
				case SDL_QUIT:
					quit = 1;
				break;
			}
		}
		
		Check_leap_year();
		Dont_go_over_days();
		Dont_go_over_hour();
		
		SDL_FillRect(backbuffer, NULL, 0);
		
		print_string("Please set the Clock", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 20, 5, backbuffer->pixels);
		
		/* Make sure to add the zeros for the day and month if they are inferior to 10 */
		if (date_selected < 10)
		{
			snprintf(string_tmp[0], sizeof(string_tmp[0]), "0%d", date_selected);
		}
		else
		{
			snprintf(string_tmp[0], sizeof(string_tmp[0]), "%d", date_selected);
		}
		
		if (month_selected < 10)
		{
			snprintf(string_tmp[1], sizeof(string_tmp[1]), "0%d", month_selected);
		}
		else
		{
			snprintf(string_tmp[1], sizeof(string_tmp[1]), "%d", month_selected);	
		}
		
		snprintf(tmp_str, sizeof(tmp_str), "%s / %s / %d %d : %d : %d", string_tmp[0], string_tmp[1], year_selected, hour_selected, minute_selected, seconds_selected);
		print_string(tmp_str, SDL_MapRGB(sdl_screen->format,255,255,255), 0, 20, 30, backbuffer->pixels);
		
		print_string("^^", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 20 + (select_cursor * 40), 50, backbuffer->pixels);
		
		print_string("Start : Quit/Update time", SDL_MapRGB(sdl_screen->format,255,255,255), 0, 20, 70, backbuffer->pixels);
		
		/* Print back buffer to the final screen */
		SDL_BlitSurface(backbuffer, NULL, sdl_screen, NULL);
		
		/* Flip the screen so it gets displayed*/
		SDL_Flip(sdl_screen);
	}
	
	/* Make sure to clean up the allocated surfaces before exiting.
	 * Most of the time when an SDL app leaks, it's for that reason.
	 * */
	if (sdl_screen)
	{
		SDL_FreeSurface(sdl_screen);
		sdl_screen = NULL;
	}
	if (backbuffer)
	{
		SDL_FreeSurface(backbuffer);
		backbuffer = NULL;
	}
	
	SDL_Quit();
	
	if (update_clock == 1)
	{
		/* Parse our selected year, month, hour... to final_long_string so it can then be executed. */
		snprintf(final_long_string, sizeof(final_long_string), "date -s '%d-%d-%d %d:%d:%d'", year_selected, month_selected, date_selected, hour_selected, minute_selected, seconds_selected);
		
		/* Executes the command to update both the system clock & the hardware clock if available */
		execlp("/bin/sh","/bin/sh", "-c", final_long_string, (char *)NULL);
		execlp("/bin/sh","/bin/sh", "-c", "hwclock --utc -w", (char *)NULL);
	}
	
	return 0;
}
