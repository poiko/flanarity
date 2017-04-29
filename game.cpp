#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

#include "game.h"

const int DEFAULT_WINWIDTH = 1024;
const int DEFAULT_WINHEIGHT = 768;
const int DEFAULT_FIELDWIDTH = 1024;
const int DEFAULT_FIELDHEIGHT = 768;
const int DEFAULT_NUMNODES = 100;
const int DEFAULT_NODESIZE = 20;

#define SQR(x) ((x)*(x))

struct vbovertex
{
	float x, y;
	float midx, midy;
};


int g_numnodes;
node *g_nodes = nullptr;
int g_numedges;
edge *g_edges = nullptr;

GLuint g_nodevao, g_edgevao;
GLuint g_vertbuf[2];	//vertex + index
GLuint g_linebuf;		//index
GLuint g_nodeprog, g_edgeprog;
GLint g_nodeprogaspect, g_edgeprogaspect;
GLint g_nodeprogsize2;

int g_winwidth, g_winheight;
int g_clientwidth, g_clientheight;
int g_fieldwidth, g_fieldheight;
int g_nodesize;

bool g_active;

using namespace std;

string LoadTextFile(const char *file)
{
	ifstream f(file);
	string line, str;
	while (getline(f, line))
		str += line + "\n";
	return str;
}

void InitSettings(const char *cfgfile)
{
	g_winwidth = DEFAULT_WINWIDTH;
	g_winheight = DEFAULT_WINHEIGHT;
	g_clientwidth = DEFAULT_WINWIDTH;
	g_clientheight = DEFAULT_WINHEIGHT;
	g_fieldwidth = DEFAULT_FIELDWIDTH;
	g_fieldheight = DEFAULT_FIELDHEIGHT;
	g_numnodes = DEFAULT_NUMNODES;
	g_nodesize = DEFAULT_NODESIZE;

	char key[64];
	char value[64];
	ifstream f(cfgfile);
	string line;
	while (getline(f, line))
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

		if (!strcmp(key, "numnodes"))
			g_numnodes = atoi(value);
		else if (!strcmp(key, "nodesize"))
			g_nodesize = atoi(value);
		//etc
	}
}

void UpdateSettings()
{
}

void SaveSettings(const char *cfgfile)
{
	ofstream f(cfgfile);
	f << "numnodes = " << g_numnodes << endl;
	f << "nodesize = " << g_nodesize << endl;
}

int CreateShaderProgram(char *vfile, char *pfile)
{
	char infobuf[512];

	string vshstr = LoadTextFile(vfile);
	const char *vshdata = vshstr.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vshdata, nullptr);
	glCompileShader(vs);

	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
		printf("Vertex shader compile failed!\n");
	glGetShaderInfoLog(vs, sizeof(infobuf), nullptr, infobuf);
	printf("%s", infobuf);

	string pshstr = LoadTextFile(pfile);
	const char *pshdata = pshstr.c_str();
	GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ps, 1, &pshdata, nullptr);
	glCompileShader(ps);

	glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
		printf("Pixel shader compile failed!\n");
	glGetShaderInfoLog(ps, sizeof(infobuf), nullptr, infobuf);
	printf("%s", infobuf);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, ps);
	glBindFragDataLocation(prog, 0, "out_color");
	glLinkProgram(prog);

	glDeleteShader(vs);
	glDeleteShader(ps);

	return prog;
}

bool InitGame()
{
	glewExperimental = GL_TRUE;
	glewInit();

	glGenVertexArrays(1, &g_nodevao);
	glGenVertexArrays(1, &g_edgevao);
	glGenBuffers(2, g_vertbuf);
	glGenBuffers(1, &g_linebuf);

	g_nodeprog = CreateShaderProgram("node.vsh", "node.psh");
	g_nodeprogaspect = glGetUniformLocation(g_nodeprog, "aspect");
	g_nodeprogsize2 = glGetUniformLocation(g_nodeprog, "size2");
	glBindVertexArray(g_nodevao);
	glBindBuffer(GL_ARRAY_BUFFER, g_vertbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_vertbuf[1]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);

	g_edgeprog = CreateShaderProgram("edge.vsh", "edge.psh");
	g_edgeprogaspect = glGetUniformLocation(g_edgeprog, "aspect");
	glBindVertexArray(g_edgevao);
	glBindBuffer(GL_ARRAY_BUFFER, g_vertbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_linebuf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2 * sizeof(float)));
	glBindVertexArray(0);

	g_active = false;

	return true;
}

