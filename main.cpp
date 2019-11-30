#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <GL/glext.h>
#include <cstdlib>
#include <time.h> /* time */
#include "shared/glcTexture.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "shared/glcWavefrontObject.h"
//#include <GLFW/glfw3.h>

#include "extras.h"
#define NUM_OBJECTS 7

/// Estruturas iniciais para armazenar vertices
//  Voc� poder� utiliz�-las adicionando novos m�todos (de acesso por exemplo) ou usar suas pr�prias estruturas.

using namespace std;

class vertice
{
public:
    float x, y, z;
};

class triangle
{
public:
    vertice v[3];
};

class vetor
{
public:
    vertice v1;
};

class barrinhas
{
public:
    bool mostra;

    bool getExibe()
    {
        return mostra;
    }
};

typedef struct
{
    float x, y;
} vertex;

typedef struct
{
    //GLMmodel* pmodel = NULL;
    glcWavefrontObject *pmodel = NULL;
} object;

/// Globals
glcTexture *textureManager;
float zdist = 3.0;
float rotationX = 0.0, rotationY = 0.0;
float rotacaoX = 0.0, rotacaoY = 0.0;
int last_x, last_y;
int width, height;
int vidasRestantes = 5;
bool pausado = false;
bool projecao_ortogonal = true;
bool rotacaoLiberada = false;
bool podeMoverABolinha = false;
bool primeiroLancamento = false;
int janela = 0;
int fase = 1;
bool efeitoEsq = false;
bool efeitoDir = false;
float slideXEsq = 0.0;
float slideXDir = 0.0;
bool initialState = true;
float xBarra = 0.0;
float xBolinha = 0.0;
float yBolinhaInicial = -0.45;
float yBolinha = yBolinhaInicial;
float xSeta = 0.40;
float ySeta = 0.0;
float PI = 3.1415927;
float raioTorus = 0.05;
bool desenhaSetaControle = true;
bool pintaPlataformaVermelho = false;
float caiObjEsq = 0;
float caiObjDir = 0;
int contObj = 0;
bool objEsquerda = false;
bool objDireita = false;
vetor *vetorSeta = new vetor();
barrinhas *vetorBloquinhos = new barrinhas[15];
vetor vetorMovimentoBolinha;
int ultimaColisao = 999;


char objectFiles[NUM_OBJECTS][50] =
{
    "../data/obj/al.obj",
    "../data/obj/dolphins.obj",
    "../data/obj/f-16.obj",
    "../data/obj/flowers.obj",
    "../data/obj/porsche.obj",
    "../data/obj/rose+vase.obj",
    "../data/obj/soccerball.obj"
    //"../data/obj/12190_Heart_v1_L3.obj"
};

object *objectList;

glcWavefrontObject *objectManager = NULL;


/// Functions

void CalculaNormal(triangle t, vertice *vn)
{
    vertice v_0 = t.v[0],
            v_1 = t.v[1],
            v_2 = t.v[2];
    vertice v1, v2;
    double len;

    /* Encontra vetor v1 */
    v1.x = v_1.x - v_0.x;
    v1.y = v_1.y - v_0.y;
    v1.z = v_1.z - v_0.z;

    /* Encontra vetor v2 */
    v2.x = v_2.x - v_0.x;
    v2.y = v_2.y - v_0.y;
    v2.z = v_2.z - v_0.z;

    /* Calculo do produto vetorial de v1 e v2 */
    vn->x = (v1.y * v2.z) - (v1.z * v2.y);
    vn->y = (v1.z * v2.x) - (v1.x * v2.z);
    vn->z = (v1.x * v2.y) - (v1.y * v2.x);

    /* normalizacao de n */
    len = sqrt(pow(vn->x, 2) + pow(vn->y, 2) + pow(vn->z, 2));

    vn->x /= len;
    vn->y /= len;
    vn->z /= len;
}

void atualizaVetorSeta()
{
    ySeta = sqrt(pow(0.60, 2) - pow(vetorSeta->v1.x, 2)); ///pit�goras: mant�m o m�dulo do vetor constante = 0.40
    if (ySeta >= 0)                                       ///a bolinha n�o deve come�ar indo pra baixo
    {
        vetorSeta->v1.y = ySeta;
    }
}

void moveBolinha()
{
    if (!pausado)
    {
        xBolinha += vetorMovimentoBolinha.v1.x;
        yBolinha += vetorMovimentoBolinha.v1.y;
    }
}

float calculaModuloVetor(vetor v)
{
    float somaQuadrados = pow(v.v1.x, 2.0) + pow(v.v1.y, 2.0);
    float modulo = sqrt(somaQuadrados);
    return modulo;
}

float calculaProdutoEscalar(vertice v1, vertice v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}
float calculaAnguloEntreVetores(vertice verticeNormal, vetor vetorMovimentoBolinha)
{
    float produtoEscalar = calculaProdutoEscalar(verticeNormal, vetorMovimentoBolinha.v1);
    vetor vetorNormal;
    vetorNormal.v1 = verticeNormal;

    float produtoDosModulos = calculaModuloVetor(vetorNormal) * calculaModuloVetor(vetorMovimentoBolinha);

    float theta = acos(produtoEscalar / produtoDosModulos);

    float angulo = theta*(180/3.1416);
    return angulo;
}
void refleteBolinha(vertice verticeNormal, int origemColisao = -1)
{
    float angulo = calculaAnguloEntreVetores(verticeNormal, vetorMovimentoBolinha);
    if(angulo > 90 && origemColisao!=ultimaColisao)
    {
        vetor vetorNormal;
        vetorNormal.v1 = verticeNormal;

        float produtoEscalar = calculaProdutoEscalar(verticeNormal, vetorMovimentoBolinha.v1);

        vetor refletido;
        refletido.v1.x = vetorMovimentoBolinha.v1.x - 2 * produtoEscalar * verticeNormal.x;
        refletido.v1.y = vetorMovimentoBolinha.v1.y - 2 * produtoEscalar * verticeNormal.y;

        vetorMovimentoBolinha = refletido;
        cout << angulo << endl;

        ultimaColisao = origemColisao;
    }
}

void reflexaoBarra()
{
    vertice vetorNormal;
    vertice verticesBarraFaceSuperior[3] = {{0.25 + xBarra, -0.625, 0.0625},
        {0.25 + xBarra, -0.625, 0.125},
        {-0.25 + xBarra, -0.625, 0.125}
    };

    triangle t = {verticesBarraFaceSuperior[0], verticesBarraFaceSuperior[1], verticesBarraFaceSuperior[2]};
    CalculaNormal(t, &vetorNormal);

    if (yBolinha < -0.60 && fabs(xBarra - xBolinha) < 0.4)
        refleteBolinha(vetorNormal);
}

