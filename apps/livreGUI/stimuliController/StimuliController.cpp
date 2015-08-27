/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/types.h>

#include <livreGUI/stimuliController/StimuliController.h>
#include <livreGUI/stimuliController/GeneratorModel.h>
#include <livreGUI/stimuliController/GeneratorPropertiesModel.h>
#include <livreGUI/stimuliController/PropertyEditDelegate.h>
#include <livreGUI/stimuliController/nestData.h>
#include <livreGUI/Controller.h>

#include <isc/steering/simulator.h>

#include <zeq/types.h>
#include <zeq/subscriber.h>
#include <zeq/hbp/vocabulary.h>

#include <QFileDialog>
#include <QModelIndex>
#include <QItemSelectionModel>

#include <boost/thread.hpp>

namespace livre
{

struct StimuliController::Impl
{

    Impl( StimuliController* stimuliController,
          Controller& controller,
          Ui_stimuliController& ui )
        : _simulator( 0 )
        , _isRegistered( false )
        , _ui( ui )
        , _stimuliController( stimuliController )
        , _controller( controller )
    {

    }

    ~Impl()
    {
    }

    void injectStimuli()
    {
        if( !_simulator )
            return;

        if( _selectedIds.empty( ))
            return;

        GeneratorPropertiesModel *model =
                static_cast< GeneratorPropertiesModel * >(
                    _ui.tblGeneratorProperties->model( ));

        QItemSelectionModel* selectionModel = _ui.tblGenerators->selectionModel();
        const QModelIndex& ind = selectionModel->currentIndex();
        if( !ind.isValid( ))
            return;

        const Strings& generators = getGenerators();
        const std::string& generator = generators[ ind.row() ];

        PropertyList properties = model->getProperties( );
        QVariantList _list;
        _list.push_back( QVariant(QString(generator.c_str( ))));
        properties.push_back( std::make_pair("model", _list ));

        const std::string& json = getJSON( properties );
        if( json.empty( ))
            return;

        const bool individual = _ui.chkIndividual->checkState() == Qt::Checked;

        if( individual)
            _simulator->injectMultipleStimuli( json, _selectedIds );
        else
            _simulator->injectStimulus( json, _selectedIds );
    }

    void generatorSelected()
    {
        QItemSelectionModel* selectionModel = _ui.tblGenerators->selectionModel();
        const QModelIndex& ind = selectionModel->currentIndex();
        if( !ind.isValid( ))
            return;

        const Strings& generators = getGenerators();
        const PropertyList& prop = getGeneratorProperties( generators[ ind.row() ] );
        GeneratorPropertiesModel* model =
                static_cast< GeneratorPropertiesModel* >(_ui.tblGeneratorProperties->model());

        model->setProperties( prop );
    }

    void connectISC()
    {
        try
        {
            const QString& uriStr = _ui.txtISCURL->text();
            const servus::URI uri( uriStr.toStdString( ));
            _simulator =  _controller.getSimulator( uri );
            _ui.btnConnectISC->setEnabled( false );
            _ui.btnDisconnectISC->setEnabled( true );
        }
        catch( const std::exception& error )
        {
            _ui.btnConnectISC->setEnabled( true );
            _ui.btnInjectStimulus->setEnabled( false );
            _ui.btnDisconnectISC->setEnabled( false );
            LBERROR << "Error:" << error.what() << std::endl;
        }
    }

    void disconnectISC()
    {
        _simulator = 0;
        _ui.btnConnectISC->setEnabled( true );
        _ui.btnDisconnectISC->setEnabled( false );
    }

    void onSelection( const ::zeq::Event& event_ )
    {
        const std::vector<uint32_t>& selection
                = zeq::hbp::deserializeSelectedIDs( event_ );
        emit _stimuliController->updateCellIdsTextBox( selection );
    }

    void updateCellIdsTextBox( const std::vector<uint32_t>& cellIds )
    {
        _selectedIds = cellIds;

        QItemSelectionModel* selectionModel = _ui.tblGenerators->selectionModel();
        const QModelIndex& ind = selectionModel->currentIndex();

        _ui.btnInjectStimulus->setEnabled( _simulator &&
                                           !_selectedIds.empty( )
                                           && ind.isValid( ));

        std::stringstream str;
        BOOST_FOREACH( const uint32_t gid, _selectedIds )
            str << gid << " ,";

        _ui.txtCellIds->setText( str.str().c_str( ));
    }

    void setSubscriber()
    {
        const QString& uriStr = _ui.txtHBPURL->text();
        const servus::URI uri( uriStr.toStdString( ));
        _controller.registerHandler(
                                 uri,
                                 zeq::hbp::EVENT_SELECTEDIDS,
                                 boost::bind( &StimuliController::Impl::onSelection,
                                            this, _1 ));
        _isRegistered = true;
    }

    void connectHBP()
    {
        try
        {
            _ui.btnConnectHBP->setEnabled( false );
            _ui.btnDisconnectHBP->setEnabled( true );
            setSubscriber();
        }
        catch( const std::exception& error )
        {
            _ui.btnConnectHBP->setEnabled( true );
            _ui.btnDisconnectHBP->setEnabled( false );
            LBERROR << "Error:" << error.what() << std::endl;
            _isRegistered = false;
        }
    }

