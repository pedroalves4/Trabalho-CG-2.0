#include "glcWavefrontObject.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-result"

glcWavefrontObject::glcWavefrontObject()
{
    this->model = NULL;
    this->modelList = NULL;
    this->numberOfObjects = 1;

    this->shading = FLAT_SHADING;
    this->render  = USE_COLOR;

    // Default color is set to orange
    this->color[0] = 1.0;
    this->color[1] = 0.5;
    this->color[2] = 0.0;
    this->color[3] = 1.0;
}

glcWavefrontObject::~glcWavefrontObject()
{
    if(this->model && this->numberOfObjects == 1)
        delete this->model;
    else if(this->model && this->numberOfObjects > 1)
        delete [] this->model;
}

//-----------------------------------------------------------
void glcWavefrontObject::SetNumberOfObjects(int n)
{
    this->numberOfObjects = n;

    this->modelList = new GLMmodel[n];
}

//-----------------------------------------------------------
int glcWavefrontObject::GetNumberOfObjects()
{
    return this->numberOfObjects;
}

//-----------------------------------------------------------
void  glcWavefrontObject::SelectObject(int obj)
{
    this->model = &modelList[obj];
}

//-----------------------------------------------------------
int glcWavefrontObject::GetNumberOfVertices()
{
    return this->model->numvertices;
}

//-----------------------------------------------------------
int glcWavefrontObject::GetNumberOfTriangles()
{
    return this->model->numtriangles;
}

//-----------------------------------------------------------
// Possible values: FLAT_SHADING e SMOOTH_SHADING
void glcWavefrontObject::SetShadingMode(int s)
{
    if(s!=FLAT_SHADING && s!=SMOOTH_SHADING)
    {
        printf("glcWavefrontObject::SetShadingMode(...) warning: Invalid flat shading mode requested."
               "Using FLAT_SHADING\n");
        this->shading = FLAT_SHADING;
        return;
    }
    this->shading = s;
}

//-----------------------------------------------------------
// Possible values: USE_COLOR, USE_MATERIAL, USE_TEXTURE
void glcWavefrontObject::SetRenderMode(int s)
{
    if(s!=USE_COLOR && s!=USE_MATERIAL && s!=USE_TEXTURE && s!=USE_TEXTURE_AND_MATERIAL)
    {
        printf("glcWavefrontObject::SetRenderMode(...) warning: Invalid render mode requested."
               "Using COLOR\n");
        this->render = USE_COLOR;
        return;
    }
    this->render = s;
}

//-----------------------------------------------------------
void  glcWavefrontObject::SetColor(float r, float g, float b, float a)
{
    this->color[0] = r;
    this->color[1] = g;
    this->color[2] = b;
    this->color[3] = a;
}

//-----------------------------------------------------------
void glcWavefrontObject::ReadObject(const char *filename)
{
    FILE*   file;

    /* open the file */
    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n",
                filename);
        exit(1);
    }

    /* allocate a new model */

    if(numberOfObjects == 1)
        this->model = (GLMmodel*) malloc(sizeof(GLMmodel));

    this->model->pathname     = strdup(filename);
    model->mtllibname   = NULL;
    model->numvertices  = 0;
    model->vertices     = NULL;
    model->numnormals   = 0;
    model->normals      = NULL;
    model->numtexcoords = 0;
    model->texcoords    = NULL;
    model->numfacetnorms = 0;
    model->facetnorms    = NULL;
    model->numtriangles  = 0;
    model->triangles       = NULL;
    model->nummaterials  = 0;
    model->materials       = NULL;
    model->numgroups       = 0;
    model->groups      = NULL;
    model->position[0]   = 0.0;
    model->position[1]   = 0.0;
    model->position[2]   = 0.0;

    /* make a first pass through the file to get a count of the number
    of vertices, normals, texcoords & triangles */
    FirstPass(file);

    /* allocate memory */
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numvertices + 1));
    model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) * model->numtriangles);
    if (model->numnormals)
    {
        model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numnormals + 1));
    }
    if (model->numtexcoords)
    {
        model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (model->numtexcoords + 1));
    }

    /* rewind to beginning of file and read in the data this pass */
    rewind(file);

    SecondPass(file);

    /* close the file */
    fclose(file);

    /* compute the bounding box */
    ComputeBoundingBox();
}