void reflexaoBloquinhos()
{
    if (xBolinha > -0.97 && xBolinha < -0.63)
    {
        if (vetorBloquinhos[10].mostra)
        {
            if ((yBolinha < 0.227 && yBolinha > 0.223) ||
                    (yBolinha > 0.352 && yBolinha < 0.358))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[10].mostra = false;
            }
            if ((yBolinha > 0.227 && yBolinha < 0.358) && (xBolinha < -0.67 && xBolinha > -0.66))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[10].mostra = false;
            }
        }

        if (vetorBloquinhos[5].mostra)
        {
            if ((yBolinha > 0.447 && yBolinha < 0.452) || (yBolinha > 0.577 && yBolinha < 0.583))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[5].mostra = false;
            }
            if ((yBolinha > 0.447 && yBolinha < 0.583) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[5].mostra = false;
            }
        }

        if (vetorBloquinhos[0].mostra)
        {
            if ((yBolinha > 0.672 && yBolinha < 0.678) || (yBolinha > 0.802 && yBolinha < 0.808))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[0].mostra = false;
            }
            if ((yBolinha > 0.672 && yBolinha < 0.808) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[0].mostra = false;
            }
        }
    }

    if (xBolinha > -0.57 && xBolinha < -0.23)
    {
        if (vetorBloquinhos[11].mostra)
        {
            if ((yBolinha < 0.227 && yBolinha > 0.223) ||
                    (yBolinha > 0.352 && yBolinha < 0.358))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[11].mostra = false;
            }
            if ((yBolinha > 0.227 && yBolinha < 0.358) && (xBolinha < -0.67 && xBolinha > -0.66))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[11].mostra = false;
            }
        }

        if (vetorBloquinhos[6].mostra)
        {
            if ((yBolinha > 0.44 && yBolinha < 0.46) || (yBolinha > 0.577 && yBolinha < 0.583))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[6].mostra = false;
            }
            if ((yBolinha > 0.447 && yBolinha < 0.583) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[6].mostra = false;
            }
        }

        if (vetorBloquinhos[1].mostra)
        {
            if ((yBolinha > 0.67 && yBolinha < 0.68) || (yBolinha > 0.802 && yBolinha < 0.808))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[1].mostra = false;
            }
            if ((yBolinha > 0.672 && yBolinha < 0.808) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[1].mostra = false;
            }
        }
    }

    if (xBolinha > -0.17 && xBolinha < 0.23)
    {
        if (vetorBloquinhos[12].mostra)
        {
            if ((yBolinha < 0.227 && yBolinha > 0.223) ||
                    (yBolinha > 0.352 && yBolinha < 0.358))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[12].mostra = false;
            }
            if ((yBolinha > 0.227 && yBolinha < 0.358) && (xBolinha < -0.67 && xBolinha > -0.66))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[12].mostra = false;
            }
        }

        if (vetorBloquinhos[7].mostra)
        {
            if ((yBolinha > 0.44 && yBolinha < 0.46) || (yBolinha > 0.577 && yBolinha < 0.583))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[7].mostra = false;
            }
            if ((yBolinha > 0.447 && yBolinha < 0.583) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[7].mostra = false;
            }
        }

        if (vetorBloquinhos[2].mostra)
        {
            if ((yBolinha > 0.67 && yBolinha < 0.68) || (yBolinha > 0.802 && yBolinha < 0.808))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[2].mostra = false;
            }
            if ((yBolinha > 0.672 && yBolinha < 0.808) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[2].mostra = false;
            }
        }
    }

    if (xBolinha > 0.27 && xBolinha < 0.63)
    {
        if (vetorBloquinhos[13].mostra)
        {
            if ((yBolinha < 0.227 && yBolinha > 0.223) ||
                    (yBolinha > 0.352 && yBolinha < 0.358))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[13].mostra = false;
            }
            if ((yBolinha > 0.227 && yBolinha < 0.358) && (xBolinha < -0.67 && xBolinha > -0.66))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[13].mostra = false;
            }
        }

        if (vetorBloquinhos[8].mostra)
        {
            if ((yBolinha > 0.44 && yBolinha < 0.46) || (yBolinha > 0.577 && yBolinha < 0.583))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[8].mostra = false;
            }
            if ((yBolinha > 0.44 && yBolinha < 0.583) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[8].mostra = false;
            }
        }

        if (vetorBloquinhos[3].mostra)
        {
            if ((yBolinha > 0.67 && yBolinha < 0.68) || (yBolinha > 0.802 && yBolinha < 0.808))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[3].mostra = false;
            }
            if ((yBolinha > 0.672 && yBolinha < 0.808) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[3].mostra = false;
            }
        }
    }

    if (xBolinha > 0.67 && xBolinha < 0.97)
    {
        if (vetorBloquinhos[14].mostra)
        {
            if ((yBolinha < 0.227 && yBolinha > 0.223) ||
                    (yBolinha > 0.352 && yBolinha < 0.358))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[14].mostra = false;
            }
            if ((yBolinha > 0.227 && yBolinha < 0.358) && (xBolinha < -0.67 && xBolinha > -0.66))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[14].mostra = false;
            }
        }

        if (vetorBloquinhos[9].mostra)
        {
            if ((yBolinha > 0.44 && yBolinha < 0.46) || (yBolinha > 0.577 && yBolinha < 0.583))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[9].mostra = false;
            }
            if ((yBolinha > 0.44 && yBolinha < 0.583) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[9].mostra = false;
            }
        }

        if (vetorBloquinhos[4].mostra)
        {
            if ((yBolinha > 0.67 && yBolinha < 0.68) || (yBolinha > 0.802 && yBolinha < 0.808))
            {
                vetorMovimentoBolinha.v1.y *= -1;
                vetorBloquinhos[4].mostra = false;
            }
            if ((yBolinha > 0.672 && yBolinha < 0.808) && (xBolinha < -0.6 && xBolinha > -0.7))
            {
                vetorMovimentoBolinha.v1.x *= -1;
                vetorBloquinhos[4].mostra = false;
            }
        }
    }
}

void preencheVetorBarrinhas()
{
    for (int i = 0; i < 5; i++)
    {
        vetorBloquinhos[i].mostra = true;
    }

    for (int i = 5; i < 10; i++)
    {
        if (fase == 2 || fase == 3)
            vetorBloquinhos[i].mostra = true;
        else
            vetorBloquinhos[i].mostra = false;
    }

    for (int i = 10; i < 15; i++)
    {
        if (fase == 3)
            vetorBloquinhos[i].mostra = true;
        else
            vetorBloquinhos[i].mostra = false;
    }
}

bool verificaPassouDeFase()
{
    for (int i = 0; i < 15; i++)
    {
        if (vetorBloquinhos[i].mostra)
            return false;
    }
    return true;
}

void restart(bool PassouDeFase = false)
{
    pintaPlataformaVermelho = false;
    xBarra = 0;
    xBolinha = 0;
    yBolinha = yBolinhaInicial;
    vetorMovimentoBolinha.v1.x = 0;
    vetorMovimentoBolinha.v1.y = 0;
    desenhaSetaControle = true;
    primeiroLancamento = false;

    if (!PassouDeFase)
    {
        if (vidasRestantes == 1)
        {
            fase = 1;
            preencheVetorBarrinhas();
            vidasRestantes = 5;
        }
        else
        {
            vidasRestantes--;
        }
    }
    ultimaColisao = -1;
}

void passaDeFase()
{
    if (verificaPassouDeFase())
    {
        if (fase < 3)
            fase++;
        else
            fase = 1;

        preencheVetorBarrinhas();
        restart(true);
    }
}

bool GameOver()
{
    if (yBolinha < -0.8)
        pintaPlataformaVermelho = true;
    if (yBolinha < -1.5)
    {
        vertice v;
        v.x = 0;
        v.y = 0;
        vetor VetorZero;
        VetorZero.v1 = v;
        vetorMovimentoBolinha = VetorZero;
        restart();
        return true;
    }
    return false;
}

bool verificaColisaoX(vertice v)
{
    return (fabs(v.x - xBolinha) < 0.1);
}

bool verificaColisaoY(vertice v)
{
    return (fabs(v.y - yBolinha) < 0.1);
}

bool verificaColisaoTriangulo(triangle t)
{
    float xBaricentro = (t.v[0].x + t.v[1].x + t.v[2].x) / 3;
    float yBaricentro = (t.v[0].y + t.v[1].y + t.v[2].y) / 3;
    float somaDosQuadrados = pow(xBolinha - xBaricentro, 2.0) + pow(yBolinha - yBaricentro, 2.0);
    float distancia = sqrt(somaDosQuadrados);
    return (distancia < 0.07);
}

