/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
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

#include "TransferFunctionEditor.h"
#include "ColorMapWidget.h"

#include <livreGUI/ui_TransferFunctionEditor.h>
#include <livreGUI/Controller.h>

#include <lexis/render/lookupTable1D.h>

#include <QFileDialog>

#include <cmath>
#include <fstream>

namespace livre
{
namespace
{
const ColorMapWidget::Channel channels[] =
                            { ColorMapWidget::Channel::red,
                              ColorMapWidget::Channel::green,
                              ColorMapWidget::Channel::blue,
                              ColorMapWidget::Channel::alpha };

}

TransferFunctionEditor::TransferFunctionEditor( livre::Controller& controller,
                                                QWidget* tfParentWidget )
    : QWidget( tfParentWidget )
    , _controller( controller )
    , _ui( new Ui::TransferFunctionEditor )
{
    _ui->setupUi( this );

    for( const auto& channel: channels )
    {
        _colorWidgets[ (size_t)channel ] = new ColorMapWidget( this, channel  );
        connect( _colorWidgets[ (size_t)channel  ], SIGNAL( colorsChanged( )),
                this, SIGNAL( transferFunctionChanged( )));
    }

    connect( _colorWidgets[ (size_t)ColorMapWidget::Channel::alpha ],
             SIGNAL( histIndexChanged(size_t,double)),
             this, SLOT( _onHistIndexChanged(size_t,double)));

    // Add the widgets to the layouts to match the exact positions on the
    // TransferFunctionEditor
    _ui->redLayout->addWidget(
                _colorWidgets[ (size_t)ColorMapWidget::Channel::red ]);
    _ui->greenLayout->addWidget(
                _colorWidgets[ (size_t)ColorMapWidget::Channel::green ]);
    _ui->blueLayout->addWidget(
                _colorWidgets[ (size_t)ColorMapWidget::Channel::blue ]);
    _ui->rgbaLayout->addWidget(
                _colorWidgets[ (size_t)ColorMapWidget::Channel::alpha ]);

    connect( _ui->histogramScaleCheckBox, SIGNAL( clicked( bool )), this,
             SLOT( _onHistogramChanged( bool )));

    connect( _ui->resetButton, SIGNAL( clicked( )), this, SLOT( _setDefault()));
    connect( _ui->clearButton, SIGNAL( clicked( )), this, SLOT( _clear()));
    connect( _ui->loadButton, SIGNAL( clicked() ), this, SLOT( _load()));
    connect( _ui->saveButton, SIGNAL( clicked( )), this, SLOT( _save()));

    connect( this, &TransferFunctionEditor::transferFunctionChanged,
             this, &TransferFunctionEditor::_onTransferFunctionChanged, Qt::QueuedConnection );

    connect( this, &TransferFunctionEditor::histogramChanged,
             this, &TransferFunctionEditor::_onHistogramChanged, Qt::QueuedConnection );

    _controller.subscribe( _histogram );
    _histogram.registerDeserializedCallback( [&]
        { emit histogramChanged( _ui->histogramScaleCheckBox->checkState() == Qt::Checked ); });

    _setDefault();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    _controller.unsubscribe( _histogram );
    delete _ui;
}

void TransferFunctionEditor::_setGradientStops()
{
    ControlPoints allControlPoints( compareControlPoints );
    for( auto& widget: _colorWidgets )
    {
        const auto& controlPoints = widget->getControlPoints();
        allControlPoints.insert( controlPoints.begin(), controlPoints.end());
    }

    QGradientStops stops;
    constexpr size_t nChannels = 4;
    for( const auto& point : allControlPoints )
    {
        int colors[ nChannels ] = { 0 };
        for( size_t j = 0; j < nChannels; ++j )
            colors[ j ] = _colorWidgets[ j ]->getColorAtPoint( point.x() );

        QColor color((0x00ff0000 & colors[ 0 ]) >> 16,  // R (16)
                     (0x0000ff00 & colors[ 1 ]) >> 8,   // G (8)
                     (0x000000ff & colors[ 2 ]),        // B (1)
                     (0xff000000 & colors[ 3 ]) >> 24); // A (24)

        stops << QGradientStop( point.x(), color );
    }

    _colorWidgets[ (size_t)ColorMapWidget::Channel::alpha ]->setGradientStops( stops );
}

void TransferFunctionEditor::_setDefault()
{
    for( auto& widget: _colorWidgets )
    {
        QPolygonF points;
        switch( widget->getChannel())
        {
        case ColorMapWidget::Channel::red:
        case ColorMapWidget::Channel::green:
            points << QPointF( 0.0, 0.0 );
            points << QPointF( 0.4, 0.0 );
            points << QPointF( 0.6, 1.0 );
            points << QPointF( 1.0, 1.0 );
            break;
        case ColorMapWidget::Channel::blue:
            points << QPointF( 0.0, 0.0 );
            points << QPointF( 0.2, 1.0 );
            points << QPointF( 0.6, 1.0 );
            points << QPointF( 0.8, 0.0 );
            points << QPointF( 1.0, 0.0 );
            break;
        case ColorMapWidget::Channel::alpha:
            points << QPointF( 0.0, 0.0 );
            points << QPointF( 0.1, 0.2 );
            points << QPointF( 1.0, 0.8 );
            break;
        }
        widget->setControlPoints( points );
    }

    emit transferFunctionChanged();
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
    lexis::render::LookupTable1D lut;

    size_t i = 0;
    for( const auto& widget: _colorWidgets )
    {
        const UInt8s& curve = widget->getCurve();
        if( curve.empty() || curve.size() * 4 != lut.getLutSize( ))
            return;

        for( uint32_t j = 0; j < lut.getLutSize() / 4; ++j )
            lut.getLut()[ 4 * j + i ] = curve[ j ];
        ++i;
    }

    _controller.publish( lut );
}

void TransferFunctionEditor::_clear()
{
    QPolygonF points;
    points.push_back( { 0, 0 } );
    points.push_back( { 1, 1 } );
    for( const auto& channel: channels )
        _colorWidgets[ (size_t)channel ]->setControlPoints( points );

    _colorWidgets[ (size_t)ColorMapWidget::Channel::alpha ]->setHistogram( lexis::render::Histogram(), false );
    emit transferFunctionChanged();
}

namespace
{
const quint32 TF_FILE_HEADER = 0xdeadbeef;
const quint32 TF_FILE_VERSION_1 = 1; // stores control points absolutely
                                     // considering widget geometry (sigh)
const quint32 TF_FILE_VERSION_2 = 2; // stores control points normalized
const QString TF_FILE_FILTER( "Transfer function's control points, *.tf" );
const QString DT_FILE_FILTER( "ImageVis3d compatible ascii, *.1dt" );
const QString TF_FILTERS( TF_FILE_FILTER + ";;" + DT_FILE_FILTER );

QPolygonF _filterPoints( const QPolygonF& points )
{
    QPolygonF filteredPoints;
    float prevSlope = 0;
    QPointF prevPoint = points.first();
    const float epsilon = 0.001f;
    for( int i = 1; i < points.size() - 1; ++i )
    {
        const auto& currentPoint = points[i];
        const QLineF currentLine( prevPoint, currentPoint );
        const float currentSlope = float(currentLine.dy()) / float(currentLine.dx());

        bool change = std::abs(prevSlope - currentSlope) > epsilon;
        if( change )
        {
            const QLineF nextLine( currentPoint, points[ i + 1 ] );
            const float nextSlope = float(nextLine.dy()) / float(nextLine.dx());
            if( std::abs(prevSlope - nextSlope) <= epsilon )
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
        if( version != TF_FILE_VERSION_1 && version != TF_FILE_VERSION_2 )
            return;

        for( auto& widget: _colorWidgets )
        {
            QPolygonF points;
            in >> points;
            if( version == TF_FILE_VERSION_1 )
            {
                for( auto& point : points )
                {
                    point.setX( point.x() / widget->width( ));
                    point.setY( 1.f - (point.y() / widget->height( )));
                }
            }
            widget->setControlPoints( points );
        }
    }
    else if( selectedFilter == DT_FILE_FILTER )
    {
        std::ifstream file( filename.toStdString( ));
        uint32_t samples;
        file >> samples;
        if( samples != COLORSAMPLES )
            return;

        constexpr size_t nChannels = 4;
        QPolygonF points[nChannels];
        for( size_t i = 0; i < nChannels; ++i )
            points[i].resize( samples );

        for( size_t i = 0; i < samples; ++i )
        {
            for( size_t j = 0; j < nChannels; ++j )
            {
                points[j][i].rx() = i/float(samples-1);
                file >> points[j][i].ry();
            }
        }

        for( size_t i = 0; i < nChannels; ++i )
            _colorWidgets[i]->setControlPoints( _filterPoints( points[i] ));
    }
    else
        return;

    emit transferFunctionChanged();
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

        out << TF_FILE_HEADER << TF_FILE_VERSION_2;

        for( const auto& widget : _colorWidgets )
            out << widget->getControlPoints();
    }
    else if( selectedFilter == DT_FILE_FILTER )
    {
        if( !filename.endsWith( ".1dt" ))
            filename.append( ".1dt" );

        std::ofstream file;
        file.open( filename.toStdString( ));
        file << COLORSAMPLES << std::endl;

        const auto& reds = _colorWidgets[0]->getCurve();
        const auto& greens = _colorWidgets[1]->getCurve();
        const auto& blues = _colorWidgets[2]->getCurve();
        const auto& alphas = _colorWidgets[3]->getCurve();

        for( size_t i = 0; i < COLORSAMPLES; ++i )
            file << reds[i] / 255.f << " " << greens[i] / 255.f << " "
                 << blues[i] / 255.f << " " << alphas[i] / 255.f << std::endl;
    }
}

void TransferFunctionEditor::_onTransferFunctionChanged()
{
    _setGradientStops();
    _publishTransferFunction();
}

void TransferFunctionEditor::_onHistogramChanged( const bool logScale )
{
    _colorWidgets[ (size_t)ColorMapWidget::Channel::alpha  ]->setHistogram( _histogram, logScale );
}

}