static void NodeQuad(float x, float y, float size, vbovertex *quad)
{
	quad[0].x = x - size;
	quad[0].y = y - size;
	quad[1].x = x + size;
	quad[1].y = y - size;
	quad[2].x = x + size;
	quad[2].y = y + size;
	quad[3].x = x - size;
	quad[3].y = y + size;
	for (int i=0; i<4; i++)
	{
		quad[i].midx = x;
		quad[i].midy = y;
	}
}

void NewGame()
{
	if (g_nodes)
		delete[] g_nodes;
	if (g_edges)
		delete[] g_edges;
	
	//generate graph
	g_numnodes = 30;
	g_nodes = new node[g_numnodes];
	
//	int gridwidth = (int)sqrt(g_numnodes);
		
	for (int i=0; i<g_numnodes; i++)
	{
		g_nodes[i].x = (float)cos((float)i*2.0*M_PI/g_numnodes) * 0.9f;
		g_nodes[i].y = (float)sin((float)i*2.0*M_PI/g_numnodes) * 0.9f;
	}

	g_numedges = g_numnodes;
	g_edges = new edge[g_numedges];
	for (int i=0; i<g_numedges; i++)
	{
		g_edges[i].n1 = i;
		g_edges[i].n2 = (i+500)%g_numnodes;
	}

	//build indices for vertex quads and lines
	vbovertex *vboverts = new vbovertex[g_numnodes*4];
	GLuint *vindices = new GLuint[g_numnodes*6];
	float vertsize = (float)g_nodesize/g_clientheight;
	for (int i=0; i<g_numnodes; i++)
	{
		NodeQuad(g_nodes[i].x, g_nodes[i].y, vertsize, &vboverts[i*4]);
	
		vindices[i*6+0] = i*4+0;
		vindices[i*6+1] = i*4+1;
		vindices[i*6+2] = i*4+2;
		vindices[i*6+3] = i*4+0;
		vindices[i*6+4] = i*4+2;
		vindices[i*6+5] = i*4+3;
	}

	GLuint *lindices = new GLuint[g_numedges*2];
	for (int i=0; i<g_numedges; i++)
	{
		lindices[i*2+0] = g_edges[i].n1*4;
		lindices[i*2+1] = g_edges[i].n2*4;
	}

	//upload data to gpu
	glBindBuffer(GL_ARRAY_BUFFER, g_vertbuf[0]);
	glBufferData(GL_ARRAY_BUFFER, 4*g_numnodes*sizeof(vbovertex), vboverts, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_vertbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*g_numnodes*sizeof(GLuint), vindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_linebuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*g_numedges*sizeof(GLuint), lindices, GL_STATIC_DRAW);
	delete[] lindices;
	delete[] vindices;
	delete[] vboverts;

	g_active = true;
}

void SaveGame(const char *gamefile)
{
}

void LoadGame(const char *gamefile)
{
}

void RenderGame()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (g_active)
	{
		//render edges
		glUseProgram(g_edgeprog);
		glUniform1f(g_edgeprogaspect, (float)g_clientwidth/g_clientheight);
		glBindVertexArray(g_edgevao);
		glDrawElements(GL_LINES, g_numedges*2, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		
		//render nodes
		glUseProgram(g_nodeprog);
		glUniform1f(g_nodeprogaspect, (float)g_clientwidth/g_clientheight);
		glUniform1f(g_nodeprogsize2, SQR((float)g_nodesize/g_clientheight));
		glBindVertexArray(g_nodevao);
		glDrawElements(GL_TRIANGLES, g_numnodes*6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

int FindNode(float x, float y)
{
	//TODO: speed up
	float nodesize2 = SQR((float)g_nodesize/g_clientheight);
	for (int i=0; i<g_numnodes; i++)
	{
		node *n = &g_nodes[i];
		if (SQR(n->x - x) + SQR(n->y - y) < nodesize2)
			return i;
	}
	return -1;
}

void SetNode(int n, float x, float y)
{
	vbovertex nodequad[4];
	NodeQuad(x, y, (float)g_nodesize/g_clientheight, nodequad);
		
	glBindBuffer(GL_ARRAY_BUFFER, g_vertbuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, n*4*sizeof(vbovertex), 4*sizeof(vbovertex), &nodequad);
	
	g_nodes[n].x = x;
	g_nodes[n].y = y;
}