void desenhaPlataforma()
{

    vertice vetorNormal;
    vertice base[4] = {{-1.0f, -1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f}
    };

    vertice faceDireita[4] = {{1.0f, -1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.25f},
        {1.0f, -1.0f, 0.25f}
    };

    vertice faceSuperior[4] = {{-1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.25f},
        {-1.0f, 1.0f, 0.25f}
    };

    ///ESTRUTURAS FACE ESQUERDA - INICIO
    vertice faceEsquerdaInferior[9];
    float xFaceEsquerdaInferior = -1.0f;
    float yFaceEsquerdaInferior = -1.0f;
    float zFaceEsquerdaInferior = 0.25f;
    faceEsquerdaInferior[0] = {xFaceEsquerdaInferior, yFaceEsquerdaInferior, zFaceEsquerdaInferior};
    for (int i = 1; i < 9; i++)
    {
        if (i % 2 == 0)
        {
            zFaceEsquerdaInferior += 0.25f;
        }
        else
        {
            zFaceEsquerdaInferior -= 0.25f;
        }
        faceEsquerdaInferior[i] = {xFaceEsquerdaInferior, yFaceEsquerdaInferior, zFaceEsquerdaInferior};

        if (i == 1 || i == 6)
        {
            yFaceEsquerdaInferior += 0.10;
        }
        else if (i == 7)
        {
            continue;
        }
        else
        {
            yFaceEsquerdaInferior += 0.05;
        }
    }

    vertice faceEsquerdaSuperior[9];
    float xFaceEsquerdaSuperior = -1.0f;
    float yFaceEsquerdaSuperior = 0.6f;
    float zFaceEsquerdaSuperior = 0.25f;
    faceEsquerdaSuperior[0] = {xFaceEsquerdaSuperior, yFaceEsquerdaSuperior, zFaceEsquerdaSuperior};
    for (int i = 1; i < 9; i++)
    {
        if (i % 2 == 0)
        {
            zFaceEsquerdaSuperior += 0.25f;
        }
        else
        {
            zFaceEsquerdaSuperior -= 0.25f;
        }
        faceEsquerdaSuperior[i] = {xFaceEsquerdaSuperior, yFaceEsquerdaSuperior, zFaceEsquerdaSuperior};

        if (i == 1 || i == 6)
        {
            yFaceEsquerdaSuperior += 0.10;
        }
        else if (i == 7)
        {
            continue;
        }
        else
        {
            yFaceEsquerdaSuperior += 0.05;
        }
    }

    vertice faceEsquerdaBarriga[20];
    float passoX = 0.01f;
    float passoY = 0.1333f; /// 1.2/9
    float xBarriga = -1.0f;
    float yBarriga = -0.6f;
    float zBarriga = 0.0f;
    int k = 4;
    faceEsquerdaBarriga[0] = {xBarriga, yBarriga, zBarriga};
    for (int i = 1; i < 20; i++)
    {
        if (i % 2 == 0)
            zBarriga = 0.00f;
        else
            zBarriga = 0.25f;

        faceEsquerdaBarriga[i] = {xBarriga, yBarriga, zBarriga};

        if (i % 2 != 0)
        {
            if (i >= 9)
            {
                xBarriga -= k * passoX;
                k++;
            }
            else
            {
                xBarriga += k * passoX;
                k--;
            }
            yBarriga += passoY;
        }
    }

    triangle trianguloBarrigaEsquerda[18];
    for (int i = 0; i < 18; i++)
    {
        trianguloBarrigaEsquerda[i] = {faceEsquerdaBarriga[i], faceEsquerdaBarriga[i + 1], faceEsquerdaBarriga[i + 2]};
    }
    ///ESTRUTURAS FACE ESQUERDA - FIM

    ///ESTRUTURAS FACE DIREITA - INICIO

    vertice faceDireitaBarriga[20];
    float passoXDir = 0.01f;
    float passoYDir = 0.1333f; /// 1.2/9
    float xBarrigaDir = 1.0f;
    float yBarrigaDir = -0.6f;
    float zBarrigaDir = 0.0f;
    int kDir = 4;
    faceDireitaBarriga[0] = {xBarrigaDir, yBarrigaDir, zBarrigaDir};
    for (int i = 1; i < 20; i++)
    {
        if (i % 2 == 0)
            zBarrigaDir = 0.00f;
        else
            zBarrigaDir = 0.25f;

        faceDireitaBarriga[i] = {xBarrigaDir, yBarrigaDir, zBarrigaDir};

        if (i % 2 != 0)
        {
            if (i >= 9)
            {
                xBarrigaDir += kDir * passoXDir;
                kDir++;
            }
            else
            {
                xBarrigaDir -= kDir * passoXDir;
                kDir--;
            }
            yBarrigaDir += passoYDir;
        }
    }

    triangle triangulosBarrigaDireita[18];
    for (int i = 0; i < 18; i++)
    {
        triangulosBarrigaDireita[i] = {faceDireitaBarriga[i], faceDireitaBarriga[i + 1], faceDireitaBarriga[i + 2]};
    }

    vertice faceDireitaInferior[9];
    float xFaceDireitaInferior = 1.0f;
    float yFaceDireitaInferior = -1.0f;
    float zFaceDireitaInferior = 0.25f;
    faceDireitaInferior[0] = {xFaceDireitaInferior, yFaceDireitaInferior, zFaceDireitaInferior};
    for (int i = 1; i < 9; i++)
    {
        if (i % 2 == 0)
        {
            zFaceDireitaInferior += 0.25f;
        }
        else
        {
            zFaceDireitaInferior -= 0.25f;
        }
        faceDireitaInferior[i] = {xFaceDireitaInferior, yFaceDireitaInferior, zFaceDireitaInferior};

        if (i == 1 || i == 6)
        {
            yFaceDireitaInferior += 0.10;
        }
        else if (i == 7)
        {
            continue;
        }
        else
        {
            yFaceDireitaInferior += 0.05;
        }
    }

    vertice faceDireitaSuperior[9];
    float xFaceDireitaSuperior = 1.0f;
    float yFaceDireitaSuperior = 0.6f;
    float zFaceDireitaSuperior = 0.25f;
    float contTx = 0.0;
    float contTy = 0.0;
    faceDireitaSuperior[0] = {xFaceDireitaSuperior, yFaceDireitaSuperior, zFaceDireitaSuperior};
    for (int i = 1; i < 9; i++)
    {
        if (i % 2 == 0)
        {
            zFaceDireitaSuperior += 0.25f;
        }
        else
        {
            zFaceDireitaSuperior -= 0.25f;
        }
        faceDireitaSuperior[i] = {xFaceDireitaSuperior, yFaceDireitaSuperior, zFaceDireitaSuperior};

        if (i == 1 || i == 6)
        {
            yFaceDireitaSuperior += 0.10;
        }
        else if (i == 7)
        {
            continue;
        }
        else
        {
            yFaceDireitaSuperior += 0.05;
        }
    }

    triangle t[27] = {{base[0], base[1], base[3]},
        {faceEsquerdaInferior[0], faceEsquerdaInferior[1], faceEsquerdaInferior[2]},
        {faceEsquerdaInferior[1], faceEsquerdaInferior[2], faceEsquerdaInferior[3]},
        {faceEsquerdaInferior[2], faceEsquerdaInferior[3], faceEsquerdaInferior[4]},
        {faceEsquerdaInferior[3], faceEsquerdaInferior[4], faceEsquerdaInferior[5]},
        {faceEsquerdaInferior[4], faceEsquerdaInferior[5], faceEsquerdaInferior[6]},
        {faceEsquerdaInferior[5], faceEsquerdaInferior[6], faceEsquerdaInferior[7]},

        {faceEsquerdaSuperior[0], faceEsquerdaSuperior[1], faceEsquerdaSuperior[2]}, /// indice 7
        {faceEsquerdaSuperior[1], faceEsquerdaSuperior[2], faceEsquerdaSuperior[3]},
        {faceEsquerdaSuperior[2], faceEsquerdaSuperior[3], faceEsquerdaSuperior[4]},
        {faceEsquerdaSuperior[3], faceEsquerdaSuperior[4], faceEsquerdaSuperior[5]},
        {faceEsquerdaSuperior[4], faceEsquerdaSuperior[5], faceEsquerdaSuperior[6]},
        {faceEsquerdaSuperior[5], faceEsquerdaSuperior[6], faceEsquerdaSuperior[7]},

        {faceDireita[0], faceDireita[1], faceDireita[2]},
        {faceSuperior[0], faceSuperior[1], faceSuperior[2]},

        {faceDireitaInferior[0], faceDireitaInferior[1], faceDireitaInferior[2]}, /// indice 15
        {faceDireitaInferior[1], faceDireitaInferior[2], faceDireitaInferior[3]},
        {faceDireitaInferior[2], faceDireitaInferior[3], faceDireitaInferior[4]},
        {faceDireitaInferior[3], faceDireitaInferior[4], faceDireitaInferior[5]},
        {faceDireitaInferior[4], faceDireitaInferior[5], faceDireitaInferior[6]},
        {faceDireitaInferior[5], faceDireitaInferior[6], faceDireitaInferior[7]},

        {faceDireitaSuperior[0], faceDireitaSuperior[1], faceDireitaSuperior[2]}, /// indice 21
        {faceDireitaSuperior[1], faceDireitaSuperior[2], faceDireitaSuperior[3]},
        {faceDireitaSuperior[2], faceDireitaSuperior[3], faceDireitaSuperior[4]},
        {faceDireitaSuperior[3], faceDireitaSuperior[4], faceDireitaSuperior[5]},
        {faceDireitaSuperior[4], faceDireitaSuperior[5], faceDireitaSuperior[6]},
        {faceDireitaSuperior[5], faceDireitaSuperior[6], faceDireitaSuperior[7]} ///indice 26
    };

    /// ESTRUTURAS FACE DIREITA - FIM

    if (pintaPlataformaVermelho)
        setColor(1.0, 0.0, 0.0);
    textureManager->Bind(12);
    glBegin(GL_QUADS);
    CalculaNormal(t[0], &vetorNormal);
    glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
    for (int i = 0; i < 4; i++)
    {
        if(base[i].x == -1.0 && base[i].y == -1.0)
        {
            glTexCoord2f(0.0, 0.0);
        }
        if(base[i].x == 1.0 && base[i].y == -1.0)
        {
            glTexCoord2f(1.0, 0.0);
        }
        if(base[i].x == 1.0 && base[i].y == 1.0)
        {
            glTexCoord2f(1.0, 1.0);
        }
        if(base[i].x == -1.0 && base[i].y == 1.0)
        {
            glTexCoord2f(0.0, 1.0);
        }

        glVertex3f(base[i].x, base[i].y, base[i].z);
    }
    glEnd();
    textureManager->Disable();

    /// ---------------- FACE ESQUERDA ---------------
    /// parte inferior
    setColor(0.6, 0.6, 0.9);
    textureManager->Bind(11);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 1; i < 6; i++)
    {
        CalculaNormal(t[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(t[i]))
        {
            refleteBolinha(vetorNormal, 5);
        }
    }
    for (int i = 0; i < 9; i++)
    {
        if(i == 0)
            glTexCoord2f(0.0, 1.0);
        if(i == 1)
            glTexCoord2f(0.0, 0.0);
        if(i == 2)
            glTexCoord2f(0.15, 1.0);
        if(i == 3)
            glTexCoord2f(0.3, 0.0);
        if(i == 4)
            glTexCoord2f(0.45, 1.0);
        if(i == 5)
            glTexCoord2f(0.6, 0.0);
        if(i == 6)
            glTexCoord2f(0.75, 1.0);
        if(i == 7)
            glTexCoord2f(0.9, 0.0);
        if(i == 8)
            glTexCoord2f(0.9, 1.0);

        glVertex3f(faceEsquerdaInferior[i].x, faceEsquerdaInferior[i].y, faceEsquerdaInferior[i].z);
    }
    glEnd();

    /// parte superior
    textureManager->Bind(11);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 7; i < 12; i++)
    {
        CalculaNormal(t[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(t[i]))
        {
            refleteBolinha(vetorNormal, 6);
        }
    }
    for (int i = 0; i < 9; i++)
    {
        if(i == 0)
            glTexCoord2f(0.0, 1.0);
        if(i == 1)
            glTexCoord2f(0.0, 0.0);
        if(i == 2)
            glTexCoord2f(0.15, 1.0);
        if(i == 3)
            glTexCoord2f(0.3, 0.0);
        if(i == 4)
            glTexCoord2f(0.45, 1.0);
        if(i == 5)
            glTexCoord2f(0.6, 0.0);
        if(i == 6)
            glTexCoord2f(0.75, 1.0);
        if(i == 7)
            glTexCoord2f(0.9, 0.0);
        if(i == 8)
            glTexCoord2f(0.9, 1.0);

        glVertex3f(faceEsquerdaSuperior[i].x, faceEsquerdaSuperior[i].y, faceEsquerdaSuperior[i].z);
    }
    glEnd();

    /// barriga esquerda
    for (int i = 1; i <= 18; i++) /// setta as normais
    {
        CalculaNormal(trianguloBarrigaEsquerda[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(trianguloBarrigaEsquerda[i]) && vetorMovimentoBolinha.v1.x * vetorNormal.x < 1)
        {
            refleteBolinha(vetorNormal, 2);
        }
    }

    textureManager->Bind(13);
    glBegin(GL_TRIANGLE_STRIP); ///desenha a barriga j� costurando os tri�ngulos
    for (int i = 0; i < 20; i++)
    {
        if( i == 0)
        {
            glTexCoord2f(0.0, 0.0);
        }

        if( i == 1)
        {
            glTexCoord2f(0.0, 1.0);
        }

        if( i == 2)
        {
            glTexCoord2f(0.11, 0.0);
        }

        if( i == 3)
        {
            glTexCoord2f(0.11, 1.0);
        }

        if( i == 4)
        {
            glTexCoord2f(0.22, 0.0);
        }

        if( i == 5)
        {
            glTexCoord2f(0.22, 1.0);
        }

        if( i == 6)
        {
            glTexCoord2f(0.33, 0.0);
        }

        if( i == 7)
        {
            glTexCoord2f(0.33, 1.0);
        }

        if( i == 8)
        {
            glTexCoord2f(0.44, 0.0);
        }

        if( i == 9)
        {
            glTexCoord2f(0.44, 1.0);
        }

        if( i == 10)
        {
            glTexCoord2f(0.55, 0.0);
        }

        if( i == 11)
        {
            glTexCoord2f(0.55, 1.0);
        }

        if( i == 12)
        {
            glTexCoord2f(0.66, 0.0);
        }

        if( i == 13)
        {
            glTexCoord2f(0.66, 1.0);
        }

        if( i == 14)
        {
            glTexCoord2f(0.77, 0.0);
        }

        if( i == 15)
        {
            glTexCoord2f(0.77, 1.0);
        }

        if( i == 16)
        {
            glTexCoord2f(0.88, 0.0);
        }

        if( i == 17)
        {
            glTexCoord2f(0.88, 1.0);
        }

        if( i == 18)
        {
            glTexCoord2f(0.99, 0.0);
        }

        if( i == 19)
        {
            glTexCoord2f(0.99, 1.0);
        }

        glVertex3f(faceEsquerdaBarriga[i].x, faceEsquerdaBarriga[i].y, faceEsquerdaBarriga[i].z);
    }
    glEnd();

    glBegin(GL_POLYGON); /// Preenche a tampa da barriga
    for (int i = 1; i < 20; i += 2)
    {
        glVertex3f(faceEsquerdaBarriga[i].x, faceEsquerdaBarriga[i].y, faceEsquerdaBarriga[i].z);
    }
    glEnd();

    glBegin(GL_POLYGON); /// preenche a lateral de fora da barriga
    glVertex3f(faceEsquerdaBarriga[0].x, faceEsquerdaBarriga[0].y, faceEsquerdaBarriga[0].z);
    glVertex3f(faceEsquerdaBarriga[18].x, faceEsquerdaBarriga[18].y, faceEsquerdaBarriga[18].z);
    glVertex3f(faceEsquerdaBarriga[19].x, faceEsquerdaBarriga[19].y, faceEsquerdaBarriga[19].z);
    glVertex3f(faceEsquerdaBarriga[1].x, faceEsquerdaBarriga[1].y, faceEsquerdaBarriga[1].z);
    glEnd();
    ///--------FIM DA FACE ESQUERDA---------


    /// --------INICIO DA FACE ESQUERDA-----

    ///------BARRIGA DIREITA-----
    textureManager->Bind(13);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 1; i <= 18; i++)
    {
        CalculaNormal(triangulosBarrigaDireita[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(triangulosBarrigaDireita[i]) && vetorMovimentoBolinha.v1.x * vetorNormal.x < 1)
        {
            refleteBolinha(vetorNormal, 3);
        }
    }
    ///desenha a barriga j� costurando os tri�ngulos
    for (int i = 0; i < 20; i++)
    {
        if( i == 0)
        {
            glTexCoord2f(0.0, 0.0);
        }

        if( i == 1)
        {
            glTexCoord2f(0.0, 1.0);
        }

        if( i == 2)
        {
            glTexCoord2f(0.11, 0.0);
        }

        if( i == 3)
        {
            glTexCoord2f(0.11, 1.0);
        }

        if( i == 4)
        {
            glTexCoord2f(0.22, 0.0);
        }

        if( i == 5)
        {
            glTexCoord2f(0.22, 1.0);
        }

        if( i == 6)
        {
            glTexCoord2f(0.33, 0.0);
        }

        if( i == 7)
        {
            glTexCoord2f(0.33, 1.0);
        }

        if( i == 8)
        {
            glTexCoord2f(0.44, 0.0);
        }

        if( i == 9)
        {
            glTexCoord2f(0.44, 1.0);
        }

        if( i == 10)
        {
            glTexCoord2f(0.55, 0.0);
        }

        if( i == 11)
        {
            glTexCoord2f(0.55, 1.0);
        }

        if( i == 12)
        {
            glTexCoord2f(0.66, 0.0);
        }

        if( i == 13)
        {
            glTexCoord2f(0.66, 1.0);
        }

        if( i == 14)
        {
            glTexCoord2f(0.77, 0.0);
        }

        if( i == 15)
        {
            glTexCoord2f(0.77, 1.0);
        }

        if( i == 16)
        {
            glTexCoord2f(0.88, 0.0);
        }

        if( i == 17)
        {
            glTexCoord2f(0.88, 1.0);
        }

        if( i == 18)
        {
            glTexCoord2f(0.99, 0.0);
        }

        if( i == 19)
        {
            glTexCoord2f(0.99, 1.0);
        }

        glVertex3f(faceDireitaBarriga[i].x, faceDireitaBarriga[i].y, faceDireitaBarriga[i].z);
    }
    glEnd();

    glBegin(GL_POLYGON); /// Preenche a tampa da barriga
    for (int i = 1; i < 20; i += 2)
    {
        glVertex3f(faceDireitaBarriga[i].x, faceDireitaBarriga[i].y, faceDireitaBarriga[i].z);
    }
    glEnd();

    glBegin(GL_POLYGON); /// preenche a lateral de fora da barriga
    glVertex3f(faceDireitaBarriga[0].x, faceDireitaBarriga[0].y, faceDireitaBarriga[0].z);
    glVertex3f(faceDireitaBarriga[18].x, faceDireitaBarriga[18].y, faceDireitaBarriga[18].z);
    glVertex3f(faceDireitaBarriga[19].x, faceDireitaBarriga[19].y, faceDireitaBarriga[19].z);
    glVertex3f(faceDireitaBarriga[1].x, faceDireitaBarriga[1].y, faceDireitaBarriga[1].z);
    glEnd();
    /// FIM DA BARRIGA DIREITA

    setColor(0.6, 0.6, 0.9);
    textureManager->Bind(11);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 15; i < 20; i++)
    {
        CalculaNormal(t[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(t[i]))
        {
            refleteBolinha(vetorNormal, 15);
        }
    }
    for (int i = 0; i < 9; i++)
    {
        if(i == 0)
            glTexCoord2f(0.0, 1.0);
        if(i == 1)
            glTexCoord2f(0.0, 0.0);
        if(i == 2)
            glTexCoord2f(0.15, 1.0);
        if(i == 3)
            glTexCoord2f(0.3, 0.0);
        if(i == 4)
            glTexCoord2f(0.45, 1.0);
        if(i == 5)
            glTexCoord2f(0.6, 0.0);
        if(i == 6)
            glTexCoord2f(0.75, 1.0);
        if(i == 7)
            glTexCoord2f(0.9, 0.0);
        if(i == 8)
            glTexCoord2f(0.9, 1.0);

        glVertex3f(faceDireitaInferior[i].x, faceDireitaInferior[i].y, faceDireitaInferior[i].z);
    }
    glEnd();

    /// parte superior
    textureManager->Bind(11);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 21; i < 26; i++)
    {
        CalculaNormal(t[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(t[i]))
        {
            refleteBolinha(vetorNormal, 16);
        }
    }
    for (int i = 0; i < 9; i++)
    {
        if(i == 0)
            glTexCoord2f(0.0, 1.0);
        if(i == 1)
            glTexCoord2f(0.0, 0.0);
        if(i == 2)
            glTexCoord2f(0.15, 1.0);
        if(i == 3)
            glTexCoord2f(0.3, 0.0);
        if(i == 4)
            glTexCoord2f(0.45, 1.0);
        if(i == 5)
            glTexCoord2f(0.6, 0.0);
        if(i == 6)
            glTexCoord2f(0.75, 1.0);
        if(i == 7)
            glTexCoord2f(0.9, 0.0);
        if(i == 8)
            glTexCoord2f(0.9, 1.0);

        glVertex3f(faceDireitaSuperior[i].x, faceDireitaSuperior[i].y, faceDireitaSuperior[i].z);
    }
    glEnd();

    /// --------FIM DA FACE ESQUERDA-----


    setColor(0.6, 0.6, 0.9);
    textureManager->Bind(11);
    glBegin(GL_QUADS);
    CalculaNormal(t[14], &vetorNormal); // Passa face triangular e endere�o do vetor normal de sa�da
    glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
    for (int i = 0; i < 4; i++)
    {
        if(i == 0)
        {
            glTexCoord2f(0.0, 0.0);
        }
        if(i == 1)
        {
            glTexCoord2f(1.0, 0.0);
        }
        if(i == 2)
        {
            glTexCoord2f(1.0, 1.0);
        }
        if(i == 3)
        {
            glTexCoord2f(0.0, 1.0);
        }

        glVertex3f(faceSuperior[i].x, faceSuperior[i].y, faceSuperior[i].z);
    }
    glEnd();
    if (verificaColisaoY(faceSuperior[0]))
    {
        refleteBolinha(vetorNormal, 7);
    }
}

void desenhaRebatedor()
{
    vertice barrigaRebatedor[20];
    float passoX = 0.05555f; /// 0.5/9
    float passoY = 0.01f;
    float xBarrigaRebatedor = -0.25f + xBarra;
    float yBarrigaRebatedor = -0.625f;
    float zBarrigaRebatedor = 0.0f;
    int k = 4;
    barrigaRebatedor[0] = {xBarrigaRebatedor, yBarrigaRebatedor, zBarrigaRebatedor};
    float contTx = 0;
    float contTy = 0;

    for (int i = 1; i < 20; i++)
    {
        if (i % 2 == 0)
            zBarrigaRebatedor = 0.00f;
        else
            zBarrigaRebatedor = 0.25f;

        barrigaRebatedor[i] = {xBarrigaRebatedor, yBarrigaRebatedor, zBarrigaRebatedor};

        if (i % 2 != 0)
        {
            if (i >= 9)
            {
                yBarrigaRebatedor -= k * passoY;
                k++;
            }
            else
            {
                yBarrigaRebatedor += k * passoY;
                k--;
            }
            xBarrigaRebatedor += passoX;
        }
    }

    textureManager->Bind(0);
    //setColor(1.0, 0.3, 0.3);
    glBegin(GL_TRIANGLE_STRIP); ///desenha a barriga j� costurando os tri�ngulos
    for (int i = 0; i < 20; i++)
    {
        if( i == 0)
        {
            glTexCoord2f(0.0, 0.0);
        }
        else
        {
            glTexCoord2f(contTx, contTy);
        }
        glVertex3f(barrigaRebatedor[i].x, barrigaRebatedor[i].y, barrigaRebatedor[i].z);

        if(i % 2 == 0)
        {
            contTx += 0.1;
            contTy += 0.1;
        }

    }
    glEnd();
    contTx = 0;
    contTy = 0;

    textureManager->Bind(0);
    glBegin(GL_POLYGON); /// Preenche a tampa da barriga
    for (int i = 1; i < 20; i += 2)
    {
        if( i == 1)
        {
            glTexCoord2f(0.0, 0.0);
        }
        else
        {
            glTexCoord2f(contTx, contTy);
        }
        glVertex3f(barrigaRebatedor[i].x, barrigaRebatedor[i].y, barrigaRebatedor[i].z);
        if(i % 2 != 0)
        {
            contTx += 0.1;
            contTy += 0.1;
        }
    }
    glEnd();
    textureManager->Bind(3);
    glBegin(GL_POLYGON); /// preenche a lateral de fora da barriga

    glTexCoord2f(0.0, 0.0);
    glVertex3f(barrigaRebatedor[0].x, barrigaRebatedor[0].y, barrigaRebatedor[0].z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(barrigaRebatedor[18].x, barrigaRebatedor[18].y, barrigaRebatedor[18].z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(barrigaRebatedor[19].x, barrigaRebatedor[19].y, barrigaRebatedor[19].z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(barrigaRebatedor[1].x, barrigaRebatedor[1].y, barrigaRebatedor[1].z);
    glEnd();
    textureManager->Disable();

    triangle triangulosBarrigaRebatedor[18];
    for (int i = 0; i < 18; i++)
    {
        triangulosBarrigaRebatedor[i] = {barrigaRebatedor[i], barrigaRebatedor[i + 1], barrigaRebatedor[i + 2]};
    }

    vertice vetorNormal;
    for (int i = 1; i < 18; i++) /// setta as normais
    {
        CalculaNormal(triangulosBarrigaRebatedor[i], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        if (verificaColisaoTriangulo(triangulosBarrigaRebatedor[i]))
            refleteBolinha(vetorNormal, 1);
    }
}

void desenhaBarrinhasDeBater()
{
    int colunas = 0;
    int linhas = 0;
    int cont = 0;

    float inicio = -0.95;
    float fim = -0.65;

    float inicioh = 0.90;
    float fimh = 0.775;

    while (linhas < 3)
    {
        colunas = 0;
        inicio = -0.95;
        fim = -0.65;

        while (colunas < 5)
        {
            vertice vetorNormal;
            vertice barraFaceTampa[4] = {{fim, inicioh, 0.125},
                {inicio, inicioh, 0.125},
                {inicio, fimh, 0.125},
                {fim, fimh, 0.125}
            };

            vertice barraFaceBase[4] = {{fim, fimh, 0.0625},
                {fim, inicioh, 0.0625},
                {inicio, inicioh, 0.0625},
                {inicio, fimh, 0.0625}
            };

            vertice barraFaceDireita[4] = {{inicio, inicioh, 0.0625},
                {inicio, fimh, 0.0625},
                {inicio, fimh, 0.125},
                {inicio, inicioh, 0.125}
            };

            vertice barraFaceSuperior[4] = {{inicio, fimh, 0.0625},
                {inicio, fimh, 0.125},
                {fim, fimh, 0.125},
                {fim, fimh, 0.0625}
            };

            vertice barraFaceEsquerda[4] = {{fim, inicioh, 0.0625},
                {fim, fimh, 0.0625},
                {fim, fimh, 0.125},
                {fim, inicioh, 0.125}
            };

            vertice barraFaceInferior[4] =
            {
                {inicio, inicioh, 0.0625},
                {fim, inicioh, 0.0625},
                {fim, inicioh, 0.125},
                {inicio, inicioh, 0.125},
            };

            triangle t[6] =
            {
                {barraFaceTampa[0], barraFaceTampa[1], barraFaceTampa[2]},
                {barraFaceBase[0], barraFaceBase[1], barraFaceBase[2]},
                {barraFaceDireita[0], barraFaceDireita[1], barraFaceDireita[2]},
                {barraFaceSuperior[0], barraFaceSuperior[1], barraFaceSuperior[2]},
                {barraFaceEsquerda[0], barraFaceEsquerda[1], barraFaceEsquerda[2]},
                {barraFaceInferior[0], barraFaceInferior[1], barraFaceInferior[2]},
            };

            setColor(0.1, 0.9, 0.1);
            glPushMatrix();

            if (vetorBloquinhos[cont].mostra == true)
            {
                glBegin(GL_QUADS);
                CalculaNormal(t[0], &vetorNormal);
                if (verificaColisaoTriangulo(t[0]))
                {
                    refleteBolinha(vetorNormal, 8);
                    vetorBloquinhos[cont].mostra = false;
                }
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceTampa[i].x, barraFaceTampa[i].y, barraFaceTampa[i].z);
                }
                glEnd();

                glBegin(GL_QUADS);
                CalculaNormal(t[1], &vetorNormal);
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                if (verificaColisaoTriangulo(t[1]))
                {
                    refleteBolinha(vetorNormal, 9);
                    vetorBloquinhos[cont].mostra = false;
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceBase[i].x, barraFaceBase[i].y, barraFaceBase[i].z);
                }
                glEnd();

                glBegin(GL_QUADS);
                CalculaNormal(t[2], &vetorNormal);
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                if (verificaColisaoTriangulo(t[2]))
                {
                    refleteBolinha(vetorNormal, 10);
                    vetorBloquinhos[cont].mostra = false;
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceDireita[i].x, barraFaceDireita[i].y, barraFaceDireita[i].z);
                }
                glEnd();

                glBegin(GL_QUADS);
                CalculaNormal(t[3], &vetorNormal);
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                if (verificaColisaoTriangulo(t[3]))
                {
                    refleteBolinha(vetorNormal, 11);
                    vetorBloquinhos[cont].mostra = false;
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceSuperior[i].x, barraFaceSuperior[i].y, barraFaceSuperior[i].z);
                }
                glEnd();

                glBegin(GL_QUADS);
                CalculaNormal(t[4], &vetorNormal);
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                if (verificaColisaoTriangulo(t[4]))
                {
                    refleteBolinha(vetorNormal, 12);
                    vetorBloquinhos[cont].mostra = false;
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceEsquerda[i].x, barraFaceEsquerda[i].y, barraFaceEsquerda[i].z);
                }
                glEnd();

                glBegin(GL_QUADS);
                CalculaNormal(t[5], &vetorNormal);
                glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
                if (verificaColisaoTriangulo(t[5]))
                {
                    refleteBolinha(vetorNormal, 13);
                    vetorBloquinhos[cont].mostra = false;
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3f(barraFaceInferior[i].x, barraFaceInferior[i].y, barraFaceInferior[i].z);
                }
                glEnd();
            }

            glPopMatrix();

            colunas++;
            cont++;

            inicio += 0.40;
            fim += 0.40;
        }
        linhas++;
        inicioh += -0.225;
        fimh += -0.225;
    }
}