//-----------------------------------------------------------
float glcWavefrontObject::Unitize()
{
    GLuint  i;
    GLfloat *maxx, *minx, *maxy, *miny, *maxz, *minz;
    GLfloat cx, cy, cz, w, h, d;
    GLfloat scale;

    assert(model);
    assert(model->vertices);

    /* recover bounding box from model */
    minx = &model->boundingbox[0];
    miny = &model->boundingbox[1];
    minz = &model->boundingbox[2];
    maxx = &model->boundingbox[3];
    maxy = &model->boundingbox[4];
    maxz = &model->boundingbox[5];

    /* calculate model width, height, and depth */
    w = Abs(*maxx - *minx);
    h = Abs(*maxy - *miny);
    d = Abs(*maxz - *minz);

    /* calculate center of the model */
    cx = (*maxx + *minx) / 2.0;
    cy = (*maxy + *miny) / 2.0;
    cz = (*maxz + *minz) / 2.0;

    /* calculate unitizing scale factor */
    scale = 0.2 / Max(Max(w, h), d);

    /* translate around center then scale */
    for (i = 1; i <= model->numvertices; i++)
    {
        model->vertices[3 * i + 0] -= cx;
        model->vertices[3 * i + 1] -= cy;
        model->vertices[3 * i + 2] -= cz;
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }

    /* correct bounding box */
    (*minx) = scale * (*minx-cx);
    (*miny) = scale * (*miny-cy);
    (*minz) = scale * (*minz-cz);
    (*maxx) = scale * (*maxx-cx);
    (*maxy) = scale * (*maxy-cy);
    (*maxz) = scale * (*maxz-cz);

    return scale;
}

//-----------------------------------------------------------
void glcWavefrontObject::GetDimensions(GLfloat* dimensions)
{
    GLfloat maxx, minx, maxy, miny, maxz, minz;

    assert(model);
    assert(model->vertices);
    assert(dimensions);

    /* recover bounding box from model */
    minx = model->boundingbox[0];
    maxx = model->boundingbox[3];
    miny = model->boundingbox[1];
    maxy = model->boundingbox[4];
    minz = model->boundingbox[2];
    maxz = model->boundingbox[5];

    /* calculate model width, height, and depth */
    dimensions[0] = Abs(maxx - minx);
    dimensions[1] = Abs(maxy - miny);
    dimensions[2] = Abs(maxz - minz);
}

//-----------------------------------------------------------
void glcWavefrontObject::Scale(GLfloat scale)
{
    GLuint i;

    for (i = 1; i <= model->numvertices; i++)
    {
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }

    for(i = 0; i < 6; i++)
        model->boundingbox[i] *= scale;
}

//-----------------------------------------------------------
void glcWavefrontObject::ComputeBoundingBox()
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;

    assert(model);
    assert(model->vertices);

    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++)
    {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];

        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];

        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }

    model->boundingbox[0] = minx;
    model->boundingbox[3] = maxx;
    model->boundingbox[1] = miny;
    model->boundingbox[4] = maxy;
    model->boundingbox[2] = minz;
    model->boundingbox[5] = maxz;
}

//-----------------------------------------------------------
void glcWavefrontObject::FacetNormal()
{
    GLuint  i;
    GLfloat u[3];
    GLfloat v[3];

    assert(model);
    assert(model->vertices);

    /* clobber any old facetnormals */
    if (model->facetnorms)
        free(model->facetnorms);

    /* allocate memory for the new facet normals */
    model->numfacetnorms = model->numtriangles;
    model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numfacetnorms + 1));

    for (i = 0; i < model->numtriangles; i++)
    {
        model->triangles[i].findex = i+1;

        u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
               model->vertices[3 * T(i).vindices[0] + 0];
        u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
               model->vertices[3 * T(i).vindices[0] + 1];
        u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
               model->vertices[3 * T(i).vindices[0] + 2];

        v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
               model->vertices[3 * T(i).vindices[0] + 0];
        v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
               model->vertices[3 * T(i).vindices[0] + 1];
        v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
               model->vertices[3 * T(i).vindices[0] + 2];

        Cross(u, v, &model->facetnorms[3 * (i+1)]);
        Normalize(&model->facetnorms[3 * (i+1)]);
    }
}

