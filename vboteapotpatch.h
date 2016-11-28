#ifndef VBOTEAPOTPATCH_H
#define VBOTEAPOTPATCH_H

#include <QVector3D>

#include <QMatrix3x3>

class VBOTeapotPatch
{
private:
    unsigned int vaoHandle;

    float *v;
    int nVerts;

    void generatePatches(float * v);
    void buildPatchReflect(int patchNum,
                           float *v, int &index,
                           bool reflectX, bool reflectY);
    void buildPatch(QVector3D patch[][4],
                    float *v, int &index, QMatrix3x3 reflect);
    void getPatch( int patchNum, QVector3D patch[][4], bool reverseV );

    QVector3D mattimesvec(QMatrix3x3 inmat, QVector3D invec);

public:
    VBOTeapotPatch();
    ~VBOTeapotPatch();

    float *getv();
    int    getnVerts();

//    void render() const;
};

#endif // VBOTEAPOTPATCH_H
