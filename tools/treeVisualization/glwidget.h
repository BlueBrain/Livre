
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__GUI_GLWIDGET_H
#define MASS_VOL__GUI_GLWIDGET_H

#include <QGLWidget>

#include <alg/types/algebraTypes.h>


namespace massVolVis
{
    class VolumeTree;
}

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget( QWidget *parent = 0 );
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;


public slots:
    void rotate( const Vector3d v );
    void resetRotation();

    void translate( const Vector3d v );
    void resetTranslation();


protected:
    void initializeGL();
    void paintGL();
    void resizeGL( int width, int height );
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent(  QMouseEvent *event );

    void wheelEvent ( QWheelEvent * event );

private:

    void _setNearFar( const float n, const float f );

    Vector3d _rotation;
    Vector3d _translation;

    float    _near;
    float    _far;
    int      _width;
    int      _height;

    QPoint _lastPos;
    QColor _qtGreen;
    QColor _qtPurple;

    massVolVis::VolumeTree* _volTree;
};


#endif // MASS_VOL__GUI_GLWIDGET_H
