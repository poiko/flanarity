#pragma once


struct node
{
	float x, y;
};

struct edge
{
	int n1, n2;
};


extern int g_winwidth, g_winheight;
extern int g_clientwidth, g_clientheight;
extern int g_fieldwidth, g_fieldheight;


void InitSettings(const char *cfgfile);
void UpdateSettings();
void SaveSettings(const char *cfgfile);

bool InitGame();
void NewGame();
void RenderGame();

int FindNode(float x, float y);
void SetNode(int n, float x, float y);
