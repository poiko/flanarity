#pragma once

struct settings
{
	int numverts;
	int vertsize;
	int fieldwidth, fieldheight;
};

extern settings game_settings;

void InitSettings(const char *cfgfile);
void SaveSettings(const char *cfgfile);

bool InitGame();
void NewGame(int numverts);
void RenderGame();
