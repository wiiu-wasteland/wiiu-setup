/*
 * wiiu-setup
 * A small utility to setup wiiu's sdcard for homebrew
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <wiiu/os.h>
#include <wiiu/procui.h>
#include <wiiu/avm.h>
#include <sys/socket.h>
#include <system/memory.h>
#include "zip/zip.h"


/* Configuration */

const char *pkg_install_list[2][] =
{
	{"payload.elf", "https://static.wiidatabase.de/JSTypeHax-Payload.zip"},
	{"Homebrew Launcher", "http://wiiubru.com/appstore/zips/homebrew_launcher.zip")},
	{"Homebrew Appstore", "http://wiiubru.com/appstore/zips/appstore.zip"},
};

const char *skip_file_list[] =
{
	"manifest.install",
	"info.json"
};

const char *pkg_tmp_path = "wiiu-setup-tmp.zip";


/* Installer */

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

void download_package(char *url, char *path)
{
	
}

int make_file_path(const char* name)
{
	int err = 0;
	char *_name = strdup(name), *p;
	for (p = strchr(_name+1, '/'); p; p = strchr(p+1, '/'))
	{
		*p = '\0';
		err = mkdir(_name, 0775) == -1;
		err = err && (errno != EEXIST);
		if (err)
			break;
		*p = '/';
	}
	free(_name);
	return !err;
}

void extract_package(char *path)
{
	struct zip_t *zip = zip_open(path, 0, 'r');
	for (int i = 0; i < zip_total_entries(zip); i++)
	{
		zip_entry_openbyindex(zip, i);
		
		// get file name
		const char *name = zip_entry_name(zip);
	
		// check if the file should be skipped
		int skip = 0;
		for (int j = 0; j < ARRAY_LENGTH(skip_file_list); j++)
			skip |= strcmp(name, skip_file_list[j]) == 0;
		
		// extract the file
		if (!skip && make_file_path(name))
			zip_entry_fread(zip, name);
		
		zip_entry_close(zip);
	}
	zip_close(zip);
}

void install_package(char *url)
{
	download_package(url, pkg_tmp_path);
	extract_package(pkg_tmp_path);
	remove(pkg_tmp_path);
}

void screen_write(const char *fmt, ...)
{
	static int screen_init = 0;
	static int screen_line = 0;
	static unsigned char *tv_buffer = NULL;
	static unsigned char *drc_buffer = NULL;
	
	if (!screen_init)
	{
		OSScreenInit();
		tv_buffer = MEM1_alloc(OSScreenGetBufferSizeEx(SCREEN_TV), 0x40);
		drc_buffer =  MEM1_alloc(OSScreenGetBufferSizeEx(SCREEN_DRC), 0x40);
		OSScreenSetBufferEx(SCREEN_TV, tv_buffer);
		OSScreenSetBufferEx(SCREEN_DRC, drc_buffer);
		OSScreenEnableEx(SCREEN_TV, TRUE);
		OSScreenEnableEx(SCREEN_DRC, TRUE);
		OSScreenClearBufferEx(SCREEN_TV, 0);
		OSScreenClearBufferEx(SCREEN_DRC, 0);
		screen_init = 1;
	}
	else if (!fmt)
	{
		OSScreenShutdown();
		MEM1_free(tv_buffer);
		MEM1_free(drc_buffer);
		screen_init = 0;
		return;
	}
	
	va_list args;
	char buffer[256];
	va_start(args, fmt);
	vsnprintf(buffer, 256, fmt, args);	
	OSScreenPutFontEx(SCREEN_TV, 0, screen_line, buffer);
	OSScreenPutFontEx(SCREEN_DRC, 0, screen_line, buffer);
	DCFlushRange(tv_buffer);
	DCFlushRange(drc_buffer);
	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
	screen_line++;
	va_end(args);	
}

int main(int argc, char **argv)
{
	int line = 0;
	
	socket_lib_init();
	
	for (int i = 0; i < ARRAY_LENGTH(pkg_install_list); i++)
	{
		const char *pkg_name = pkg_install_list[i][0];
		const char *pkg_url = pkg_install_list[i][1];
		screen_write("[*] Downloading %s ...", pkg_name);
		download_package(pkg_url, pkg_tmp_path);
		screen_write("[*] Installing %s ...", pkg_name);
		extract_package(pkg_tmp_path);
	}
	
	screen_write("[!] Install finished! Rebooting in 5 seconds ...");

	OSSleepTicks(5 * (248625000 / 4))
	
	screen_write(NULL);
	OSShutdown(1);
	
	return 0;
}
