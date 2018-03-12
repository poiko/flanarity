#pragma once


struct Node
{
	float x, y;

	Node(): x(0), y(0) {}
	Node(float _x, float _y): x(_x), y(_y) {}
	
	Node &operator+=(const Node &rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}

	Node &operator-=(const Node &rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}
};

struct Edge
{
	int n[2];
};


extern int g_winwidth, g_winheight;
extern int g_clientposx, g_clientposy;
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
void UpdateNodeTangle(int node);
bool CheckUntangled();