void desenhaVidas()
{
    setColor(1.0, 0.0, 0.0);
    glTranslatef(-0.95, 1.3, 0.25);
    for (int i = 0; i < vidasRestantes; i++)
    {
        glutSolidSphere(0.08, 20, 20);
        glTranslatef(0.5, 0, 0);
    }
}

void desenhaBolinha()
{
    if (!GameOver())
    {
        setColor(1.0, 0.5, 0.1);
        glPushMatrix();
        glTranslatef(xBolinha, yBolinha, 0.125);
        glutSolidSphere(0.0625, 20, 20);
        glPopMatrix();
    }
}

void desenhaSeta()
{
    if (desenhaSetaControle)
    {
        vetorSeta->v1.x = xSeta;
        atualizaVetorSeta();
        glPushMatrix();
        setColor(0.0, 1.0, 0.3);
        glBegin(GL_LINES);
        glVertex3f(xBolinha, yBolinha, 0.125);
        glVertex3f(xBolinha + vetorSeta->v1.x, yBolinha + vetorSeta->v1.y, 0.125);
        glEnd();
        glPopMatrix();
    }
}

void desenhaBarrasObjViewer()
{
    float inicio_largura = -0.55;
    float fim_largura = -0.25;
    int cont = 0;

    while (cont < 2)
    {

        vertice vetorNormal;
        vertice barraFaceTampa[4] = {{fim_largura, 0.95, 0.125},
            {inicio_largura, 0.95, 0.125},
            {inicio_largura, 0.99, 0.125},
            {fim_largura, 0.99, 0.125}
        };

        vertice barraFaceBase[4] = {{fim_largura, 0.95, 0.0625},
            {fim_largura, 0.99, 0.0625},
            {inicio_largura, 0.99, 0.0625},
            {inicio_largura, 0.95, 0.0625}
        };

        vertice barraFaceDireita[4] = {{fim_largura, 0.95, 0.0625},
            {fim_largura, 0.99, 0.0625},
            {fim_largura, 0.99, 0.125},
            {fim_largura, 0.95, 0.125}
        };

        vertice barraFaceSuperior[4] = {{inicio_largura, 0.95, 0.0625},
            {inicio_largura, 0.95, 0.125},
            {fim_largura, 0.95, 0.125},
            {fim_largura, 0.95, 0.0625}
        };

        vertice barraFaceEsquerda[4] = {{inicio_largura, 0.95, 0.0625},
            {inicio_largura, 0.99, 0.0625},
            {inicio_largura, 0.99, 0.125},
            {inicio_largura, 0.95, 0.125}
        };

        vertice barraFaceInferior[4] = {{-0.55, 0.99, 0.0625},
            {-0.55, 0.99, 0.125},
            {-0.55, 0.99, 0.125},
            {-0.55, 0.99, 0.0625}
        };

        triangle t[6] = {{barraFaceTampa[0], barraFaceTampa[1], barraFaceTampa[2]},
            {barraFaceBase[0], barraFaceBase[1], barraFaceBase[2]},
            {barraFaceDireita[0], barraFaceDireita[1], barraFaceDireita[2]},
            {barraFaceSuperior[0], barraFaceSuperior[1], barraFaceSuperior[2]},
            {barraFaceEsquerda[0], barraFaceEsquerda[1], barraFaceEsquerda[2]},
            {barraFaceInferior[0], barraFaceInferior[1], barraFaceInferior[2]}
        };

        setColor(1.0, 3.0, 1.0);
        glPushMatrix();
        glBegin(GL_QUADS);
        CalculaNormal(t[5], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceInferior[i].x, barraFaceInferior[i].y, barraFaceInferior[i].z);
        }

        glBegin(GL_QUADS);
        CalculaNormal(t[0], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceTampa[i].x, barraFaceTampa[i].y, barraFaceTampa[i].z);
        }
        glEnd();

        glBegin(GL_QUADS);
        CalculaNormal(t[1], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceBase[i].x, barraFaceBase[i].y, barraFaceBase[i].z);
        }
        glEnd();

        glBegin(GL_QUADS);
        CalculaNormal(t[2], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceDireita[i].x, barraFaceDireita[i].y, barraFaceDireita[i].z);
        }
        glEnd();

        glBegin(GL_QUADS);
        CalculaNormal(t[3], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceSuperior[i].x, barraFaceSuperior[i].y, barraFaceSuperior[i].z);
        }
        glEnd();

        glBegin(GL_QUADS);
        CalculaNormal(t[4], &vetorNormal);
        glNormal3f(vetorNormal.x, vetorNormal.y, vetorNormal.z);
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(barraFaceEsquerda[i].x, barraFaceEsquerda[i].y, barraFaceEsquerda[i].z);
        }
        glEnd();
        glPopMatrix();

        inicio_largura += 0.8;
        fim_largura += 0.8;
        cont++;
    }
}

