
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "glwidget.h"

#include <alg/types/box.h>
#include <alg/volumeTree.h>
#include <alg/orderEstimator.h>
#include <alg/cameraParameters.h>

#include <math.h>

#include <QtGui>
#include <QtOpenGL>
#include <QDebug>


using namespace massVolVis;


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    , _near(   1.0 )
    , _far(  1000.0 )
    , _width(  300 )
    , _height( 300 )
{
    resetRotation();
    resetTranslation();

    _qtGreen  = QColor::fromCmykF( 0.40, 0.0,  1.0, 0.0 );
    _qtPurple = QColor::fromCmykF( 0.39, 0.39, 0.0, 0.0 );

    _volTree = new VolumeTree( Vec3_ui16(256,256,256), 128 );

    for( int i = 0; i < 10; ++i )
    {
        Stat * stat = _volTree->getNodeStat( 0 );
        if( stat )
            stat->loaded = true;
    }

}


GLWidget::~GLWidget()
{
    delete _volTree;
}


void GLWidget::resetRotation( )
{
    _rotation    = Vector3d( 0, 0, 0 );
}


void GLWidget::resetTranslation( )
{
    _translation = Vector3d( 0, 0, -600 );
}


QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}


QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}


static void _normalizeRotation( Vector3d& v )
{
    for( int i = 0; i < 3; ++i )
    {
        while( v[i] < 0.0 ) v[i] += 360.0;
        while( v[i] > 360 ) v[i] -= 360.0;
    }
}

void GLWidget::rotate( const Vector3d v )
{
    if( v == Vector3d( 0, 0, 0 ))
        return;

    _rotation += v;
    _normalizeRotation( _rotation );

    updateGL();
}


void GLWidget::translate( const Vector3d v )
{
    if( v == Vector3d( 0, 0, 0 ))
        return;

    _translation += v;

    updateGL();
}