//-----------------------------------------------------------
void glcWavefrontObject::VertexNormals(GLfloat angle)
{
    GLMnode*    node;
    GLMnode*    tail;
    GLMnode** members;
    GLfloat*    normals;
    GLuint  numnormals;
    GLfloat average[3];
    GLfloat dot, cos_angle;
    GLuint  i, avg;

    assert(model);
    assert(model->facetnorms);

    /* calculate the cosine of the angle (in degrees) */
    cos_angle = cos(angle * M_PI / 180.0);

    /* nuke any previous normals */
    if (model->normals)
        free(model->normals);

    /* allocate space for new normals */
    model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));

    /* allocate a structure that will hold a linked list of triangle
    indices for each vertex */
    members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
    for (i = 1; i <= model->numvertices; i++)
        members[i] = NULL;

    /* for every triangle, create a node for each vertex in it */
    for (i = 0; i < model->numtriangles; i++)
    {
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[0]];
        members[T(i).vindices[0]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[1]];
        members[T(i).vindices[1]] = node;

        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[2]];
        members[T(i).vindices[2]] = node;
    }

    /* calculate the average normal for each vertex */
    numnormals = 1;
    for (i = 1; i <= model->numvertices; i++)
    {
        /* calculate an average normal for this vertex by averaging the
            facet normal of every triangle this vertex is in */
        node = members[i];
        if (!node)
            fprintf(stderr, "glcWavefrontObject::VertexNormals(): vertex w/o a triangle\n");
        average[0] = 0.0;
        average[1] = 0.0;
        average[2] = 0.0;
        avg = 0;
        while (node)
        {
            /* only average if the dot product of the angle between the two
            facet normals is greater than the cosine of the threshold
            angle -- or, said another way, the angle between the two
                facet normals is less than (or equal to) the threshold angle */
            dot = Dot(&model->facetnorms[3 * T(node->index).findex],
                      &model->facetnorms[3 * T(members[i]->index).findex]);
            if (dot > cos_angle)
            {
                node->averaged = GL_TRUE;
                average[0] += model->facetnorms[3 * T(node->index).findex + 0];
                average[1] += model->facetnorms[3 * T(node->index).findex + 1];
                average[2] += model->facetnorms[3 * T(node->index).findex + 2];
                avg = 1;            /* we averaged at least one normal! */
            }
            else
            {
                node->averaged = GL_FALSE;
            }
            node = node->next;
        }

        if (avg)
        {
            /* normalize the averaged normal */
            Normalize(average);

            /* add the normal to the vertex normals list */
            model->normals[3 * numnormals + 0] = average[0];
            model->normals[3 * numnormals + 1] = average[1];
            model->normals[3 * numnormals + 2] = average[2];
            avg = numnormals;
            numnormals++;
        }

        /* set the normal of this vertex in each triangle it is in */
        node = members[i];
        while (node)
        {
            if (node->averaged)
            {
                /* if this node was averaged, use the average normal */
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = avg;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = avg;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = avg;
            }
            else
            {
                /* if this node wasn't averaged, use the facet normal */
                model->normals[3 * numnormals + 0] =
                    model->facetnorms[3 * T(node->index).findex + 0];
                model->normals[3 * numnormals + 1] =
                    model->facetnorms[3 * T(node->index).findex + 1];
                model->normals[3 * numnormals + 2] =
                    model->facetnorms[3 * T(node->index).findex + 2];
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = numnormals;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = numnormals;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = numnormals;
                numnormals++;
            }
            node = node->next;
        }
    }

    model->numnormals = numnormals - 1;

    /* free the member information */
    for (i = 1; i <= model->numvertices; i++)
    {
        node = members[i];
        while (node)
        {
            tail = node;
            node = node->next;
            free(tail);
        }
    }
    free(members);

    /* pack the normals array (we previously allocated the maximum
    number of normals that could possibly be created (numtriangles *
    3), so get rid of some of them (usually alot unless none of the
    facet normals were averaged)) */
    normals = model->normals;
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
    for (i = 1; i <= model->numnormals; i++)
    {
        model->normals[3 * i + 0] = normals[3 * i + 0];
        model->normals[3 * i + 1] = normals[3 * i + 1];
        model->normals[3 * i + 2] = normals[3 * i + 2];
    }
    free(normals);
}