void caiObjetoEsq()
{
    if (!pausado)
    {
        if (caiObjEsq >= 1.3)
        {
            caiObjEsq = 0;
            slideXEsq = 0.0;
        }
        else
        {
            caiObjEsq += 0.005;
        }
    }
}

void caiObjetoDir()
{
    if (!pausado)
    {
        if (caiObjDir >= 1.3)
        {
            caiObjDir = 0;
            slideXDir = 0.0;
        }
        else
        {

            caiObjDir += 0.002;
        }
    }
}

void caiObjetoEsquerda(int time)
{
    glutPostRedisplay();
    glPushMatrix();
    if (projecao_ortogonal)
    {
        glTranslated(-0.40 - slideXEsq, 0.7 - caiObjEsq, 0.0625);
    }
    else
        glTranslated(-0.40 - slideXEsq, 0.7 - caiObjEsq, 0.0625);
    if (efeitoEsq)
    {
        if (caiObjEsq < 1.3)
        {
            slideXEsq += 0.1;
        }
        efeitoEsq = false;
    }
    //textureManager->Bind(13);
    objectManager->SelectObject(6);
    objectManager->SetShadingMode(SMOOTH_SHADING); // Possible values: FLAT_SHADING e SMOOTH_SHADING
    objectManager->SetRenderMode(USE_MATERIAL);    // Possible values: USE_COLOR, USE_MATERIAL, USE_TEXTURE (not available in this example)
    objectManager->Unitize();
    objectManager->Draw();
    glPopMatrix();
}

