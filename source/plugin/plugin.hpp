/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _PLUGIN_HPP_
#define _PLUGIN_HPP_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "config/config.hpp"
#include "loader/disc.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ios;

#define TAG_GAME_ID		"{gameid}"
#define TAG_LOC			"{loc}"
#define TAG_CONSOLE		"{console}"

#define PLUGIN			"PLUGIN"
#define PLUGIN_DEV		"{device}"
#define PLUGIN_PATH		"{path}"
#define PLUGIN_NAME		"{name}"
#define PLUGIN_NOEXT	"{name_no_ext}"
#define PLUGIN_LDR		"{loader}"

struct PluginOptions
{
	string path;
	u32 magic;
	u32 caseColor;
	int romPartition;
	string romDir;
	string explorerPath; //
	bool fileNamesAsTitles; //
	string guideName; //
	string fileTypes;	
	string DolName;
	string coverFolder;
	// string consoleCoverID;
	string foldersToSkip; //
	string BannerSound;
	u32 BannerSoundSize;
	vector<string> Args;
	wstringEx DisplayName;
	s8 boxMode;
	bool state;
};

class Plugin
{
public:
	u8 *GetBannerSound(u32 magic);
	u32 GetBannerSoundSize();
	u32 GetCaseColor(u8 pos);
	const char *GetDolName(u32 magic);
	const char *GetCoverFolderName(u32 magic);
	const char *GetRomDir(u8 pos);
	const char *GetExplorerPath(u32 magic); //
	bool GetFileNamesAsTitles(u8 pos); //
	string GetGuideName(u32 magic); //
	const string& GetFoldersToSkip(u32 magic); //
	string GetRomName(const char *FullPath);
	string GetRomId(char *romPath, u32 Magic, Config &m_crc, const char *datadir, const char *platform, const char *name);
	int GetRomPartition(u8 pos);
	const string& GetFileTypes(u8 pos);
	wstringEx GetPluginName(u8 pos);
	string GetPluginPath(u8 pos);
	u32 GetPluginMagic(u8 pos);
	u8 GetPluginPosition(u32 magic);
	s8 GetBoxMode(u8 pos);
	bool GetEnabledStatus(u8 pos);
	bool GetEnabledStatus(const char *magic);
	const vector<bool> &GetEnabledPlugins(u8 *num);
	
	void SetRomDir(u8 pos, const string &rd);
	void SetRomPartition(u8 pos, int part);
	void SetExplorerPath(u8 pos, const string &ep); //
	void SetCaseColor(u8 pos, u32 caseColor);
	void SetFileNamesAsTitles(u8 pos, bool FileNamesAsTitles); //
	void SetEnablePlugin(u8 pos, u8 ForceMode = 0);
	void SetCoverFolderName(u8 pos, const string &cf); //
	void SetPluginName(u8 pos, const string &cf); //
	void SetBoxMode(u8 pos, int mode); //

	void init(const string& m_pluginsDir);
	void AddPlugin(Config &plugin, const string &iniPath);
	void Cleanup();
	bool PluginExist(u8 pos);
	
	vector<string> CreateArgs(const char *device, const char *path, 
		const char *title, const char *loader, u32 title_len_no_ext, u32 magic);
	char PluginMagicWord[9];
	
private:
	vector<PluginOptions> Plugins;
	vector<bool> enabledPlugins;
	s16 Plugin_Pos;
	string pluginsDir;
};

extern Plugin m_plugin;

#endif