    void disconnectHBP()
    {
        const QString& uriStr = _ui.txtHBPURL->text();
        const servus::URI uri( uriStr.toStdString( ));
        _controller.deregisterHandler( uri,
                                       zeq::hbp::EVENT_SELECTEDIDS,
                                       boost::bind( &StimuliController::Impl::onSelection,
                                                  this, _1 ));
        _isRegistered = false;
        _ui.btnConnectHBP->setEnabled( true );
        _ui.btnDisconnectHBP->setEnabled( false );
    }

    void propertiesChanged()
    {
        GeneratorPropertiesModel* model =
                static_cast< GeneratorPropertiesModel* >(_ui.tblGeneratorProperties->model());

        for( int i = 0; i < model->rowCount(); ++i )
        {
            _ui.tblGeneratorProperties->closePersistentEditor( model->index( i, 1 ));
            _ui.tblGeneratorProperties->openPersistentEditor( model->index( i, 1 ));
        }
    }

public:

    isc::Simulator* _simulator;
    bool _isRegistered;
    Ui_stimuliController& _ui;
    StimuliController* _stimuliController;
    Controller& _controller;
    std::vector<uint32_t> _selectedIds;
};

StimuliController::StimuliController( Controller& controller,
                                      QWidget* parentWgt )
    : QWidget( parentWgt ),
      _impl( new StimuliController::Impl( this, controller, _ui ))
{
    qRegisterMetaType< std::vector<uint32_t> >("std::vector<uint32_t>");

    _ui.setupUi( this );
    _ui.txtCellIds->setReadOnly( true );
    GeneratorModel *generatorModel = new GeneratorModel( _ui.tblGenerators );
    _ui.tblGenerators->setModel( generatorModel );
    _ui.tblGenerators->setColumnWidth( 0, 300 );
    _ui.tblGenerators->setSelectionMode( QAbstractItemView::SingleSelection );
    _ui.tblGenerators->setSelectionBehavior( QAbstractItemView::SelectRows );

    PropertyEditDelegate* editorDelegate =
            new PropertyEditDelegate( _ui.tblGeneratorProperties );
    _ui.tblGeneratorProperties->setItemDelegate( editorDelegate );

    GeneratorPropertiesModel *model =
            new GeneratorPropertiesModel( _ui.tblGeneratorProperties );

    _ui.tblGeneratorProperties->setModel( model );
    _ui.tblGeneratorProperties->setColumnWidth( 0, 150 );
    _ui.tblGeneratorProperties->setColumnWidth( 1, 150 );

    _ui.btnInjectStimulus->setEnabled( false );

    setWindowFlags(( windowFlags() ^ Qt::Dialog) | Qt::Window );

    connect( _ui.btnInjectStimulus, SIGNAL(clicked()),
             this, SLOT(_injectStimuli()));

    QItemSelectionModel* selectionModel = _ui.tblGenerators->selectionModel();
    connect( selectionModel, SIGNAL(selectionChanged(const QItemSelection&,
                                                     const QItemSelection& )),
             this, SLOT(_generatorSelected(const QItemSelection&,
                                          const QItemSelection& )));

    _ui.btnDisconnectHBP->setEnabled( false );
    _ui.btnDisconnectISC->setEnabled( false );

    connect( model, SIGNAL(layoutChanged()), this, SLOT(_propertiesChanged()));
    connect( _ui.btnConnectISC, SIGNAL(pressed( )), this, SLOT(_connectISC( )));
    connect( _ui.btnConnectHBP, SIGNAL(pressed( )), this, SLOT(_connectHBP( )));
    connect( _ui.btnDisconnectISC, SIGNAL(pressed( )), this, SLOT(_disconnectISC( )));
    connect( _ui.btnDisconnectHBP, SIGNAL(pressed( )), this, SLOT(_disconnectHBP( )));

    connect( this, SIGNAL(updateCellIdsTextBox(std::vector<uint32_t>)),
             this, SLOT(_updateCellIdsTextBox(std::vector<uint32_t>)),
             Qt::QueuedConnection );

    _ui.tblGenerators->selectRow( 0 );
}

StimuliController::~StimuliController( )
{
    delete _impl;
}

void StimuliController::_injectStimuli()
{
     _impl->injectStimuli();
}

void StimuliController::_generatorSelected( const QItemSelection&,
                                            const QItemSelection& )
{
    _impl->generatorSelected();
}

void StimuliController::_updateCellIdsTextBox( std::vector<uint32_t> cellIds )
{
    _impl->updateCellIdsTextBox( cellIds );
}

void StimuliController::_connectISC()
{
    _impl->connectISC();
}

void StimuliController::_connectHBP()
{
    _impl->connectHBP();
}

void StimuliController::_disconnectISC()
{
     _impl->disconnectISC();
}

void StimuliController::_disconnectHBP()
{
     _impl->disconnectHBP();
}

void StimuliController::_propertiesChanged()
{
    _impl->propertiesChanged();
}

}
