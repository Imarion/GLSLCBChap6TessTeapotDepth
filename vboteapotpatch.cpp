#include "vboteapotpatch.h"
#include "teapotdata.h"

#include <cstdio>

#include <QVector3D>

#include <QMatrix3x3>

VBOTeapotPatch::VBOTeapotPatch()
{
    nVerts = 32 * 16;
    v = new float[ nVerts * 3 ];

    generatePatches(v);
/*
    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    unsigned int handle;
    glGenBuffers(1, &handle);    

    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), v, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray(0);  // Vertex position    

    glBindVertexArray(0);
    */
}

VBOTeapotPatch::~VBOTeapotPatch()
{
    delete[] v;
}

void VBOTeapotPatch::generatePatches(float * v) {
    int idx = 0;

    // Build each patch
    // The rim
    buildPatchReflect(0, v, idx, true, true);
    // The body
    buildPatchReflect(1, v, idx, true, true);
    buildPatchReflect(2, v, idx, true, true);
    // The lid
    buildPatchReflect(3, v, idx, true, true);
    buildPatchReflect(4, v, idx, true, true);
    // The bottom
    buildPatchReflect(5, v, idx, true, true);
    // The handle
    buildPatchReflect(6, v, idx, false, true);
    buildPatchReflect(7, v, idx, false, true);
    // The spout
    buildPatchReflect(8, v, idx, false, true);
    buildPatchReflect(9, v, idx, false, true);
}

void VBOTeapotPatch::buildPatchReflect(int patchNum,
           float *v, int &index, bool reflectX, bool reflectY)
{
    QVector3D patch[4][4];
    QVector3D patchRevV[4][4];
    getPatch(patchNum, patch, false);
    getPatch(patchNum, patchRevV, true);

    // Patch without modification
    buildPatch(patchRevV, v, index, QMatrix3x3());

    float matxdata[9] = {
        -1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f};

    // Patch reflected in x
    if( reflectX ) {
        buildPatch(patch, v,
                   index, QMatrix3x3(matxdata));
    }

    // Patch reflected in y
    float matydata[9] = {
        1.0f,  0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f,  0.0f, 1.0f
    };
    if( reflectY ) {
        buildPatch(patch, v,
                   index, QMatrix3x3(matydata));
    }

    // Patch reflected in x and y
    float matxydata[9] = {
        -1.0f,  0.0f, 0.0f,
         0.0f, -1.0f, 0.0f,
         0.0f,  0.0f, 1.0f
     };
    if( reflectX && reflectY ) {
        buildPatch(patchRevV, v,
                   index, QMatrix3x3(matxydata));
    }
}

void VBOTeapotPatch::buildPatch(QVector3D patch[][4],
                           float *v, int &index, QMatrix3x3 reflect )
{
    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0 ; j < 4; j++)
        {
            QVector3D pt = mattimesvec(reflect, patch[i][j]);

            v[index] = pt.x();
            v[index+1] = pt.y();
            v[index+2] = pt.z();

            index += 3;
        }
    }
}

QVector3D VBOTeapotPatch::mattimesvec(QMatrix3x3 inmat, QVector3D invec)
{

    QGenericMatrix<3,3,float> m1;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            m1(r, c) = inmat(r, c);

    float data[3];
    data[0] = invec.x();
    data[1] = invec.y();
    data[2] = invec.z();

    QGenericMatrix<1,3,float> m2(data);

    QGenericMatrix<1,3,float> result = m1 * m2;

    return QVector3D(result(0,0), result(1,0), result(2,0));
}

void VBOTeapotPatch::getPatch( int patchNum, QVector3D patch[][4], bool reverseV )
{
    for( int u = 0; u < 4; u++) {          // Loop in u direction
        for( int v = 0; v < 4; v++ ) {     // Loop in v direction
            if( reverseV ) {
                patch[u][v] = QVector3D(
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+(3-v)]][0],
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+(3-v)]][1],
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+(3-v)]][2]
                        );
            } else {
                patch[u][v] = QVector3D(
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+v]][0],
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+v]][1],
                        TeapotData::cpdata[TeapotData::patchdata[patchNum][u*4+v]][2]
                        );
            }
        }
    }
}

int   VBOTeapotPatch::getnVerts() {
    return nVerts;
}

float *VBOTeapotPatch::getv()
{
    return v;
}

/*
void VBOTeapotPatch::render() const {
    glPatchParameteri(GL_PATCH_VERTICES, 16);

    glBindVertexArray(vaoHandle);
    glDrawArrays(GL_PATCHES, 0, 512);
}
*/
