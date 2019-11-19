/*
  Name:         glcWavefrontObject.h
  Author:       Rodrigo Luis de Souza da Silva
  Date:         06/12/2017
                View glcWavefrontObject_Updates.txt to see the updates of this class.
  Description:  GL class to help dealing with wavefront objects. It's a simplified version
                of the well-known glm file reader developed by Nate Robins.
*/

#ifndef _GLC_WAVEFRONT_OBJECT_H
#define _GLC_WAVEFRONT_OBJECT_H

#include <math.h>
#include <stdio.h>
#include <GL/gl.h>

using namespace std;

#define T(x) (model->triangles[(x)])

#define FLAT_SHADING    0       /* render with facet normals */
#define SMOOTH_SHADING  1       /* render with vertex normals */

#define USE_COLOR                 0       /* render with colors */
#define USE_MATERIAL              1       /* render with materials */
#define USE_TEXTURE               2       /* render with texture coords */
#define USE_TEXTURE_AND_MATERIAL  3       /* render with texture coords and material (if available) */

typedef struct _GLMaterial  GLMmaterial;
typedef struct _GLMtriangle GLMtriangle;
typedef struct _GLMgroup    GLMgroup;
typedef struct _GLMmodel    GLMmodel;
typedef struct _GLMnode     GLMnode;

class glcWavefrontObject
{
    public:
        glcWavefrontObject();
        ~glcWavefrontObject();
        void  SetNumberOfObjects(int );
        int   GetNumberOfObjects();
        void  SelectObject(int);
        void  SetShadingMode(int);  // Possible values: FLAT_SHADING e SMOOTH_SHADING
        void  SetRenderMode(int);   // Possible values: USE_COLOR, USE_MATERIAL, USE_TEXTURE
        void  SetColor(float, float, float, float a = 1.0);
        void Draw();
        void DrawBoundingBox();
        void ReadObject(const char *filename);
        int GetNumberOfVertices();
        int GetNumberOfTriangles();

        void GetDimensions(GLfloat* dimensions);

        float Unitize();
        void FacetNormal();
        void VertexNormals(GLfloat angle);
        void Scale(GLfloat scale);

    private:
        GLMgroup* FindGroup(char* name);
        GLMgroup* AddGroup(char* name);
        GLuint FindMaterial(char* name);

        float Dot(GLfloat* u, GLfloat* v);
        float Max(GLfloat a, GLfloat b);
        float Abs(GLfloat f);
        void  Cross(GLfloat* u, GLfloat* v, GLfloat* n);

        void FirstPass(FILE *file);
        void SecondPass(FILE *file);
        char* DirName(char* path);
        void  ReadMTL(char* name);
        void  Normalize(GLfloat* v);
        void  ComputeBoundingBox();

        // Attributes
        int shading;    // shading mode
        int render;     // rendering mode
        float color[4];

        GLMmodel* modelList;
        GLMmodel* model; // Current Object
        int numberOfObjects;
};

// Auxiliary structures
struct _GLMaterial
{
    char* name;                   /* name of material */
    GLfloat diffuse[4];           /* diffuse component */
    GLfloat ambient[4];           /* ambient component */
    GLfloat specular[4];          /* specular component */
    GLfloat emmissive[4];         /* emmissive component */
    GLfloat shininess;            /* specular exponent */
};

// GLMtriangle: Structure that defines a triangle in a model.
struct _GLMtriangle
{
    GLuint vindices[3];           /* array of triangle vertex indices */
    GLuint nindices[3];           /* array of triangle normal indices */
    GLuint tindices[3];           /* array of triangle texcoord indices*/
    GLuint findex;                /* index of triangle facet normal */
};

// GLMgroup: Structure that defines a group in a model.
struct _GLMgroup
{
    char*             name;           /* name of this group */
    GLuint            numtriangles;   /* number of triangles in this group */
    GLuint*           triangles;      /* array of triangle indices */
    GLuint            material;       /* index to material for group */
    struct _GLMgroup* next;           /* pointer to next group in model */
};

// GLMmodel: Structure that defines a model.
struct _GLMmodel
{
    char*    pathname;            /* path to this model */
    char*    mtllibname;          /* name of the material library */

    GLuint   numvertices;         /* number of vertices in model */
    GLfloat* vertices;            /* array of vertices  */

    GLuint   numnormals;          /* number of normals in model */
    GLfloat* normals;             /* array of normals */

    GLuint   numtexcoords;        /* number of texcoords in model */
    GLfloat* texcoords;           /* array of texture coordinates */

    GLuint   numfacetnorms;       /* number of facetnorms in model */
    GLfloat* facetnorms;          /* array of facetnorms */

    GLuint       numtriangles;    /* number of triangles in model */
    GLMtriangle* triangles;       /* array of triangles */

    GLuint       nummaterials;    /* number of materials in model */
    GLMmaterial* materials;       /* array of materials */

    GLuint       numgroups;       /* number of groups in model */
    GLMgroup*    groups;          /* linked list of groups */

    GLfloat position[3];          /* position of the model */
    GLfloat boundingbox[6];       /* [minx, miny, minz, maxx, maxy, maxz] */
};

struct _GLMnode
{
    GLuint         index;
    GLboolean      averaged;
    struct _GLMnode* next;
};

#endif


