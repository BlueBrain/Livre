
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "window.h"
#include "glwidget.h"

#include <alg/volumeTree.h>

#include <QtGui>


Window::Window()
{
    _glWidget = new GLWidget;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget( _glWidget );
    setLayout( mainLayout );

    setWindowTitle( tr("TreeVis") );

//    massVolVis::VolumeTree tree( 64300, 65000, 65000, 512 );
}


void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}
