/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Philipp Schlegel <schlegel@ifi.uzh.ch>
 *                          David Steiner <steiner@ifi.uzh.ch>
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

#include <QFileDialog>

#include <livreGUI/editor/graphcore/GaussGraphCore.h>
#include <livreGUI/editor/graphcore/DoubleGaussGraphCore.h>
#include <livreGUI/editor/graphs/AnchorGraph.h>
#include <livreGUI/editor/TransferFunctionEditor.h>

#include <livreGUI/Controller.h>

namespace livre
{

TransferFunctionEditor::TransferFunctionEditor( QWidget* parentWgt )
    : QWidget( parentWgt )
    , controller_( new Controller )
{
    ui_.setupUi( this );
    setWindowFlags( (windowFlags( ) ^ Qt::Dialog) | Qt::Window);
}

TransferFunctionEditor::~TransferFunctionEditor( )
{
}

void TransferFunctionEditor::updateCurve( const bool immediate )
{
    if( ui_.previewCheckBox_->isChecked() && immediate )
        controller_->publishTransferFunction();
}

void TransferFunctionEditor::closeEvent( QCloseEvent* closeEvt )
{
    position_ = pos();
    QWidget::closeEvent( closeEvt );
}

void TransferFunctionEditor::showEvent( QShowEvent* showEvt )
{
    createAndConnectGraph_( GT_GAUSS );
    QWidget::showEvent( showEvt );
}

void TransferFunctionEditor::createAndConnectGraph_( const GraphType graphType )
{
    switch ( graphType )
    {
    case GT_GAUSS:
        transferFunctionGraphPtr_.reset( new AnchorGraph( AnchorGraphCorePtr( new GaussGraphCore( ) ) ) );
        break;
    case GT_DOUBLEGAUSS:
        transferFunctionGraphPtr_.reset( new AnchorGraph( AnchorGraphCorePtr( new DoubleGaussGraphCore( ) ) ) );
        break;
    }
    connectGraph_( );
    initializeGraph_( );
}

void TransferFunctionEditor::changeGraph( int graphTypei )
{
    GraphType graphType = static_cast< GraphType >( graphTypei );
    createAndConnectGraph_( graphType );
    return;
}

void TransferFunctionEditor::connectGraph_()
{
    ui_.gridLayout2_->addWidget( transferFunctionGraphPtr_.get( ), 0, 0, 0, 0);
    connect( ui_.checkBoxR_, SIGNAL(toggled(bool)), transferFunctionGraphPtr_.get( ), SLOT(setStateR(const bool)));
    connect( ui_.checkBoxG_, SIGNAL(toggled(bool)), transferFunctionGraphPtr_.get( ), SLOT(setStateG(const bool)));
    connect( ui_.checkBoxB_, SIGNAL(toggled(bool)), transferFunctionGraphPtr_.get( ), SLOT(setStateB(const bool)));
    connect( ui_.checkBoxA_, SIGNAL(toggled(bool)), transferFunctionGraphPtr_.get( ), SLOT(setStateA(const bool)));
    connect( transferFunctionGraphPtr_.get( ), SIGNAL(curveUpdate(bool)), this, SLOT( updateCurve(const bool)) );
}

void TransferFunctionEditor::initializeGraph_( )
{
    transferFunctionGraphPtr_->setTransferFunction(
        controller_->getTransferFunction( ));
    transferFunctionGraphPtr_->setStateR( true );
    transferFunctionGraphPtr_->setStateG( true );
    transferFunctionGraphPtr_->setStateB( true );
    transferFunctionGraphPtr_->setStateA( true );
    transferFunctionGraphPtr_->update();
}

void TransferFunctionEditor::setHistogram( const UInt8Vector& histogram )
{
    histogram_ = histogram;
    transferFunctionGraphPtr_->setHistogram( histogram );
}

TransferFunctionEditor::GraphType TransferFunctionEditor::getGraphType( ) const
{
    return static_cast< GraphType >( ui_.graphComboBox_->currentIndex( ) );
}

void TransferFunctionEditor::setGraphType( const GraphType graphType )
{
    if( ui_.graphComboBox_->currentIndex( ) == graphType )
        return;

    ui_.graphComboBox_->setCurrentIndex( static_cast< uint32_t >( graphType ) );
}

TransferFunctionGraphPtr TransferFunctionEditor::getGraph( ) const
{
    return transferFunctionGraphPtr_;
}

}
