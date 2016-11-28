#include "TessTeapotDepth.h"

#include <QtGlobal>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QTime>

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

#include <cmath>
#include <cstring>

MyWindow::~MyWindow()
{
    if (mProgram != 0) delete mProgram;
    if (mTeapot  != 0) delete mTeapot;
}

MyWindow::MyWindow()
    : mProgram(0), currentTimeMs(0), currentTimeS(0), angle(M_PI / 3.0f), tPrev(0.0f), rotSpeed(M_PI / 8.0f)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
    create();

    resize(800, 600);

    mContext = new QOpenGLContext(this);
    mContext->setFormat(format);
    mContext->create();

    mContext->makeCurrent( this );

    mFuncs = mContext->versionFunctions<QOpenGLFunctions_4_3_Core>();
    if ( !mFuncs )
    {
        qWarning( "Could not obtain OpenGL versions object" );
        exit( 1 );
    }
    if (mFuncs->initializeOpenGLFunctions() == GL_FALSE)
    {
        qWarning( "Could not initialize core open GL functions" );
        exit( 1 );
    }

    initializeOpenGLFunctions();

    QTimer *repaintTimer = new QTimer(this);
    connect(repaintTimer, &QTimer::timeout, this, &MyWindow::render);
    repaintTimer->start(1000/60);

    QTimer *elapsedTimer = new QTimer(this);
    connect(elapsedTimer, &QTimer::timeout, this, &MyWindow::modCurTime);
    elapsedTimer->start(1);       
}

void MyWindow::modCurTime()
{
    currentTimeMs++;
    currentTimeS=currentTimeMs/1000.0f;
}

void MyWindow::initialize()
{
    mFuncs->glGenVertexArrays(1, &mVAO);
    mFuncs->glBindVertexArray(mVAO);

    CreateVertexBuffer();
    initShaders();
    initMatrices();

    mFuncs->glPatchParameteri(GL_PATCH_VERTICES, 16);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
}

void MyWindow::CreateVertexBuffer()
{    
    // *** Teapot
    mFuncs->glGenVertexArrays(1, &mVAOTeapot);
    mFuncs->glBindVertexArray(mVAOTeapot);

    QMatrix4x4 transform;
    //transform.translate(QVector3D(0.0f, 1.5f, 0.25f));
    mTeapot = new VBOTeapotPatch();

    // Create and populate the buffer objects
    unsigned int TeapotHandle;
    glGenBuffers(1, &TeapotHandle);

    glBindBuffer(GL_ARRAY_BUFFER, TeapotHandle);
    glBufferData(GL_ARRAY_BUFFER, (3 * mTeapot->getnVerts()) * sizeof(float), mTeapot->getv(), GL_STATIC_DRAW);

    // Setup the VAO
    // Vertex positions
    mFuncs->glBindVertexBuffer(0, TeapotHandle, 0, sizeof(GLfloat) * 3);
    mFuncs->glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(0, 0);

    mFuncs->glBindVertexArray(0);
}

void MyWindow::initMatrices()
{
    /*
    ViewMatrix.setToIdentity();
    ViewMatrix.lookAt(QVector3D(4.25f * cos(angle), 3.0f, 4.25f * sin(angle)), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    */
    ModelMatrix.translate(0.0f, -1.5f, 0.0f);
    ModelMatrix.rotate(-90.0f, QVector3D(1.0f, 0.0f, 0.0f));
}

void MyWindow::resizeEvent(QResizeEvent *)
{
    mUpdateSize = true;

    ProjectionMatrix.setToIdentity();
    ProjectionMatrix.perspective(60.0f, (float)this->width()/(float)this->height(), 0.3f, 100.0f);
    //float c = 3.5f;
    //ProjectionMatrix.ortho(-0.4f * c, 0.4f * c, -0.3f * c, 0.3f * c, 0.1f, 100.0f);

    float w2 = (float) this->width()  / 2.0f;
    float h2 = (float) this->height() / 2.0f;

    ViewPortMatrix = QMatrix4x4(w2,   0.0f, 0.0f, 0.0f,
                                0.0f, h2,   0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                w2,   h2,   0.0f, 1.0f);

}