void caiObjetoDireita(int time)
{
    glPushMatrix();
    if (projecao_ortogonal)
    {
        glTranslated(0.40 + slideXDir, 0.7 - caiObjDir, 0.0625);
    }
    else
        glTranslated(0.40 + slideXDir, 0.7 - caiObjDir, 0.0625);
    if (efeitoDir)
    {
        if (caiObjDir < 1.3)
        {
            slideXDir += 0.1;
        }
        efeitoDir = false;
    }
    objectManager->SelectObject(6);
    objectManager->SetShadingMode(SMOOTH_SHADING); // Possible values: FLAT_SHADING e SMOOTH_SHADING
    objectManager->SetRenderMode(USE_MATERIAL);    // Possible values: USE_COLOR, USE_MATERIAL, USE_TEXTURE (not available in this example)
    objectManager->Unitize();
    objectManager->Draw();
    glPopMatrix();
}

void verificaColisaoObj()
{
    float posicaoYDoObjEsq = 0.7 - caiObjEsq;
    float posicaoYDoObjDir = 0.7 - caiObjDir;

    if ((xBolinha < -0.37) && (xBolinha > -0.43) && (fabs(yBolinha - posicaoYDoObjEsq) < 0.2))
    {
        //xBolinha *= -1;
        efeitoEsq = true;
    }

    if ((xBolinha < 0.43) && (xBolinha > 0.37) && (fabs(yBolinha - posicaoYDoObjDir) < 0.2))
    {
        //xBolinha *= -1;
        efeitoDir = true;
    }
}

