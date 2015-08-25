/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livreGUI/tfEditor/TransferFunctionEditor.h>
#include <livreGUI/ui_TransferFunctionEditor.h>
#include <livreGUI/tfEditor/Utilities.h>
#include <livreGUI/tfEditor/HoverPoints.h>

#include <zeq/event.h>
#include <zeq/hbp/vocabulary.h>

#include <QMessageBox>

namespace livre
{

TransferFunctionEditor::TransferFunctionEditor( livre::Controller& controller,
                                                QWidget* tfParentWidget )
    : QWidget( tfParentWidget )
    , _controller( controller )
    , ui( new Ui::TransferFunctionEditor )
    , _isConnected( false )
    , _tfReceived( false )
    , _redWidget( new ColorMapWidget( ColorMapWidget::RED_SHADE, this ))
    , _greenWidget( new ColorMapWidget( ColorMapWidget::GREEN_SHADE, this ))
    , _blueWidget( new ColorMapWidget( ColorMapWidget::BLUE_SHADE, this ))
    , _alphaWidget( new ColorMapWidget( ColorMapWidget::ARGB_SHADE, this ))
    , _gradientRenderer( new GradientRenderer( this ))
{
    qRegisterMetaType< UInt8Vector >("UInt8Vector");

    ui->setupUi( this );

    // Add the widgets to the layouts to match the exact positions on the TransferFunctionEditor.
    ui->redLayout->addWidget( _redWidget );
    ui->greenLayout->addWidget( _greenWidget );
    ui->blueLayout->addWidget( _blueWidget );
    ui->rgbaLayout->addWidget( _alphaWidget );
    ui->rgbGradientLayout->addWidget( _gradientRenderer );

    connect( _redWidget, SIGNAL( colorsChanged()),
            this, SLOT( _pointsUpdated()));
    connect( _greenWidget, SIGNAL( colorsChanged()),
            this, SLOT( _pointsUpdated()));
    connect( _blueWidget, SIGNAL( colorsChanged()),
            this, SLOT( _pointsUpdated()));
    connect( _alphaWidget, SIGNAL( colorsChanged()),
            this, SLOT( _pointsUpdated()));

    connect( ui->resetButton, SIGNAL( clicked()), this, SLOT( _setDefault()));
    connect( ui->clearButton, SIGNAL( clicked()), this, SLOT( _clear()));
    connect( ui->btnConnect, SIGNAL( pressed()), this, SLOT( _connect( )));
    connect( ui->btnDisconnect, SIGNAL( pressed()), this, SLOT( _disconnect()));

    connect( this, &TransferFunctionEditor::transferFunctionChanged,
             this, &TransferFunctionEditor::_onTransferFunctionChanged );

    QTimer::singleShot( 50, this, SLOT( _setDefault()));
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
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
        if ( i + 1 < allPoints.size() && xPoint == allPoints.at(i + 1).x())
            continue;

        const int red = _redWidget->getColorAtPoint( xPoint );
        const int green = _greenWidget->getColorAtPoint( xPoint );
        const int blue = _blueWidget->getColorAtPoint( xPoint );
        const int alpha =_alphaWidget->getColorAtPoint( xPoint );
        QColor color((0x00ff0000 & red) >> 16,      // R (16)
                     (0x0000ff00 & green) >> 8,     // G (8)
                     (0x000000ff & blue),           // B (1)
                     (0xff000000 & alpha) >> 24);   // A (24)

        // Outlier
        if ( xPoint / alphaWidgetWidth > 1 )
            return;

        stops << QGradientStop( xPoint / alphaWidgetWidth, color );
    }

    _publishTransferFunction();

    _alphaWidget->setGradientStops( stops );
    _gradientRenderer->setGradientStops( stops );
}

