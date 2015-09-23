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

#include <monsteer/steering/simulator.h>

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
          const servus::URI& selectionZeqSchema,
          const servus::URI& simulationZeqSchema,
          Ui_stimuliController& ui )
        : _simulator( 0 )
        , _isRegistered( false )
        , _ui( ui )
        , _stimuliController( stimuliController )
        , _controller( controller )
        , _selectionZeqSchema( selectionZeqSchema )
        , _simulationZeqSchema( simulationZeqSchema )
    {
        connectHBP();
        connectISC();
    }

    ~Impl()
    {
        disconnectISC();
        disconnectHBP();
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

        const int generatorIndex = _ui.generatorsComboBox->currentIndex();
        if( generatorIndex < 0 )
            return;

        const Strings& generators = getGenerators();
        const std::string& generator = generators[ generatorIndex ];

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

    void generatorSelected( const int generatorIndex )
    {
        if( generatorIndex < 0 )
            return;

        const Strings& generators = getGenerators();
        const PropertyList& prop = getGeneratorProperties( generators[ generatorIndex ] );
        GeneratorPropertiesModel* model =
                static_cast< GeneratorPropertiesModel* >( _ui.tblGeneratorProperties->model( ));

        model->setProperties( prop );
    }

    void connectISC()
    {
        try
        {
            _simulator = _controller.getSimulator( _simulationZeqSchema );
        }
        catch( const std::exception& error )
        {
            _ui.btnInjectStimulus->setEnabled( false );
            LBERROR << "Error:" << error.what() << std::endl;
        }
    }

    void disconnectISC()
    {
        _simulator = 0;
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

        const int generatorIndex = _ui.generatorsComboBox->currentIndex();

        _ui.btnInjectStimulus->setEnabled( _simulator &&
                                           !_selectedIds.empty( )
                                           && generatorIndex >= 0 );

        std::stringstream str;
        BOOST_FOREACH( const uint32_t gid, _selectedIds )
            str << gid << " ,";

        _ui.txtCellIds->setText( str.str().c_str( ));
    }

    void setSubscriber()
    {
        _controller.registerHandler( _selectionZeqSchema,
                                     zeq::hbp::EVENT_SELECTEDIDS,
                                     boost::bind( &StimuliController::Impl::onSelection,
                                                  this, _1 ));
        _isRegistered = true;
    }

    void connectHBP()
    {
        try
        {
            setSubscriber();
        }
        catch( const std::exception& error )
        {
            LBERROR << "Error:" << error.what() << std::endl;
            _isRegistered = false;
        }
    }

    void disconnectHBP()
    {
        _controller.deregisterHandler( _selectionZeqSchema,
                                       zeq::hbp::EVENT_SELECTEDIDS,
                                       boost::bind( &StimuliController::Impl::onSelection,
                                                  this, _1 ));
        _isRegistered = false;
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

    ::monsteer::Simulator* _simulator;
    bool _isRegistered;
    Ui_stimuliController& _ui;
    StimuliController* _stimuliController;
    Controller& _controller;
    servus::URI _selectionZeqSchema;
    servus::URI _simulationZeqSchema;

    std::vector<uint32_t> _selectedIds;
};

StimuliController::StimuliController( Controller& controller,
                                      const servus::URI& selectionZeqSchema,
                                      const servus::URI& simulationZeqSchema,
                                      QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new StimuliController::Impl( this, controller, selectionZeqSchema,
                                          simulationZeqSchema, _ui ))
{
    qRegisterMetaType< std::vector<uint32_t> >("std::vector<uint32_t>");

    _ui.setupUi( this );
    _ui.txtCellIds->setReadOnly( true );
    GeneratorModel* generatorModel = new GeneratorModel( _ui.generatorsComboBox );
    _ui.generatorsComboBox->setModel( generatorModel );

    PropertyEditDelegate* editorDelegate =
            new PropertyEditDelegate( _ui.tblGeneratorProperties );
    _ui.tblGeneratorProperties->setItemDelegate( editorDelegate );

    GeneratorPropertiesModel *model =
            new GeneratorPropertiesModel( _ui.tblGeneratorProperties );

    _ui.tblGeneratorProperties->setModel( model );
    _ui.tblGeneratorProperties->setColumnWidth( 0, 150 );
    _ui.tblGeneratorProperties->setColumnWidth( 1, 150 );
    _ui.tblGeneratorProperties->setMinimumWidth( 310 );

    _ui.btnInjectStimulus->setEnabled( false );

    setWindowFlags(( windowFlags() ^ Qt::Dialog) | Qt::Window );

    connect( _ui.btnInjectStimulus, SIGNAL(clicked()),
             this, SLOT(_injectStimuli()));

    connect( _ui.generatorsComboBox, SIGNAL( currentIndexChanged( int )),
             this, SLOT( _generatorSelected( int )));

    connect( model, SIGNAL( layoutChanged( )),
             this, SLOT( _propertiesChanged( )));

    connect( this, SIGNAL( updateCellIdsTextBox( std::vector<uint32_t> )),
             this, SLOT( _updateCellIdsTextBox( std::vector<uint32_t> )),
             Qt::QueuedConnection );

    _ui.generatorsComboBox->setCurrentIndex( 0 );
    // Emulate currentIndexChanged() signal as qApp has not been created yet
    _impl->generatorSelected( 0 );
}

StimuliController::~StimuliController( )
{
    delete _impl;
}

void StimuliController::_injectStimuli()
{
     _impl->injectStimuli();
}

void StimuliController::_generatorSelected( const int index )
{
    _impl->generatorSelected( index );
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
