/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Stefan.Eilemann@epfl.ch
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <livreGUI/AppSettings.h>
#include <livreGUI/Controller.h>
#include <livreGUI/MainWindow.h>

#include <QApplication>
#include <QDesktopWidget>

QT_USE_NAMESPACE

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    Q_INIT_RESOURCE( resources );

    app.setAttribute( Qt::AA_DontCreateNativeWidgetSiblings );
    livre::setCoreSettingsNames( );

    livre::Controller controller;

    MainWindow window( controller );
    window.resize( QDesktopWidget().availableGeometry( &window ).size() / 2.0 );
    window.show();

    return app.exec();
}