void TransferFunctionEditor::setColorMapStops( const QGradientStops& stops )
{
    QPolygonF redPoints, greenPoints, bluePoints, alphaPoints;

    const double redCMHt = _redWidget->height();
    const double greenCMHt = _greenWidget->height();
    const double blueCMHt = _blueWidget->height();
    const double alphaCMHt = _alphaWidget->height();

    for( int32_t i = 0; i < stops.size(); ++i )
    {
        double position = stops.at( i ).first;
        QRgb color = stops.at( i ).second.rgba();

        redPoints << QPointF( position * _redWidget->width(),
                             redCMHt - qRed(color) * redCMHt / 255 );
        greenPoints << QPointF( position * _greenWidget->width(),
                               greenCMHt - qGreen(color) * greenCMHt / 255 );
        bluePoints << QPointF( position * _blueWidget->width(),
                              blueCMHt - qBlue(color) * blueCMHt / 255 );
        alphaPoints << QPointF( position * _alphaWidget->width(),
                               alphaCMHt - qAlpha(color) * alphaCMHt / 255 );
    }

    _redWidget->setPoints( redPoints );
    _greenWidget->setPoints( greenPoints );
    _blueWidget->setPoints( bluePoints );
    _alphaWidget->setPoints( alphaPoints );
}

void TransferFunctionEditor::_setDefault()
{
    QGradientStops stops;

    stops << QGradientStop( 0.0, QColor::fromRgba( 0x000000ff ));
    stops << QGradientStop( 0.1, QColor::fromRgba( 0x330000ff ));
    stops << QGradientStop( 0.2, QColor::fromRgba( 0x53007dff ));
    stops << QGradientStop( 0.3, QColor::fromRgba( 0x7300ffff ));
    stops << QGradientStop( 0.4, QColor::fromRgba( 0x7f00ff7d ));
    stops << QGradientStop( 0.5, QColor::fromRgba( 0x8500ff00 ));
    stops << QGradientStop( 0.6, QColor::fromRgba( 0x86ffff00 ));
    stops << QGradientStop( 0.7, QColor::fromRgba( 0x8cff7d00 ));
    stops << QGradientStop( 0.8, QColor::fromRgba( 0x99ff0000 ));
    stops << QGradientStop( 0.925, QColor::fromRgba( 0xb3ff007d ));
    stops << QGradientStop( 1.00, QColor::fromRgba( 0xffff7dff ));

    setColorMapStops( stops );
    _gradientRenderer->setGradientStops( stops );
    _pointsUpdated();
}

void TransferFunctionEditor::_publishTransferFunction()
{
    const UInt8Vector& redCurve =  _redWidget->getCurve();
    const UInt8Vector& greenCurve =  _greenWidget->getCurve();
    const UInt8Vector& blueCurve =  _blueWidget->getCurve();
    const UInt8Vector& alphaCurve =  _alphaWidget->getCurve();

    if( redCurve.empty()
            || greenCurve.empty()
            || blueCurve.empty()
            || alphaCurve.empty())
    {
        return;
    }

    UInt8Vector transferFunction;
    transferFunction.reserve( redCurve.size( ) * 4u );
    for( uint32_t i = 0; i < redCurve.size(); ++i )
    {
        transferFunction.push_back( redCurve[i] );
        transferFunction.push_back( greenCurve[i] );
        transferFunction.push_back( blueCurve[i] );
        transferFunction.push_back( alphaCurve[i] );
    }

    if( _isConnected && transferFunction.size() == 1024 )
    {
        const servus::URI uri( ui->txtURL->text().toStdString());
        _controller.publish( uri,
                             zeq::hbp::serializeLookupTable1D( transferFunction ));
    }
}