//-----------------------------------------------------------
void glcWavefrontObject::Normalize(GLfloat* v)
{
    GLfloat l;

    assert(v);

    l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

//-----------------------------------------------------------
float glcWavefrontObject::Dot(GLfloat* u, GLfloat* v)
{
    assert(u);
    assert(v);

    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

//-----------------------------------------------------------
void glcWavefrontObject::Cross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    assert(u);
    assert(v);
    assert(n);

    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];
}

//-----------------------------------------------------------
float glcWavefrontObject::Max(GLfloat a, GLfloat b)
{
    if (b > a)
        return b;
    return a;
}

//-----------------------------------------------------------
float glcWavefrontObject::Abs(GLfloat f)
{
    if (f < 0)
        return -f;
    return f;
}

//-----------------------------------------------------------
void glcWavefrontObject::DrawBoundingBox()
{
    float pmin[3] = {model->boundingbox[0], model->boundingbox[1], model->boundingbox[2]};
    float pmax[3] = {model->boundingbox[3], model->boundingbox[4], model->boundingbox[5]};

    bool lightWasEnabled   = glIsEnabled(GL_LIGHTING);
    bool textureWasEnabled = glIsEnabled(GL_TEXTURE_2D);

    if(lightWasEnabled)
        glDisable(GL_LIGHTING);   // Disabling Lighting
    if(textureWasEnabled)
        glDisable(GL_TEXTURE_2D); // Disabling texture

    // Draw Bounding Box
    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_LINE_STRIP);
    glVertex3f(pmin[0],pmin[1],pmin[2]);
    glVertex3f(pmax[0],pmin[1],pmin[2]);
    glVertex3f(pmax[0],pmin[1],pmax[2]);
    glVertex3f(pmin[0],pmin[1],pmax[2]); // Base

    glVertex3f(pmin[0],pmin[1],pmin[2]);
    glVertex3f(pmin[0],pmax[1],pmin[2]);
    glVertex3f(pmin[0],pmax[1],pmax[2]);
    glVertex3f(pmin[0],pmin[1],pmax[2]); // Left

    glVertex3f(pmax[0],pmin[1],pmax[2]);
    glVertex3f(pmax[0],pmax[1],pmax[2]);
    glVertex3f(pmin[0],pmax[1],pmax[2]); // Back
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(pmin[0],pmax[1],pmin[2]);
    glVertex3f(pmax[0],pmax[1],pmin[2]);
    glVertex3f(pmax[0],pmax[1],pmax[2]); // Top
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(pmax[0],pmin[1],pmin[2]);
    glVertex3f(pmax[0],pmax[1],pmin[2]); // Last edge to close front and right
    glEnd();

    if(lightWasEnabled)
        glEnable(GL_LIGHTING);   // Enable Lighting, if applicable
    if(textureWasEnabled)
        glEnable(GL_TEXTURE_2D); // Disabling texture
}