void MyWindow::render()
{
    if(!isVisible() || !isExposed())
        return;

    if (!mContext->makeCurrent(this))
        return;

    static bool initialized = false;
    if (!initialized) {
        initialize();
        initialized = true;
    }

    if (mUpdateSize) {
        glViewport(0, 0, size().width(), size().height());
        mUpdateSize = false;
    }

    float deltaT = currentTimeS - tPrev;
    if(tPrev == 0.0f) deltaT = 0.0f;
    tPrev = currentTimeS;
    angle += rotSpeed * deltaT;
    if (angle > TwoPI) angle -= TwoPI;    

    static float EvolvingVal = 0;
    EvolvingVal += 0.1f;

    //angle = M_PI / 3.0f;
    ViewMatrix.setToIdentity();
    ViewMatrix.lookAt(QVector3D(0.0f, 1.0f, 6.25f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    QMatrix4x4 mv1 = ViewMatrix * ModelMatrix;

    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearColor(0.5f,0.5f,0.5f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //QMatrix4x4 RotationMatrix;
    //RotationMatrix.rotate(EvolvingVal, QVector3D(0.1f, 0.0f, 0.1f));    

    mFuncs->glBindVertexArray(mVAOTeapot);

    glEnableVertexAttribArray(0);

    mProgram->bind();
    {
        mProgram->setUniformValue("MinTessLevel",  2);
        mProgram->setUniformValue("MaxTessLevel", 15);
        mProgram->setUniformValue("MaxDepth",     20.0f);
        mProgram->setUniformValue("MinDepth",      2.0f);

        mProgram->setUniformValue("LineWidth", 0.8f);
        mProgram->setUniformValue("LineColor", QVector4D(0.05f,0.0f,0.05f,1.0f));        

        mProgram->setUniformValue("LightPosition",  QVector4D(0.0f,0.0f,0.0f,1.0f));
        mProgram->setUniformValue("LightIntensity", QVector3D(1.0f,1.0f,1.0f));
        mProgram->setUniformValue("Kd", QVector3D(0.9f, 0.9f, 1.0f));

        //1st
        ModelMatrix.setToIdentity();
        ModelMatrix.translate(-2.0f,  0.0f, 0.0f);
        ModelMatrix.translate( 0.0f, -1.5f, 0.0f);
        ModelMatrix.rotate(-90.0f, QVector3D(1.0f,0.0f,0.0f));

        mv1 = ViewMatrix * ModelMatrix;
        mProgram->setUniformValue("ModelViewMatrix", mv1);
        mProgram->setUniformValue("MVP", ProjectionMatrix * mv1);
        mProgram->setUniformValue("ViewportMatrix", ViewPortMatrix);
        mProgram->setUniformValue("NormalMatrix", mv1.normalMatrix());

        glDrawArrays(GL_PATCHES, 0, 512);

        //2nd
        ModelMatrix.setToIdentity();
        ModelMatrix.translate( 2.0f,  0.0f, -5.0f);
        ModelMatrix.translate( 0.0f, -1.5f,  0.0f);
        ModelMatrix.rotate(-90.0f, QVector3D(1.0f,0.0f,0.0f));

        mv1 = ViewMatrix * ModelMatrix;
        mProgram->setUniformValue("ModelViewMatrix", mv1);
        mProgram->setUniformValue("MVP", ProjectionMatrix * mv1);
        mProgram->setUniformValue("ViewportMatrix", ViewPortMatrix);
        mProgram->setUniformValue("NormalMatrix", mv1.normalMatrix());

        glDrawArrays(GL_PATCHES, 0, 512);

        //3rd
        ModelMatrix.setToIdentity();
        ModelMatrix.translate( 7.0f,  0.0f, -10.0f);
        ModelMatrix.translate( 0.0f, -1.5f,   0.0f);
        ModelMatrix.rotate(-90.0f, QVector3D(1.0f,0.0f,0.0f));

        mv1 = ViewMatrix * ModelMatrix;
        mProgram->setUniformValue("ModelViewMatrix", mv1);
        mProgram->setUniformValue("MVP", ProjectionMatrix * mv1);
        mProgram->setUniformValue("ViewportMatrix", ViewPortMatrix);
        mProgram->setUniformValue("NormalMatrix", mv1.normalMatrix());

        glDrawArrays(GL_PATCHES, 0, 512);

        //4th
        ModelMatrix.setToIdentity();
        ModelMatrix.translate( 17.0f,  0.0f, -20.0f);
        ModelMatrix.translate(  0.0f, -1.5f,   0.0f);
        ModelMatrix.rotate(-90.0f, QVector3D(1.0f,0.0f,0.0f));

        mv1 = ViewMatrix * ModelMatrix;
        mProgram->setUniformValue("ModelViewMatrix", mv1);
        mProgram->setUniformValue("MVP", ProjectionMatrix * mv1);
        mProgram->setUniformValue("ViewportMatrix", ViewPortMatrix);
        mProgram->setUniformValue("NormalMatrix", mv1.normalMatrix());

        glDrawArrays(GL_PATCHES, 0, 512);
    }
    mProgram->release();

    glDisableVertexAttribArray(0);

    mContext->swapBuffers(this);
}

void MyWindow::initShaders()
{
    QOpenGLShader vShader(QOpenGLShader::Vertex);
    QOpenGLShader tcsShader(QOpenGLShader::TessellationControl);
    QOpenGLShader tesShader(QOpenGLShader::TessellationEvaluation);
    QOpenGLShader gShader(QOpenGLShader::Geometry);
    QOpenGLShader fShader(QOpenGLShader::Fragment);
    QFile         shaderFile;
    QByteArray    shaderSource;

    shaderFile.setFileName(":/vshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex    compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/tcsshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "tess ctrl compile: " << tcsShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/tesshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "tess eval compile: " << tesShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/gshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "geom      compile: " << gShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag      compile: " << fShader.compileSourceCode(shaderSource);

    mProgram = new (QOpenGLShaderProgram);
    mProgram->addShader(&vShader);
    mProgram->addShader(&tcsShader);
    mProgram->addShader(&tesShader);
    mProgram->addShader(&fShader);
    mProgram->addShader(&gShader);
    qDebug() << "shader link (mProgram): " << mProgram->link();
}

void MyWindow::PrepareTexture(GLenum TextureUnit, GLenum TextureTarget, const QString& FileName, bool flip)
{
    QImage TexImg;

    if (!TexImg.load(FileName)) qDebug() << "Erreur chargement texture " << FileName;
    if (flip==true) TexImg=TexImg.mirrored();

    glActiveTexture(TextureUnit);
    GLuint TexObject;
    glGenTextures(1, &TexObject);
    glBindTexture(TextureTarget, TexObject);
    mFuncs->glTexStorage2D(TextureTarget, 1, GL_RGBA8, TexImg.width(), TexImg.height());
    mFuncs->glTexSubImage2D(TextureTarget, 0, 0, 0, TexImg.width(), TexImg.height(), GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    //glTexImage2D(TextureTarget, 0, GL_RGB, TexImg.width(), TexImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void MyWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_P:
            break;
        case Qt::Key_Up:
            break;
        case Qt::Key_Down:
            break;
        case Qt::Key_Left:
            break;
        case Qt::Key_Right:
            break;
        case Qt::Key_Delete:
            break;
        case Qt::Key_PageDown:
            break;
        case Qt::Key_Home:
            break;
        case Qt::Key_Z:
            break;
        case Qt::Key_Q:
            break;
        case Qt::Key_S:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_A:
            break;
        case Qt::Key_E:
            break;
        default:
            break;
    }
}

void MyWindow::printMatrix(const QMatrix4x4& mat)
{
    const float *locMat = mat.transposed().constData();

    for (int i=0; i<4; i++)
    {
        qDebug() << locMat[i*4] << " " << locMat[i*4+1] << " " << locMat[i*4+2] << " " << locMat[i*4+3];
    }
}