void desenhaTelaInicial()
{
    setColor(1.0, 0.0, 0.0);
    glPushMatrix();
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1, -1, 0.625);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, -1, 0.625);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.625);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1, 1.0, 0.625);

    glEnd();
    glPopMatrix();
}

void drawObject()
{
    desenhaPlataforma();
    desenhaRebatedor();
    desenhaBarrinhasDeBater();
    desenhaBarrasObjViewer();

    desenhaBolinha();
    desenhaSeta();
    desenhaVidas();
    passaDeFase();
}

void loadSkyBox()
{
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);

    glRotatef(50.0, 1.0, 0.0, 0.0);

    // Render the front quad
    textureManager->Bind(7);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(  5.0f, -5.0f, -5.0f );
    glTexCoord2f(1, 0);
    glVertex3f( -5.0f, -5.0f, -5.0f );
    glTexCoord2f(1, 1);
    glVertex3f( -5.0f,  5.0f, -5.0f );
    glTexCoord2f(0, 1);
    glVertex3f(  5.0f,  5.0f, -5.0f );
    glEnd();

    // Render the left quad
    textureManager->Bind(8);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(  5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 0);
    glVertex3f(  5.0f, -5.0f, -5.0f );
    glTexCoord2f(1, 1);
    glVertex3f(  5.0f,  5.0f, -5.0f );
    glTexCoord2f(0, 1);
    glVertex3f(  5.0f,  5.0f,  5.0f );
    glEnd();

    // Render the back quad
    textureManager->Bind(5);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f( -5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 0);
    glVertex3f(  5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 1);
    glVertex3f(  5.0f,  5.0f,  5.0f );
    glTexCoord2f(0, 1);
    glVertex3f( -5.0f,  5.0f,  5.0f );

    glEnd();

    // Render the right quad
    textureManager->Bind(9);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f( -5.0f, -5.0f, -5.0f );
    glTexCoord2f(1, 0);
    glVertex3f( -5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 1);
    glVertex3f( -5.0f,  5.0f,  5.0f );
    glTexCoord2f(0, 1);
    glVertex3f( -5.0f,  5.0f, -5.0f );
    glEnd();

    // Render the top quad
    textureManager->Bind(10);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f( -5.0f,  5.0f, -5.0f );
    glTexCoord2f(0, 0);
    glVertex3f( -5.0f,  5.0f,  5.0f );
    glTexCoord2f(1, 0);
    glVertex3f(  5.0f,  5.0f,  5.0f );
    glTexCoord2f(1, 1);
    glVertex3f(  5.0f,  5.0f, -5.0f );
    glEnd();

    // Render the bottom quad
    textureManager->Bind(6);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f( -5.0f, -5.0f, -5.0f );
    glTexCoord2f(0, 1);
    glVertex3f( -5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 1);
    glVertex3f(  5.0f, -5.0f,  5.0f );
    glTexCoord2f(1, 0);
    glVertex3f(  5.0f, -5.0f, -5.0f );
    glEnd();

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float w = width;
    float h = height;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ortho = 3;

    if(!initialState)
    {

        if (!projecao_ortogonal)
        {
            glOrtho(-ortho, ortho, -ortho * h / w, ortho * h / w, -100.0, 100.0);
        }
        else
            gluPerspective(40.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f); // Calculate The Aspect Ratio Of The Window

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (projecao_ortogonal)
        {
            gluLookAt(0.0, -2.5, 2.8, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


            glPushMatrix();
            if (pausado && rotacaoLiberada)
            {
                glRotatef(rotationY, 0.0, 1.0, 0.0);
                rotacaoY = rotationY;
                glRotatef(rotationX, 1.0, 0.0, 0.0);
                rotacaoX = rotationX;
            }
            else
            {
                glRotatef(rotacaoY, 0.0, 1.0, 0.0);
                glRotatef(rotacaoX, 1.0, 0.0, 0.0);
            }

            loadSkyBox();

            drawObject();


            glPopMatrix();
        }

        if (!projecao_ortogonal)
        {
            gluLookAt(0.0, 0.0, zdist, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

            glPushMatrix();
            glRotatef(0, 0.0, 1.0, 0.0);
            glRotatef(0, 1.0, 0.0, 0.0);
            drawObject();
            glPopMatrix();
        }

        caiObjetoDireita(2);
        caiObjetoEsquerda(2);

        glutSwapBuffers();
        caiObjetoEsq();
        caiObjetoDir();
        moveBolinha();
        verificaColisaoObj();
        //reflexaoBloquinhos();
    }

    else
    {
        textureManager->Bind(1);
        gluPerspective(40.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);
        gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glPushMatrix();
        desenhaTelaInicial();
        glPopMatrix();
        glutSwapBuffers();
    }

    textureManager->Disable();
}

void idle()
{
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    width = w;
    height = h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.01, 200.0);
}