//-----------------------------------------------------------
void glcWavefrontObject::Draw()
{
    static GLuint i,j;
    static GLMgroup* group;
    static GLMtriangle* triangle;
    static GLMmaterial* material;

    assert(model);
    assert(model->vertices);

    if (this->shading == FLAT_SHADING && !model->facetnorms)
    {
        printf("glcWavefrontObject::Draw() warning: flat shading mode requested with no facet normals defined.\nChanging to SMOOTH_SHADING!\n");
        this->shading = SMOOTH_SHADING;
    }
    if (this->shading == SMOOTH_SHADING && !model->normals)
    {
        printf("glcWavefrontObject::Draw() warning: smooth shading mode requested with no normals defined. Changing to FLAT_SHADING!\n");
        this->shading = FLAT_SHADING;
    }
    if ( (this->render == USE_TEXTURE || this->render == USE_TEXTURE_AND_MATERIAL) && !model->texcoords)
    {
        printf("glcWavefrontObject::Draw() warning: texture render mode requested with no texture coordinates defined.\nChanging to Color Rendering.\n");
        this->render = USE_COLOR;
    }
    if (this->render == USE_MATERIAL && !model->materials)
    {
        printf("glcWavefrontObject::Draw() warning: material render mode requested with no materials defined.\nChanging to Color Rendering.\n");
        this->render = USE_COLOR;
    }

    if (this->render == USE_COLOR)
        glEnable(GL_COLOR_MATERIAL);
    else if (this->render == USE_MATERIAL)
        glDisable(GL_COLOR_MATERIAL);

    group = model->groups;
    while (group)
    {
        if (this->render == USE_MATERIAL || this->render == USE_TEXTURE_AND_MATERIAL)
        {
            if(model->materials) // check if USE_TEXTURE_AND_MATERIAL is selected (some models may have only texture)
            {
                material = &model->materials[group->material];
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
            }
        }

        if (this->render == USE_COLOR)
        {
            glColor4fv(color);
        }

        glBegin(GL_TRIANGLES);
        for (i = 0; i < group->numtriangles; i++)
        {
            triangle = &T(group->triangles[i]);

            for (j = 0; j < 3; j++)
            {
                //if (mode & FLAT_SHADING)
                if(this->shading == FLAT_SHADING)
                    glNormal3fv(&model->facetnorms[3 * triangle->findex]);

                //if (mode & SMOOTH_SHADING)
                if(this->shading == SMOOTH_SHADING)
                    glNormal3fv(&model->normals[3 * triangle->nindices[j]]);
                if (this->render == USE_TEXTURE || this->render == USE_TEXTURE_AND_MATERIAL )
                    glTexCoord2fv(&model->texcoords[2 * triangle->tindices[j]]);
                glVertex3fv(&model->vertices[3 * triangle->vindices[j]]);
            }
        }
        glEnd();

        group = group->next;
    }
}

//-----------------------------------------------------------------------------
// PRIVATE METHODS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------
// Add a group to the model
GLMgroup* glcWavefrontObject::AddGroup(char* name)
{
    GLMgroup* group;

    group = FindGroup(name);
    if (!group)
    {
        group = (GLMgroup*)malloc(sizeof(GLMgroup));
        group->name = strdup(name);
        group->material = 0;
        group->numtriangles = 0;
        group->triangles = NULL;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }

    return group;
}

//-----------------------------------------------------------
/* glmFindGroup: Find a material in the model */
GLuint glcWavefrontObject::FindMaterial(char* name)
{
    GLuint i;

    /* XXX doing a linear search on a string key'd list is pretty lame,
    but it works and is fast enough for now. */
    for (i = 0; i < model->nummaterials; i++)
    {
        if (!strcmp(model->materials[i].name, name))
            goto found;
    }

    /* didn't find the name, so print a warning and return the default material (0). */
    printf("glcWavefrontObject::FindMaterial():  can't find material \"%s\".\n", name);
    i = 0;

found:
    return i;
}

