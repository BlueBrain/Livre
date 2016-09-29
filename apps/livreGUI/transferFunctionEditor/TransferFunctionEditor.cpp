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
#include <livreGUI/ui_TransferFunctionEditor.h>
#include <livreGUI/Controller.h>

#include <lunchbox/file.h>

#include <QMessageBox>

#include <fstream>

namespace livre
{
namespace
{
const size_t nChannels = 4;
const lexis::render::ColorMap::Channel channels[] =
                            { lexis::render::ColorMap::Channel::red,
                              lexis::render::ColorMap::Channel::green,
                              lexis::render::ColorMap::Channel::blue,
                              lexis::render::ColorMap::Channel::alpha };

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
        _colorWidgets[ (size_t)channel ] = new ColorMapWidget( this, _colorMap, channel  );
        connect( _colorWidgets[ (size_t)channel  ], SIGNAL( colorsChanged( )),
                this, SLOT( _onColorsChanged()));
    }

    connect( _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::alpha ],
             SIGNAL( histIndexChanged(size_t,double)),
             this, SLOT( _onHistIndexChanged(size_t,double)));

    // Add the widgets to the layouts to match the exact positions on the
    // TransferFunctionEditor
    _ui->redLayout->addWidget(
                _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::red ]);
    _ui->greenLayout->addWidget(
                _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::green ]);
    _ui->blueLayout->addWidget(
                _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::blue ]);
    _ui->rgbaLayout->addWidget(
                _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::alpha ]);

    connect( _ui->histogramScaleCheckBox, SIGNAL( stateChanged( int )), this,
             SLOT( _onScaleChanged( int )));

    connect( _ui->resetButton, SIGNAL( clicked( )), this, SLOT( _setDefault()));
    connect( _ui->clearButton, SIGNAL( clicked( )), this, SLOT( _clear()));
    connect( _ui->loadButton, SIGNAL( clicked() ), this, SLOT( _load()));
    connect( _ui->saveButton, SIGNAL( clicked( )), this, SLOT( _save()));

    connect( this, &TransferFunctionEditor::transferFunctionChanged,
             this, &TransferFunctionEditor::_onTransferFunctionChanged );

    connect( this, &TransferFunctionEditor::histogramChanged,
             this, &TransferFunctionEditor::_onHistogramChanged );

    QTimer::singleShot( 50, this, SLOT( _setDefault()));

    _controller.subscribe( _colorMap );
    _controller.subscribe( _histogram );
    _colorMap.registerDeserializedCallback( [&]
        { return _onTransferFunction(); });
    _histogram.registerDeserializedCallback( [&]
        { return emit histogramChanged(); });
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    _controller.unsubscribe( _colorMap );
    _controller.unsubscribe( _histogram );
    delete _ui;
}

void TransferFunctionEditor::_setGradientStops()
{
    QGradientStops stops;
    const auto& colors = _colorMap.sampleColors< uint8_t >( 256 );
    float xPos = 0.0f;
    const float diff = width() / 256.0f;
    for( const auto& color: colors )
    {
        const QColor col( color.r, color.g, color.b, color.a );
        stops << QGradientStop( xPos / width(), col );
        xPos += diff;
    }

    _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::alpha ]->setGradientStops( stops );
}

void TransferFunctionEditor::_setHistogram()
{
    const bool isLogScale = _ui->histogramScaleCheckBox->checkState() == Qt::Checked;
    _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::alpha  ]->setHistogram( _histogram,
                                                                                     isLogScale );
}

void TransferFunctionEditor::_widgetsUpdated()
{
    for( auto& widget: _colorWidgets )
        widget->update();
}

void TransferFunctionEditor::_setDefault()
{
    _colorMap = lexis::render::ColorMap::getDefaultColorMap( 0.0f, 256.0f );
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
    _controller.publish( _colorMap );
}

void TransferFunctionEditor::_onTransferFunction()
{
    emit transferFunctionChanged();
    _colorMap.registerDeserializedCallback( nullptr );
}

void TransferFunctionEditor::_clear()
{
    _colorMap.clear();

    for( const auto& channel: channels )
    {
        _colorMap.addControlPoint( { 0.0f, 0.0f }, channel );
        _colorMap.addControlPoint( { 255.0f, 1.0f }, channel );
    }

    _colorWidgets[ (size_t)lexis::render::ColorMap::Channel::alpha ]->setHistogram( Histogram(),
                                                                                    false );
    _onColorsChanged();
    _widgetsUpdated();
    _publishTransferFunction();
}

namespace
{
const QString LBA_FILE_FILTER( "Transfer function's values ascii, *.lba" );
const QString LBB_FILE_FILTER( "Transfer function's values binary, *.lbb" );
const QString TF_FILTERS( LBA_FILE_FILTER + ";;" + LBB_FILE_FILTER );
}
void TransferFunctionEditor::_load()
{
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName( this, "Load transfer function",
                                                     QString(),
                                                     TF_FILTERS, &selectedFilter );
    if( filename.isEmpty( ))
        return;

    lexis::render::ColorMap colorMap;

    if( selectedFilter == LBA_FILE_FILTER )
    {
        if( !filename.endsWith( ".lba" ))
            filename.append( ".lba" );

        lunchbox::loadAscii( colorMap, filename.toStdString( ));
    }
    else if( selectedFilter == LBB_FILE_FILTER )
    {
        if( !filename.endsWith( ".lbb" ))
            filename.append( ".lbb" );

        lunchbox::loadBinary( colorMap, filename.toStdString( ));
    }

    _colorMap = colorMap;
    _onTransferFunction();
    _onColorsChanged();
}

void TransferFunctionEditor::_save()
{
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName( this, "Save transfer function",
                                                           QString(),
                                                           TF_FILTERS, &selectedFilter );

    if( selectedFilter == LBA_FILE_FILTER )
    {
        if( !filename.endsWith( ".lba" ))
            filename.append( ".lba" );

        lunchbox::saveAscii( _colorMap, filename.toStdString( ));
    }
    else if( selectedFilter == LBB_FILE_FILTER )
    {
        if( !filename.endsWith( ".lbb" ))
            filename.append( ".lbb" );

        lunchbox::saveBinary( _colorMap, filename.toStdString( ));
    }
}

void TransferFunctionEditor::_onTransferFunctionChanged()
{
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
