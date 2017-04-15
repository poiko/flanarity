#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "game.h"

const int DEFAULT_NUMVERTS = 100;
const int DEFAULT_VERTSIZE = 10;


struct vertex
{
	float x, y;
};

struct edge
{
	int v1, v2;
};

struct geometry
{
	GLuint vao;
	int numvertices;
	vertex *vertices;
	GLuint vertvbo;
	int numedges;
	edge *edges;
};

settings game_settings;
geometry game_geometry;
bool game_active;

void LoadTextFile(char *file, const char **data)
{
	std::ifstream f(file);
	std::string line, str;
	while (std::getline(f, line))
		str += line + "\n";
	*data = _strdup(str.c_str());
}

void InitSettings(const char *cfgfile)
{
	game_settings.numverts = DEFAULT_NUMVERTS;
	game_settings.vertsize = DEFAULT_VERTSIZE;

	char key[64];
	char value[64];
	std::ifstream f(cfgfile);
	std::string line;
	while (std::getline(f, line))
	{
		int len = line.length();
		int i = 0;
		//skip spaces
		while ((i < len) && (isspace(line[i])))
			i++;
		int j = 0;
		//read key	
		while ((i < len) && isalnum(line[i]) && (j < sizeof(key)-1))
			key[j++] = line[i++];
		key[j] = 0;
		//skip spaces
		while ((i < len) && (isspace(line[i])))
			i++;
		if (line[i++] != '=')
			continue;
		//skip spaces
		while ((i < len) && (isspace(line[i])))
			i++;
		//read value
		j = 0;
		while ((i < len) && isalnum(line[i]) && (j < sizeof(value)-1))
			value[j++] = line[i++];
		value[j] = 0;

		if (!strcmp(key, "numverts"))
			game_settings.numverts = atoi(value);
		else if (!strcmp(key, "vertsize"))
			game_settings.vertsize = atoi(value);
		//etc
	}
}

void SaveSettings(const char *cfgfile)
{
	std::ofstream f(cfgfile);

	f << "numverts = " << game_settings.numverts << std::endl;
	f << "vertsize = " << game_settings.vertsize << std::endl;
}

bool InitGame()
{
	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	game_geometry.vao = vao;	
	glGenBuffers(1, &game_geometry.vertvbo);

	const char *vshdata;
	LoadTextFile("flanarity.vsh", &vshdata);
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vshdata, nullptr);
	glCompileShader(vshader);

	GLint status;
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
		printf("Vertex shader compile failed!\n");
	char buffer[512];
	glGetShaderInfoLog(vshader, 512, NULL, buffer);
	printf("%s\n", buffer);

	const char *pshdata;
	LoadTextFile("flanarity.psh", &pshdata);
	GLuint pshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pshader, 1, &pshdata, nullptr);
	glCompileShader(pshader);

	glGetShaderiv(pshader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
		printf("Pixel shader compile failed!\n");
	glGetShaderInfoLog(pshader, 512, NULL, buffer);
	printf("%s\n", buffer);

	GLuint shprogram = glCreateProgram();
	glAttachShader(shprogram, vshader);
	glAttachShader(shprogram, pshader);
	glBindFragDataLocation(shprogram, 0, "out_color");
	glLinkProgram(shprogram);
	glUseProgram(shprogram);
	
	glBindBuffer(GL_ARRAY_BUFFER, game_geometry.vertvbo);
	GLint posattrib = glGetAttribLocation(shprogram, "in_pos");
	glVertexAttribPointer(posattrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(posattrib);
	GLint midattrib = glGetAttribLocation(shprogram, "in_mid");
	glVertexAttribPointer(midattrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(midattrib);

	//GLint colattrib = glGetAttribLocation(shprogram, "color");
	//glVertexAttribPointer(colattrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
	//glEnableVertexAttribArray(colattrib);

	game_active = false;

	return true;
}


struct vbovertex
{
	float x, y;
	float midx, midy;
};

void NewGame(int numverts)
{
	delete game_geometry.vertices;
	delete game_geometry.edges;
	numverts = 3;
	vertex *verts = new vertex[numverts];
	
	verts[0].x = -0.5f;
	verts[0].y = -0.5f;
	verts[1].x = 0.5f;
	verts[1].y = -0.5f;
	verts[2].x = 0;
	verts[2].y = 0.5f;

	int numedges = 3;
	edge *edges = new edge[numedges];
	edges[0].v1 = 0;
	edges[0].v2 = 1;
	edges[1].v1 = 1;
	edges[1].v2 = 2;
	edges[2].v1 = 2;
	edges[2].v2 = 0;

	vbovertex *vboverts = new vbovertex[numverts*6];
	for (int i=0; i<numverts; i++)
	{
		vboverts[i*6+0].x = verts[i].x - 0.1f;
		vboverts[i*6+0].y = verts[i].y - 0.1f;
		vboverts[i*6+1].x = verts[i].x + 0.1f;
		vboverts[i*6+1].y = verts[i].y - 0.1f;
		vboverts[i*6+2].x = verts[i].x + 0.1f;
		vboverts[i*6+2].y = verts[i].y + 0.1f;

		vboverts[i*6+3].x = verts[i].x - 0.1f;
		vboverts[i*6+3].y = verts[i].y - 0.1f;
		vboverts[i*6+4].x = verts[i].x + 0.1f;
		vboverts[i*6+4].y = verts[i].y + 0.1f;
		vboverts[i*6+5].x = verts[i].x - 0.1f;
		vboverts[i*6+5].y = verts[i].y + 0.1f;

		for (int j=0; j<6; j++)
		{
			vboverts[i*6+j].midx = verts[i].x;
			vboverts[i*6+j].midy = verts[i].y;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, game_geometry.vertvbo);
	glBufferData(GL_ARRAY_BUFFER, numverts*sizeof(vbovertex)*6, vboverts, GL_DYNAMIC_DRAW);

//	delete vboverts;

	game_geometry.vertices = verts;
	game_geometry.numvertices = numverts;
	game_geometry.edges = edges;
	game_geometry.numedges = numedges;

	game_active = true;
}

void SaveGame(const char *gamefile)
{
}

void LoadGame(const char *gamefile)
{
}

static void RenderVertices()
{
	glDrawArrays(GL_TRIANGLES, 0, game_geometry.numvertices*6);
}

static void RenderEdges()
{
}

void RenderGame()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (game_active)
	{
		RenderEdges();
		RenderVertices();
	}
}