bool TransferFunctionEditor::_requestTransferFunction()
{
    const QString& uriStr = ui->txtURL->text();
    const servus::URI uri( uriStr.toStdString( ));
    _controller.registerHandler( uri,
                                 zeq::vocabulary::EVENT_EXIT,
                                 boost::bind( &TransferFunctionEditor::_disconnect,
                                              this ));
    _controller.registerHandler( uri,
                                 zeq::hbp::EVENT_LOOKUPTABLE1D,
                                 boost::bind( &TransferFunctionEditor::_receiveTransferFunction,
                                              this, _1 ));
    _controller.registerHandler( uri,
                                 zeq::vocabulary::EVENT_HEARTBEAT,
                                 boost::bind( &TransferFunctionEditor::_onHeartbeat,
                                              this ));

    _isConnected.timedWaitEQ( true, 2000 /*ms*/ );
    return _isConnected;
}

void TransferFunctionEditor::_receiveTransferFunction( const zeq::Event& tfEvent )
{
    if( !_tfReceived )
        emit transferFunctionChanged( zeq::hbp::deserializeLookupTable1D( tfEvent ));
    _tfReceived = true;
}

void TransferFunctionEditor::_onHeartbeat()
{
    if( !_tfReceived )
    {
        const QString& uriStr = ui->txtURL->text();
        const servus::URI uri( uriStr.toStdString( ));
        const zeq::Event& zeqEvent =
            zeq::vocabulary::serializeRequest( zeq::hbp::EVENT_LOOKUPTABLE1D );
        _controller.publish( uri, zeqEvent );
    }
    _isConnected = true;
}

void TransferFunctionEditor::_clear()
{
    QGradientStops stops;

    stops << QGradientStop( 0.00, QColor::fromRgba( 0 ));
    stops << QGradientStop( 1.00, QColor::fromRgba( 0xffffffff ));

    setColorMapStops( stops );
    _gradientRenderer->setGradientStops( stops );
    _pointsUpdated();
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
    qDebug() << convertedPoints;
    return convertedPoints;
}
}

void TransferFunctionEditor::_onTransferFunctionChanged( UInt8Vector tf )
{
    QGradientStops stops;
    QPolygon redPoints, bluePoints, greenPoints, alphaPoints;
    for( size_t i = 0; i < 256; ++i )
    {
        redPoints << QPoint(i, tf[i*4]);
        greenPoints << QPoint(i, tf[i*4+1]);
        bluePoints << QPoint(i, tf[i*4+2]);
        alphaPoints << QPoint(i, tf[i*4+3]);
        stops << QGradientStop( float(i) / 255.f , QColor( tf[i*4], tf[i*4+1], tf[i*4+2], tf[i*4+3]));
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
    _gradientRenderer->setGradientStops( stops );

    _enableWidget( true );
}

void TransferFunctionEditor::_connect()
{
    try
    {
        _enableWidget( false );
        if( _requestTransferFunction())
        {
            ui->btnConnect->setEnabled( false );
            ui->btnDisconnect->setEnabled( true );
        }
        else
            _enableWidget( true );
    }
    catch( const std::exception& error )
    {
        ui->btnConnect->setEnabled( true );
        ui->btnDisconnect->setEnabled( false );
        LBERROR << "Error:" << error.what() << std::endl;
        _isConnected = false;
    }
}

void TransferFunctionEditor::_disconnect()
{
    const QString& uriStr = ui->txtURL->text();
    const servus::URI uri( uriStr.toStdString( ));
    _controller.deregisterHandler( uri,
                                   zeq::vocabulary::EVENT_HEARTBEAT );
    _controller.deregisterHandler( uri,
                                   zeq::vocabulary::EVENT_EXIT );
    _controller.deregisterHandler( uri,
                                   zeq::hbp::EVENT_LOOKUPTABLE1D );

    _isConnected = false;
    _tfReceived = false;
    ui->btnConnect->setEnabled( true );
    ui->btnDisconnect->setEnabled( false );
}

void TransferFunctionEditor::_enableWidget( bool enable )
{
    _redWidget->setEnabled( enable );
    _greenWidget->setEnabled( enable );
    _blueWidget->setEnabled( enable );
    _alphaWidget->setEnabled( enable );
    _gradientRenderer->setEnabled( enable );
}

}
