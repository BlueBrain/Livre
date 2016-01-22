/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
 *                          Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livreGUI/transferFunctionEditor/ColorMapWidget.h>
#include <livreGUI/transferFunctionEditor/HoverPoints.h>
#include <livreGUI/transferFunctionEditor/TransferFunctionEditor.h>
#include <livreGUI/transferFunctionEditor/Utilities.h>
#include <livreGUI/ui_TransferFunctionEditor.h>
#include <livreGUI/Controller.h>

#include <QMessageBox>

namespace livre
{

TransferFunctionEditor::TransferFunctionEditor( livre::Controller& controller,
                                                QWidget* tfParentWidget )
    : QWidget( tfParentWidget )
    , _controller( controller )
    , _ui( new Ui::TransferFunctionEditor )
    , _redWidget( new ColorMapWidget( ColorMapWidget::RED_SHADE, this ))
    , _greenWidget( new ColorMapWidget( ColorMapWidget::GREEN_SHADE, this ))
    , _blueWidget( new ColorMapWidget( ColorMapWidget::BLUE_SHADE, this ))
    , _alphaWidget( new ColorMapWidget( ColorMapWidget::ARGB_SHADE, this ))
{
    qRegisterMetaType< UInt8s >("UInt8s");

    _ui->setupUi( this );

    // Add the widgets to the layouts to match the exact positions on the
    // TransferFunctionEditor
    _ui->redLayout->addWidget( _redWidget );
    _ui->greenLayout->addWidget( _greenWidget );
    _ui->blueLayout->addWidget( _blueWidget );
    _ui->rgbaLayout->addWidget( _alphaWidget );

    connect( _redWidget, SIGNAL( colorsChanged( )), this,
             SLOT( _pointsUpdated()));
    connect( _greenWidget, SIGNAL( colorsChanged( )), this,
             SLOT( _pointsUpdated( )));
    connect( _blueWidget, SIGNAL( colorsChanged( )), this,
             SLOT( _pointsUpdated( )));
    connect( _alphaWidget, SIGNAL( colorsChanged( )), this,
             SLOT( _pointsUpdated( )));

    connect( _ui->resetButton, SIGNAL( clicked()), this, SLOT( _setDefault()));
    connect( _ui->clearButton, SIGNAL( clicked()), this, SLOT( _clear()));
    connect( _ui->loadButton, SIGNAL( clicked()), this, SLOT( _load()));
    connect( _ui->saveButton, SIGNAL( clicked()), this, SLOT( _save()));

    connect( this, &TransferFunctionEditor::transferFunctionChanged,
             this, &TransferFunctionEditor::_onTransferFunctionChanged );

    QTimer::singleShot( 50, this, SLOT( _setDefault()));

    _controller.subscribe( _lut );
    _lut.setUpdatedFunction(
        std::bind( &TransferFunctionEditor::_onTransferFunction, this ));
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    _controller.unsubscribe( _lut );
    delete _ui;
}

void TransferFunctionEditor::_pointsUpdated()
{
    const double alphaWidgetWidth = _alphaWidget->width();
    QGradientStops stops;
    QPolygonF redPoints, greenPoints, bluePoints, alphaPoints, allPoints;

    redPoints = _redWidget->getPoints();
    greenPoints = _greenWidget->getPoints();
    bluePoints = _blueWidget->getPoints();
    alphaPoints = _alphaWidget->getPoints();

    std::sort( redPoints.begin(), redPoints.end(), xLessThan );
    std::sort( greenPoints.begin(), greenPoints.end(), xLessThan );
    std::sort( bluePoints.begin(), bluePoints.end(), xLessThan );
    std::sort( alphaPoints.begin(), alphaPoints.end(), xLessThan );

    allPoints = redPoints;
    allPoints += greenPoints;
    allPoints += bluePoints;
    allPoints += alphaPoints;
    std::sort( allPoints.begin(), allPoints.end(), xLessThan );

    for( int32_t i = 0; i < allPoints.size(); ++i )
    {
        const int xPoint = int( allPoints.at(i).x());
        if( i + 1 < allPoints.size() && xPoint == allPoints.at(i + 1).x( ))
            continue;

        const int red = _redWidget->getColorAtPoint( xPoint );
        const int green = _greenWidget->getColorAtPoint( xPoint );
        const int blue = _blueWidget->getColorAtPoint( xPoint );
        const int alpha = _alphaWidget->getColorAtPoint( xPoint );
        QColor color((0x00ff0000 & red) >> 16,      // R (16)
                     (0x0000ff00 & green) >> 8,     // G (8)
                     (0x000000ff & blue),           // B (1)
                     (0xff000000 & alpha) >> 24);   // A (24)

        // Outlier
        if( xPoint / alphaWidgetWidth > 1 )
            return;

        stops << QGradientStop( xPoint / alphaWidgetWidth, color );
    }

    _publishTransferFunction();
    _alphaWidget->setGradientStops( stops );
}

void TransferFunctionEditor::_setDefault()
{
    QPolygonF points;
    double h = _redWidget->height();
    double w = _redWidget->width();
    points << QPointF( 0.0 * w, h );
    points << QPointF( 0.4 * w, h );
    points << QPointF( 0.6 * w, 0.0 );
    points << QPointF( 1.0 * w, 0.0 );
    _redWidget->setPoints( points );

    h = _greenWidget->height();
    w = _greenWidget->width();
    points.clear();
    points << QPointF( 0.0 * w, h );
    points << QPointF( 0.2 * w, 0.0 );
    points << QPointF( 0.6 * w, 0.0 );
    points << QPointF( 0.8 * w, h );
    points << QPointF( 1.0 * w, h );
    _greenWidget->setPoints( points );

    h = _blueWidget->height();
    w = _blueWidget->width();
    points.clear();
    points << QPointF( 0.0 * w, 0.0 );
    points << QPointF( 0.2 * w, 0.0 );
    points << QPointF( 0.4 * w, h );
    points << QPointF( 0.8 * w, h );
    points << QPointF( 1.0 * w, 0.0 );
    _blueWidget->setPoints( points );

    h = _alphaWidget->height();
    w = _alphaWidget->width();
    points.clear();
    points << QPointF( 0.0 * w, h );
    points << QPointF( 0.1 * w, 0.8 * h );
    points << QPointF( 1.0 * w, 0.1 * h );
    _alphaWidget->setPoints( points );

    _pointsUpdated();
}

void TransferFunctionEditor::_publishTransferFunction()
{
    const UInt8s& redCurve =  _redWidget->getCurve();
    const UInt8s& greenCurve =  _greenWidget->getCurve();
    const UInt8s& blueCurve =  _blueWidget->getCurve();
    const UInt8s& alphaCurve =  _alphaWidget->getCurve();

    if( redCurve.empty() || greenCurve.empty() || blueCurve.empty() ||
        alphaCurve.empty() || redCurve.size() * 4 != _lut.getLutSize( ))
    {
        return;
    }

    uint8_t* lut = _lut.getLut();
    for( uint32_t i = 0; i < redCurve.size(); ++i )
    {
        lut[ 4*i + 0 ] = redCurve[i];
        lut[ 4*i + 1 ] = greenCurve[i];
        lut[ 4*i + 2 ] = blueCurve[i];
        lut[ 4*i + 3 ] = alphaCurve[i];
    }

    _controller.publish( _lut );
}

void TransferFunctionEditor::_onTransferFunction()
{
    emit transferFunctionChanged();
    _lut.setUpdatedFunction( servus::Serializable::ChangeFunc( ));
}

void TransferFunctionEditor::_clear()
{
    QPolygonF points;
    const double h = _redWidget->height();
    const double w = _redWidget->width();
    points << QPointF( 0.0 * w, h );
    points << QPointF( 1.0 * w, 0.0 );

    _redWidget->setPoints( points );
    _greenWidget->setPoints( points );
    _blueWidget->setPoints( points );
    _alphaWidget->setPoints( points );
    _pointsUpdated();
}

const quint32 TF_FILE_HEADER = 0xdeadbeef;
const quint32 TF_FILE_VERSION = 1;
const QString TF_FILE_FILTER( "Transfer function files (*.tf)" );

void TransferFunctionEditor::_load()
{
    const QString filename = QFileDialog::getOpenFileName( this, "Load transfer function",
                                                           QString(),
                                                           TF_FILE_FILTER );
    if( filename.isEmpty( ))
        return;

    QFile file( filename );
    file.open( QIODevice::ReadOnly );
    QDataStream in( &file );

    quint32 header;
    in >> header;
    if( header != TF_FILE_HEADER )
        return;

    quint32 version;
    in >> version;
    if( version != TF_FILE_VERSION )
        return;

    QPolygonF redPoints, greenPoints, bluePoints, alphaPoints;
    QGradientStops gradientStops;
    in >> redPoints
       >> greenPoints
       >> bluePoints
       >> alphaPoints;

    _redWidget->setPoints( redPoints );
    _greenWidget->setPoints( greenPoints );
    _blueWidget->setPoints( bluePoints );
    _alphaWidget->setPoints( alphaPoints );
    _pointsUpdated();
}

void TransferFunctionEditor::_save()
{
    QString filename = QFileDialog::getSaveFileName( this, "Save transfer function",
                                                           QString(),
                                                           TF_FILE_FILTER );
    if( filename.isEmpty( ))
        return;

    if( !filename.endsWith( ".tf" ))
        filename.append( ".tf" );

    QFile file( filename );
    file.open( QIODevice::WriteOnly );
    QDataStream out( &file );
    out.setVersion( QDataStream::Qt_5_0 );

    out << TF_FILE_HEADER << TF_FILE_VERSION;
    out << _redWidget->getPoints()
        << _greenWidget->getPoints()
        << _blueWidget->getPoints()
        << _alphaWidget->getPoints();
}

namespace
{
QPolygon _filterPoints( const QPolygon& points )
{
    QPolygon filteredPoints;
    float prevSlope = 0;
    QPoint prevPoint = points.first();
    for( int i = 1; i < points.size() - 1; ++i )
    {
        const QPoint& currentPoint = points[i];
        const QLine currentLine( prevPoint, currentPoint );
        const float currentSlope = float(currentLine.dy()) / float(currentLine.dx());

        bool change = std::abs(prevSlope - currentSlope) > std::numeric_limits<float>::epsilon();
        if( change )
        {
            const QLine nextLine( currentPoint, points[i+1] );
            const float nextSlope = float(nextLine.dy()) / float(nextLine.dx());
            if( std::abs(prevSlope - nextSlope) <= std::numeric_limits<float>::epsilon() )
                change = false;
        }

        if( change || i == 1 )
        {
            prevSlope = currentSlope;
            filteredPoints << prevPoint;
        }
        prevPoint = points[i];
    }
    filteredPoints << points.last();
    return filteredPoints;
}

QPolygonF _convertPoints( const QPolygon& points, const int width, const int height )
{
    QPolygonF convertedPoints;
    for( int32_t i = 0; i < points.size(); ++i )
    {
        const int position = points.at( i ).x();
        const int value = points.at( i ).y();

        convertedPoints << QPointF( (position/255.f) * width,
                             height - value * height / 255 );
    }
    return convertedPoints;
}
}

void TransferFunctionEditor::_onTransferFunctionChanged()
{
    QGradientStops stops;
    QPolygon redPoints, bluePoints, greenPoints, alphaPoints;
    const uint8_t* lut = _lut.getLut();
    for( size_t i = 0; i < _lut.getLutSize(); i += 4 )
    {
        redPoints << QPoint(i, lut[i+0]);
        greenPoints << QPoint(i, lut[i+1]);
        bluePoints << QPoint(i, lut[i+2]);
        alphaPoints << QPoint(i, lut[i+3]);
        stops << QGradientStop( float(i/4) / 255.f,
                                QColor( lut[i+0], lut[i+1], lut[i+2], lut[i+3]));
    }

    QPolygonF redPointsF = _convertPoints( _filterPoints( redPoints ),
                                           _redWidget->width(),
                                           _redWidget->height());
    QPolygonF greenPointsF = _convertPoints( _filterPoints( greenPoints ),
                                             _greenWidget->width(),
                                             _greenWidget->height());
    QPolygonF bluePointsF = _convertPoints( _filterPoints( bluePoints ),
                                            _blueWidget->width(),
                                            _blueWidget->height());
    QPolygonF alphaPointsF = _convertPoints( _filterPoints( alphaPoints ),
                                             _alphaWidget->width(),
                                             _alphaWidget->height());

    _redWidget->setPoints( redPointsF );
    _greenWidget->setPoints( greenPointsF );
    _blueWidget->setPoints( bluePointsF );
    _alphaWidget->setPoints( alphaPointsF );
    _pointsUpdated();
}

}