void keyboard(unsigned char key, int x, int y)
{

    switch (tolower(key))
    {

    case 27:
        exit(0);
        break;
    case 32:
        if (pausado)
        {
            pausado = false;
            podeMoverABolinha = false;
        }
        else
        {
            pausado = true;
        }
        break;
    case 'p':
        if (projecao_ortogonal)
        {
            projecao_ortogonal = false;
        }
        else
        {
            projecao_ortogonal = true;
        }
        break;
    case 'c':
        if (projecao_ortogonal && pausado)
        {
            if (rotacaoLiberada)
            {
                rotacaoLiberada = false;
            }
            else
            {
                rotacaoLiberada = true;
            }
            break;
        case 'r':
            restart();
            break;
        }
    }
}

void specialKeys(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_F12:
        janela++;
        if (janela % 2 == 0)
        {
            glutReshapeWindow(1000, 600);
            glutPositionWindow(100, 100);
        }
        else
        {

            glutFullScreen();
        }
        break;
    }
    glutPostRedisplay();
}

// Motion callback
void motion(int x, int y)
{
    if (pausado)
    {
        if (projecao_ortogonal && rotacaoLiberada)
        {
            rotationX += (float)(y - last_y);
            rotationY += (float)(x - last_x);

            last_x = x;
            last_y = y;
        }
    }
}

void motionBarra(int x, int y)
{
    if(!initialState)
    {
        float posicaoX = (float)x / 250 - 2;

        if (!pausado)
        {
            if (posicaoX < 0.80 && posicaoX > -0.80)
            {
                xBarra = posicaoX;

                if (!primeiroLancamento)
                {
                    xBolinha = posicaoX;
                }
            }
        }
    }
}

// Mouse callback
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        last_x = x;
        last_y = y;
    }

    if (button == 3) // Scroll up
    {
        if (xSeta <= 0.40)
        {
            xSeta += 0.05;
            atualizaVetorSeta();
        }
    }
    if (button == 4) // Scroll Down
    {
        if (xSeta >= -0.40)
        {
            xSeta -= 0.05;
            atualizaVetorSeta();
        }
    }


    if (!pausado)
    {
        if (!primeiroLancamento)
        {
            if(initialState)
            {
                if(button == GLUT_LEFT_BUTTON)
                {
                    initialState = false;
                }
            }
            else
            {
                if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
                {
                    desenhaSetaControle = false;
                    vetorMovimentoBolinha.v1.x = vetorSeta->v1.x / 70;
                    vetorMovimentoBolinha.v1.y = vetorSeta->v1.y / 70;
                    primeiroLancamento = true;
                }
            }

        }
    }

}

void init(void)
{
    initLight(width, height); // Fun��o extra para tratar ilumina��o.

    /*glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    GLfloat light0_position[] = {-3.0f, 3.0f, 10.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,0.0f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);*/
    // LOAD OBJECTS
    objectManager = new glcWavefrontObject();
    objectManager->SetNumberOfObjects(NUM_OBJECTS);
    for(int i = 0; i < NUM_OBJECTS; i++)
    {
        objectManager->SelectObject(i);
        objectManager->ReadObject(objectFiles[i]);
        objectManager->Unitize();
        objectManager->FacetNormal();
        objectManager->VertexNormals(90.0);
        objectManager->Scale(5);
    }

    preencheVetorBarrinhas();

    textureManager = new glcTexture();            // Criação do arquivo que irá gerenciar as texturas
    textureManager->SetNumberOfTextures(14);       // Estabelece o número de texturas que será utilizado
    textureManager->CreateTexture("../data/wood.png", 0); // Para testar magnificação, usar a imagem marble128
    textureManager->CreateTexture("../data/arkanoidd.png", 1);
    textureManager->CreateTexture("../data/paper.png", 3);
    textureManager->CreateTexture("../data/brick-2.png", 2);
    textureManager->CreateTexture("../data/basketball.png", 4);
    textureManager->CreateTexture("../data/back.png", 5);
    textureManager->CreateTexture("../data/bottom.png", 6);
    textureManager->CreateTexture("../data/front.png", 7);
    textureManager->CreateTexture("../data/left.png", 8);
    textureManager->CreateTexture("../data/right.png", 9);
    textureManager->CreateTexture("../data/top.png", 10);
    textureManager->CreateTexture("../data/sky.png", 11);
    textureManager->CreateTexture("../data/sky2.png", 12);
    textureManager->CreateTexture("../data/skybarriga.png", 13);

}

/// Main
int main(int argc, char **argv)
{
    for (int i = 0; i < 15; i++)
    {
        vetorBloquinhos[i].mostra = false;
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motionBarra);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