//-----------------------------------------------------------
void glcWavefrontObject::FirstPass(FILE *file)
{
    GLuint  numvertices;        /* number of vertices in model */
    GLuint  numnormals;         /* number of normals in model */
    GLuint  numtexcoords;       /* number of texcoords in model */
    GLuint  numtriangles;       /* number of triangles in model */
    GLMgroup* group;            /* current group */
    unsigned    v, n, t;
    char        buf[128];

    /* make a default group */
    group = AddGroup((char *) "default");

    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case '#':               /* comment */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'v':               /* v, vn, vt */
                switch(buf[1])
                {
                    case '\0':          /* vertex */
                        /* eat up rest of line */
                        fgets(buf, sizeof(buf), file);
                        numvertices++;
                        break;
                    case 'n':           /* normal */
                        /* eat up rest of line */
                        fgets(buf, sizeof(buf), file);
                        numnormals++;
                        break;
                    case 't':           /* texcoord */
                        /* eat up rest of line */
                        fgets(buf, sizeof(buf), file);
                        numtexcoords++;
                        break;
                    default:
                        printf("glcWavefrontObject::FirstPass(): Unknown token \"%s\".\n", buf);
                        exit(1);
                        break;
                }
                break;
            case 'm':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                model->mtllibname = strdup(buf);
                ReadMTL(buf);
                break;
            case 'u':
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = AddGroup(buf);
                break;
            case 'f':               /* face */
                v = n = t = 0;
                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//"))
                {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d//%d", &v, &n) > 0)
                    {
                        numtriangles++;
                        group->numtriangles++;
                    }
                }
                else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
                {
                    /* v/t/n */
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0)
                    {
                        numtriangles++;
                        group->numtriangles++;
                    }
                }
                else if (sscanf(buf, "%d/%d", &v, &t) == 2)
                {
                    /* v/t */
                    fscanf(file, "%d/%d", &v, &t);
                    fscanf(file, "%d/%d", &v, &t);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d/%d", &v, &t) > 0)
                    {
                        numtriangles++;
                        group->numtriangles++;
                    }
                }
                else
                {
                    /* v */
                    fscanf(file, "%d", &v);
                    fscanf(file, "%d", &v);
                    numtriangles++;
                    group->numtriangles++;
                    while(fscanf(file, "%d", &v) > 0)
                    {
                        numtriangles++;
                        group->numtriangles++;
                    }
                }
                break;

            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
    }

    /* set the stats in the model structure */
    model->numvertices  = numvertices;
    model->numnormals   = numnormals;
    model->numtexcoords = numtexcoords;
    model->numtriangles = numtriangles;

    /* allocate memory for the triangles in each group */
    group = model->groups;
    while(group)
    {
        group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
        group->numtriangles = 0;
        group = group->next;
    }
}

//-----------------------------------------------------------
void glcWavefrontObject::SecondPass(FILE* file)
{
    GLuint  numvertices;        /* number of vertices in model */
    GLuint  numnormals;         /* number of normals in model */
    GLuint  numtexcoords;       /* number of texcoords in model */
    GLuint  numtriangles;       /* number of triangles in model */
    GLfloat*    vertices;           /* array of vertices  */
    GLfloat*    normals;            /* array of normals */
    GLfloat*    texcoords;          /* array of texture coordinates */
    GLMgroup* group;            /* current group pointer */
    GLuint  material;           /* current material */
    GLuint  v, n, t;
    char        buf[128];

    /* set the pointer shortcuts */
    vertices   = model->vertices;
    normals    = model->normals;
    texcoords  = model->texcoords;
    group      = model->groups;

    /* on the second pass through the file, read all the data into the
    allocated arrays */
    numvertices = numnormals = numtexcoords = 1;
    numtriangles = 0;
    material = 0;
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case '#':               /* comment */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'v':               /* v, vn, vt */
                switch(buf[1])
                {
                    case '\0':          /* vertex */
                        fscanf(file, "%f %f %f",
                               &vertices[3 * numvertices + 0],
                               &vertices[3 * numvertices + 1],
                               &vertices[3 * numvertices + 2]);
                        numvertices++;
                        break;
                    case 'n':           /* normal */
                        fscanf(file, "%f %f %f",
                               &normals[3 * numnormals + 0],
                               &normals[3 * numnormals + 1],
                               &normals[3 * numnormals + 2]);
                        numnormals++;
                        break;
                    case 't':           /* texcoord */
                        fscanf(file, "%f %f",
                               &texcoords[2 * numtexcoords + 0],
                               &texcoords[2 * numtexcoords + 1]);
                        numtexcoords++;
                        break;
                }
                break;
            case 'u':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                group->material = material = FindMaterial(buf);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = FindGroup(buf);
                group->material = material;
                break;
            case 'f':               /* face */
                v = n = t = 0;
                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//"))
                {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).nindices[0] = n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).nindices[1] = n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d//%d", &v, &n) > 0)
                    {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).nindices[2] = n;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }
                }
                else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
                {
                    /* v/t/n */
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).tindices[0] = t;
                    T(numtriangles).nindices[0] = n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).tindices[1] = t;
                    T(numtriangles).nindices[1] = n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0)
                    {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).tindices[2] = t;
                        T(numtriangles).nindices[2] = n;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }
                }
                else if (sscanf(buf, "%d/%d", &v, &t) == 2)
                {
                    /* v/t */
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).tindices[0] = t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).tindices[1] = t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d/%d", &v, &t) > 0)
                    {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).tindices[2] = t;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }
                }
                else
                {
                    /* v */
                    sscanf(buf, "%d", &v);
                    T(numtriangles).vindices[0] = v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[1] = v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[2] = v;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d", &v) > 0)
                    {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).vindices[2] = v;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }
                }
                break;

            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
    }
}

