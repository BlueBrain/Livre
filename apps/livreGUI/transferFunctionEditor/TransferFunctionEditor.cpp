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

#include <lunchbox/file.h>

#include <QMessageBox>

#include <fstream>

namespace livre
{

TransferFunctionEditor::TransferFunctionEditor( livre::Controller& controller,
                                                QWidget* tfParentWidget )
    : QWidget( tfParentWidget )
    , _controller( controller )
    , _ui( new Ui::TransferFunctionEditor )
{
    qRegisterMetaType< UInt8s >("UInt8s");
    _ui->setupUi( this );

    for( size_t i = 0; i < ColorMapWidget::SHADE_COUNT; ++i )
    {
        _colorWidgets[ i ] = new ColorMapWidget( ColorMapWidget::ShadeType( i ), this );
        connect( _colorWidgets[ i ], SIGNAL( colorsChanged( )), this, SLOT( _onColorsChanged()));
    }

    connect( _colorWidgets[ ColorMapWidget::ALPHA_SHADE ], SIGNAL( histIndexChanged(size_t,double)),
             this, SLOT( _onHistIndexChanged(size_t,double)));

    // Add the widgets to the layouts to match the exact positions on the
    // TransferFunctionEditor
    _ui->redLayout->addWidget( _colorWidgets[ ColorMapWidget::RED_SHADE ] );
    _ui->greenLayout->addWidget( _colorWidgets[ ColorMapWidget::GREEN_SHADE ] );
    _ui->blueLayout->addWidget( _colorWidgets[ ColorMapWidget::BLUE_SHADE ] );
    _ui->rgbaLayout->addWidget( _colorWidgets[ ColorMapWidget::ALPHA_SHADE ] );

    connect( _ui->histogramScaleCheckBox, SIGNAL( stateChanged( int )), this,
             SLOT( _onScaleChanged( int )));

    connect( _ui->resetButton, SIGNAL( clicked()), this, SLOT( _setDefault()));
    connect( _ui->clearButton, SIGNAL( clicked()), this, SLOT( _clear()));
    connect( _ui->loadButton, SIGNAL( clicked()), this, SLOT( _load()));
    connect( _ui->saveButton, SIGNAL( clicked()), this, SLOT( _save()));

    connect( this, &TransferFunctionEditor::transferFunctionChanged,
             this, &TransferFunctionEditor::_onTransferFunctionChanged );

    connect( this, &TransferFunctionEditor::histogramChanged,
             this, &TransferFunctionEditor::_onHistogramChanged );

    QTimer::singleShot( 50, this, SLOT( _setDefault()));

    _controller.subscribe( _lut );
    _controller.subscribe( _histogram );
    _lut.registerDeserializedCallback( [&]
        { return _onTransferFunction(); });
    _histogram.registerDeserializedCallback( [&]
        { return emit histogramChanged(); });
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    _controller.unsubscribe( _lut );
    _controller.unsubscribe( _histogram );
    delete _ui;
}

void TransferFunctionEditor::_setGradientStops()
{
    QPolygonF allPoints;
    for( auto& widget: _colorWidgets )
    {
        QPolygonF points = widget->getPoints();
        std::sort( points.begin(), points.end(), xLessThan );
        allPoints += points;
    }
    std::sort( allPoints.begin(), allPoints.end(), xLessThan );

    QGradientStops stops;
    const float colorWidgetWidth = _colorWidgets[ ColorMapWidget::ALPHA_SHADE  ]->width();
    for( int i = 0; i < allPoints.size(); ++i )
    {
        const int xPoint = int( allPoints.at( i ).x());
        if( i + 1 < allPoints.size() && xPoint == allPoints.at( i + 1 ).x( ))
            continue;

        int colors[ ColorMapWidget::SHADE_COUNT ] = { 0 };
        for( size_t j = 0; j < ColorMapWidget::SHADE_COUNT; ++j )
            colors[ j ] = _colorWidgets[ j ]->getColorAtPoint( xPoint );

        QColor color((0x00ff0000 & colors[ 0 ]) >> 16,  // R (16)
                     (0x0000ff00 & colors[ 1 ]) >> 8,   // G (8)
                     (0x000000ff & colors[ 2 ]),        // B (1)
                     (0xff000000 & colors[ 3 ]) >> 24); // A (24)

        // Outlier
        if( xPoint / colorWidgetWidth > 1 )
            return;

        stops << QGradientStop( xPoint / colorWidgetWidth, color );
    }
    _colorWidgets[ ColorMapWidget::ALPHA_SHADE  ]->setGradientStops( stops );
}

void TransferFunctionEditor::_setHistogram()
{
    const bool isLogScale = _ui->histogramScaleCheckBox->checkState() == Qt::Checked;
    _colorWidgets[ ColorMapWidget::ALPHA_SHADE  ]->setHistogram( _histogram, isLogScale );
}

void TransferFunctionEditor::_widgetsUpdated()
{
    for( auto& widget: _colorWidgets )
        widget->update();
}

void TransferFunctionEditor::_setDefault()
{
    for( auto& widget: _colorWidgets )
    {
        const float h = widget->height();
        const float w = widget->width();
        QPolygonF points;
        switch( widget->getShadeType( ))
        {
        case ColorMapWidget::RED_SHADE:
        case ColorMapWidget::GREEN_SHADE:
            points << QPointF( 0.0 * w, h );
            points << QPointF( 0.4 * w, h );
            points << QPointF( 0.6 * w, 0.0 );
            points << QPointF( 1.0 * w, 0.0 );
            break;
        case ColorMapWidget::BLUE_SHADE:
            points << QPointF( 0.0 * w, h );
            points << QPointF( 0.2 * w, 0.0 );
            points << QPointF( 0.6 * w, 0.0 );
            points << QPointF( 0.8 * w, h );
            points << QPointF( 1.0 * w, h );
            break;
        case ColorMapWidget::ALPHA_SHADE:
            points << QPointF( 0.0 * w, h );
            points << QPointF( 0.1 * w, 0.8 * h );
            points << QPointF( 1.0 * w, 0.1 * h );
            break;
        case ColorMapWidget::SHADE_COUNT:
        default:
            break;
        }
        widget->setPoints( points );
    }

    _setHistogram();
    _onColorsChanged();
    _widgetsUpdated();
    _publishTransferFunction();
}

void TransferFunctionEditor::_onColorsChanged()
{
    _setGradientStops();
    _widgetsUpdated();
    _publishTransferFunction();
}

void TransferFunctionEditor::_onHistIndexChanged( size_t index, const double ratio )
{
    const QString indexText = index == -1u ? "" : QString( "%1" ).arg( index, 4 );
    const QString valText = index == -1u ? "" : QString( "%1" ).arg( ratio * 100.0, 4, 'f', 3 );
    _ui->histogramValLabel->setText( valText );
    _ui->histogramIndexLabel->setText( indexText );
}

void TransferFunctionEditor::_publishTransferFunction()
{
    for( const auto& widget: _colorWidgets )
    {
        const UInt8s& curve = widget->getCurve();
        if( curve.empty() || curve.size() * 4 != _lut.getLutSize( ))
            return;
    }

    uint8_t* lut = _lut.getLut();
    for( size_t i = 0; i < ColorMapWidget::SHADE_COUNT; ++i )
    {
        const UInt8s& curve =  _colorWidgets[ i ]->getCurve();
        for( uint32_t j = 0; j < _lut.getLutSize() / ColorMapWidget::SHADE_COUNT; ++j )
            lut[ ColorMapWidget::SHADE_COUNT * j + i ] = curve[ j ];
    }

    _controller.publish( _lut );
}

void TransferFunctionEditor::_onTransferFunction()
{
    emit transferFunctionChanged();
    _lut.registerDeserializedCallback( nullptr );
}

void TransferFunctionEditor::_clear()
{
    const float h = _colorWidgets[ 0 ]->height();
    const float w = _colorWidgets[ 0 ]->width();
    QPolygonF points;

    points << QPointF( 0.0 * w, h );
    points << QPointF( 1.0 * w, 0.0 );

    for( auto& widget: _colorWidgets )
        widget->setPoints( points );

    _colorWidgets[ ColorMapWidget::ALPHA_SHADE ]->setHistogram( Histogram(), false );
    _onColorsChanged();
    _widgetsUpdated();
    _publishTransferFunction();
}

namespace
{
const quint32 TF_FILE_HEADER = 0xdeadbeef;
const quint32 TF_FILE_VERSION = 1;
const QString TF_FILE_FILTER( "Transfer function's control points, *.tf" );
const QString DT_FILE_FILTER( "ImageVis3d compatible ascii, *.1dt" );
const QString LBA_FILE_FILTER( "Transfer function's values ascii, *.lba" );
const QString LBB_FILE_FILTER( "Transfer function's values binary, *.lbb" );
const QString TF_FILTERS( TF_FILE_FILTER + ";;" + DT_FILE_FILTER + ";;" +
                          LBA_FILE_FILTER + ";;" + LBB_FILE_FILTER );
}
void TransferFunctionEditor::_load()
{
    QString selectedFilter;
    const QString filename = QFileDialog::getOpenFileName( this, "Load transfer function",
                                                           QString(),
                                                           TF_FILTERS, &selectedFilter );
    if( filename.isEmpty( ))
        return;

    if( selectedFilter == TF_FILE_FILTER )
    {
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

        for( auto& widget: _colorWidgets )
        {
            QPolygonF points;
            in >> points;
            widget->setPoints( points );
        }

        _setHistogram();
        _widgetsUpdated();
        _publishTransferFunction();
        return;
    }
    else if(( selectedFilter == DT_FILE_FILTER ) ||
            ( selectedFilter == LBA_FILE_FILTER ) ||
            ( selectedFilter == LBB_FILE_FILTER ))
    {
        livre::TransferFunction1D lut( filename.toStdString( ));
        _lut = lut;
        _onTransferFunction();
        return;
    }
}

void TransferFunctionEditor::_save()
{
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName( this, "Save transfer function",
                                                           QString(),
                                                           TF_FILTERS, &selectedFilter );
    if( selectedFilter == TF_FILE_FILTER )
    {
        if( !filename.endsWith( ".tf" ))
            filename.append( ".tf" );

        QFile file( filename );
        file.open( QIODevice::WriteOnly );
        QDataStream out( &file );
        out.setVersion( QDataStream::Qt_5_0 );

        out << TF_FILE_HEADER << TF_FILE_VERSION;

        for( size_t i = 0; i < ColorMapWidget::SHADE_COUNT; ++i )
            out << _colorWidgets[ i ]->getPoints();
    }
    else if( selectedFilter == DT_FILE_FILTER )
    {
        if( !filename.endsWith( ".1dt" ))
            filename.append( ".1dt" );

        std::ofstream file;
        file.open( filename.toStdString( ));
        size_t tfSize = _lut.getLutSize() / ColorMapWidget::SHADE_COUNT;
        file << tfSize << " uint8" << std::endl;

        for( size_t i = 0; i < tfSize; ++i )
        {
            for( const auto& widget: _colorWidgets )
                file << (uint32_t)widget->getCurve()[ i ] << " ";
            file << std::endl;
        }

    }
    else if( selectedFilter == LBA_FILE_FILTER )
    {
        if( !filename.endsWith( ".lba" ))
            filename.append( ".lba" );

        lunchbox::saveAscii( _lut, filename.toStdString( ));
    }
    else if( selectedFilter == LBB_FILE_FILTER )
    {
        if( !filename.endsWith( ".lbb" ))
            filename.append( ".lbb" );

        lunchbox::saveBinary( _lut, filename.toStdString( ));
    }
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
            const QLine nextLine( currentPoint, points[ i + 1 ] );
            const float nextSlope = float(nextLine.dy()) / float(nextLine.dx());
            if( std::abs(prevSlope - nextSlope) <= std::numeric_limits<float>::epsilon( ))
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

    const uint8_t* lut = _lut.getLut();
    for( size_t i = 0; i < ColorMapWidget::SHADE_COUNT; ++i )
    {
        QPolygon points;
        const float h = _colorWidgets[ i ]->height();
        const float w = _colorWidgets[ i ]->width();
        for( size_t j = 0; j < _lut.getLutSize() / ColorMapWidget::SHADE_COUNT; ++j )
            points << QPoint( j, lut[ j * ColorMapWidget::SHADE_COUNT + i ] );

        QPolygonF fPoints = _convertPoints( _filterPoints( points ), w, h );
        _colorWidgets[ i ]->setPoints( fPoints );
    }

    _setHistogram();
    _widgetsUpdated();
    _publishTransferFunction();
}

void TransferFunctionEditor::_onScaleChanged( int )
{
     emit histogramChanged();
}

void TransferFunctionEditor::_onHistogramChanged()
{
    _setGradientStops();
    _setHistogram();
    _widgetsUpdated();
}

}