void GLWidget::initializeGL()
{
    qglClearColor( _qtPurple.dark( ));

    // init GL data

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

//    glClearDepth( 1.f );

    glEnable( GL_CULL_FACE   );
//    glShadeModel( GL_SMOOTH  );
    glEnable( GL_LIGHTING    );
    glEnable( GL_LIGHT0      );
 //   glEnable( GL_MULTISAMPLE );

    glEnable(    GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}


static void _grawBox( const QColor& color )
{
    // render six axis-aligned colored quads around the origin
    glColor4f( color.redF(), color.greenF(), color.blueF(), color.alphaF() );

    const float s =  0.99;
    const float d = -0.99;
    for( int j = 0; j < 2; ++j )
    {
        glCullFace( j == 0 ? GL_BACK : GL_FRONT );

        for( int i = 0; i < 6; ++i )
        {
            glNormal3f( 0.0f, 0.0f, j == 0 ? -1.0f : 1.0f );
            glBegin( GL_TRIANGLE_STRIP );
                glVertex3f(  s,  s, d );
                glVertex3f( -s,  s, d );
                glVertex3f(  s, -s, d );
                glVertex3f( -s, -s, d );
            glEnd();

            if( i < 3 )
                glRotatef(  90.0f, 0.0f, 1.0f, 0.0f );
            else if( i == 3 )
                glRotatef(  90.0f, 1.0f, 0.0f, 0.0f );
            else
                glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
        }
    }
}

static void _grawBox( const Box_i32& box, const QColor& color )
{
    // render six axis-aligned colored quads around the origin
    glColor4f( color.redF(), color.greenF(), color.blueF(), color.alphaF() );

    for( int j = 0; j < 2; ++j )
    {
        glCullFace( j == 0 ? GL_FRONT : GL_BACK );

        const float norm = (j == 0 ? -1.0f : 1.0f);
//
// Cube verticies layout:
//
//    7   5
//  1   3
//
//    6   4
//  0   2
//
        glBegin( GL_QUADS );
            glNormal3f(  norm, 0.0f, 0.0f );
            glVertex3f( box.s.x, box.s.y, box.s.z ); // 0
            glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
            glVertex3f( box.s.x, box.e.y, box.e.z ); // 3
            glVertex3f( box.s.x, box.e.y, box.s.z ); // 2

            glNormal3f( 0.0f,  -norm, 0.0f );
            glVertex3f( box.s.x, box.e.y, box.s.z ); // 2
            glVertex3f( box.s.x, box.e.y, box.e.z ); // 3
            glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
            glVertex3f( box.e.x, box.e.y, box.s.z ); // 4

            glNormal3f( -norm, 0.0f, 0.0f );
            glVertex3f( box.e.x, box.e.y, box.s.z ); // 4
            glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
            glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
            glVertex3f( box.e.x, box.s.y, box.s.z ); // 6

            glNormal3f( 0.0f,  norm, 0.0f );
            glVertex3f( box.e.x, box.s.y, box.s.z ); // 6
            glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
            glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
            glVertex3f( box.s.x, box.s.y, box.s.z ); // 0

            glNormal3f( 0.0f, 0.0f, -norm );
            glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
            glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
            glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
            glVertex3f( box.s.x, box.e.y, box.e.z ); // 3

            glNormal3f( 0.0f, 0.0f,  norm );
            glVertex3f( box.s.x, box.s.y, box.s.z ); // 0
            glVertex3f( box.s.x, box.e.y, box.s.z ); // 2
            glVertex3f( box.e.x, box.e.y, box.s.z ); // 4
            glVertex3f( box.e.x, box.s.y, box.s.z ); // 6
        glEnd();
    }
}

static void _drawAxiss( const float size )
{
    const float ls  = size / 40.f;
    const float ls2 = ls*2.f;
    glDisable( GL_LIGHTING    );

    glBegin( GL_LINES );
        // X
        glColor3f(   1.f,  0.f,  0.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f( size,  0.f,  0.f );

        glVertex3f( size+ls-ls, -ls,  0.f );
        glVertex3f( size+ls+ls,  ls,  0.f );
        glVertex3f( size+ls-ls,  ls,  0.f );
        glVertex3f( size+ls+ls, -ls,  0.f );

        // Y
        glColor3f(   0.f,  1.f,  0.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f(  0.f, size,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(   ls, size+ls2+ls,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(  -ls, size+ls2+ls,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(  0.f, size+ls2-ls,  0.f );

        // Z
        glColor3f(   0.f,  0.f,  1.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f(  0.f,  0.f, size );
    glEnd();
    glBegin( GL_LINE_STRIP );
        glVertex3f( -ls,  ls, size+ls );
        glVertex3f(  ls,  ls, size+ls );
        glVertex3f( -ls, -ls, size+ls );
        glVertex3f(  ls, -ls, size+ls );
    glEnd();

    glEnable( GL_LIGHTING    );


}


void GLWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glLoadIdentity();

    const float lightPos[] = { 0.0f, 0.0f, -1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );


    glTranslatef( _translation.x(), _translation.y(), _translation.z() );
    glRotatef( _rotation.x(), 1.0, 0.0, 0.0 );
    glRotatef( _rotation.y(), 0.0, 1.0, 0.0 );
    glRotatef( _rotation.z(), 0.0, 0.0, 1.0 );

    glTranslatef( -128, -128, -128 );

    // draw

    std::vector< QColor > _colors;
    for( int i = 0; i < 8; ++i )
    {
        const int r =  i&1;
        const int g = (i&2) >> 1;
        const int b = (i&4) >> 2;
        _colors.push_back( QColor( 100+r*155, 100+g*155, 100+b*155, 150 ));
    }

    Matrix4d MV;
    glGetDoublev( GL_MODELVIEW_MATRIX, MV.array );

    CameraParameters cp( MV );
    vec32_t          outPoses;
    Box_i32          box = _volTree->getAbsCoordinates(0);
    OrderEstimator::compute( outPoses, *_volTree, 1, box, cp );

    for( size_t i = 0; i < outPoses.size(); ++i )
    {
        _grawBox( _volTree->getAbsCoordinates( outPoses[i] ), _colors[ outPoses[i] % _colors.size()] );
    }


    std::stringstream ss;
    for( size_t i = 0; i < outPoses.size(); ++i )
        ss << outPoses[i] << " ";

    _drawAxiss( 255 );
//    qDebug() << ss.str().c_str();


/*
    for( int z = -100; z < 100; z+=100 )
    for( int y = -100; y < 100; y+=100 )
    for( int x = -100; x < 100; x+=100 )
    {
        _grawBox( Box_i32( Vec3_ui32(x+5,y+5,z+5), Vec3_ui32(x+95,y+95,z+95) ),
            QColor( 100+(x+155), 100+(y+155), 100+(z+155), 150 ));
    }
*/
}


static void _adjustNearFar( const int w, const int h, const float n, const float f )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    const float a = float( w ) / h;

    const float l = 1.0 * a / 3.0 * n;
    const float t = 1.0     / 3.0 * n;

    glFrustum( -l, l,
               -t, t,
                n, f );

    glMatrixMode( GL_MODELVIEW );
}


void GLWidget::_setNearFar( const float n, const float f )
{
    _adjustNearFar( _width, _height, n, f );
}


void GLWidget::resizeGL( int width, int height )
{
    qDebug() << "resize: " << width << " x " << height;
    glViewport( 0, 0, width, height );

    _width  = width;
    _height = height;

    _setNearFar( _near, _far );
}


void GLWidget::mousePressEvent( QMouseEvent *event )
{
    _lastPos = event->pos();
}


void GLWidget::mouseMoveEvent( QMouseEvent *event )
{
    const int dx = event->x() - _lastPos.x();
    const int dy = event->y() - _lastPos.y();

    if( event->buttons() & Qt::LeftButton )
    {
        rotate( Vector3d( dy, dx, 0 ));
    }else
    if( event->buttons() & Qt::RightButton )
    {
        translate( Vector3d( dx/2.0, -dy/2.0, 0 ));
    }
    _lastPos = event->pos();
}

void GLWidget::wheelEvent( QWheelEvent * event )
{
    translate( Vector3d( 0, 0, event->delta()/10.0 ));

    _lastPos = event->pos();
}