//-----------------------------------------------------------
char* glcWavefrontObject::DirName(char* path)
{
    char* dir;
    char* s;

    dir = strdup(path);

    s = strrchr(dir, '/');
    if (s)
        s[1] = '\0';
    else
        dir[0] = '\0';

    return dir;
}

//-----------------------------------------------------------
void glcWavefrontObject::ReadMTL(char* name)
{
    FILE* file;
    char* dir;
    char* filename;
    char  buf[128];
    GLuint nummaterials, i;

    dir = DirName(model->pathname);
    filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
    strcpy(filename, dir);
    strcat(filename, name);
    free(dir);

    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "glcWavefrontObject::ReadMTL() failed: can't open material file \"%s\".\n", filename);
        exit(1);
    }
    free(filename);

    /* count the number of materials in the file */
    nummaterials = 1;
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case '#':               /* comment */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'n':               /* newmtl */
                fgets(buf, sizeof(buf), file);
                nummaterials++;
                sscanf(buf, "%s %s", buf, buf);
                break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
    }

    rewind(file);

    model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
    model->nummaterials = nummaterials;

    /* set the default material */
    for (i = 0; i < nummaterials; i++)
    {
        model->materials[i].name = NULL;
        model->materials[i].shininess = 65.0;
        model->materials[i].diffuse[0] = 0.8;
        model->materials[i].diffuse[1] = 0.8;
        model->materials[i].diffuse[2] = 0.8;
        model->materials[i].diffuse[3] = 1.0;
        model->materials[i].ambient[0] = 0.2;
        model->materials[i].ambient[1] = 0.2;
        model->materials[i].ambient[2] = 0.2;
        model->materials[i].ambient[3] = 1.0;
        model->materials[i].specular[0] = 0.0;
        model->materials[i].specular[1] = 0.0;
        model->materials[i].specular[2] = 0.0;
        model->materials[i].specular[3] = 1.0;
    }
    model->materials[0].name = strdup("default");

    /* now, read in the data */
    nummaterials = 0;
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case '#':               /* comment */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'n':               /* newmtl */
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                nummaterials++;
                model->materials[nummaterials].name = strdup(buf);
                break;
            case 'N':
                fscanf(file, "%f", &model->materials[nummaterials].shininess);
                /* wavefront shininess is from [0, 1000], so scale for OpenGL */
                model->materials[nummaterials].shininess /= 1000.0;
                model->materials[nummaterials].shininess *= 128.0;
                break;
            case 'K':
                switch(buf[1])
                {
                    case 'd':
                        fscanf(file, "%f %f %f",
                               &model->materials[nummaterials].diffuse[0],
                               &model->materials[nummaterials].diffuse[1],
                               &model->materials[nummaterials].diffuse[2]);
                        break;
                    case 's':
                        fscanf(file, "%f %f %f",
                               &model->materials[nummaterials].specular[0],
                               &model->materials[nummaterials].specular[1],
                               &model->materials[nummaterials].specular[2]);
                        break;
                    case 'a':
                        fscanf(file, "%f %f %f",
                               &model->materials[nummaterials].ambient[0],
                               &model->materials[nummaterials].ambient[1],
                               &model->materials[nummaterials].ambient[2]);
                        break;
                    default:
                        /* eat up rest of line */
                        fgets(buf, sizeof(buf), file);
                        break;
                }
                break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
    }
}

//-----------------------------------------------------------
// Find a group in the model
GLMgroup* glcWavefrontObject::FindGroup(char* name)
{
    GLMgroup* group;

    assert(model);

    group = model->groups;
    while(group)
    {
        if (!strcmp(name, group->name))
            break;
        group = group->next;
    }

    return group;
}

#pragma GCC diagnostic pop
