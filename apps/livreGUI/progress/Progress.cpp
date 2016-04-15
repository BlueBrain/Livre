/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#include "Progress.h"
#include "../Controller.h"

#include <livreGUI/ui_Progress.h>
#include <zerobuf/data/progress.h>
#include <QLabel>
#include <QProgressBar>

namespace livre
{
namespace
{
class OperationUI
{
public:
    OperationUI( QWidget* parent )
        : layout( new QHBoxLayout )
        , name( new QLabel( parent ))
        , bar( new QProgressBar( parent ))
    {
        layout->addWidget( name );
        layout->addWidget( bar );
    }

    ~OperationUI()
    {
        delete bar;
        delete name;
        delete layout;
    }

    QHBoxLayout* layout;
    QLabel* name;
    QProgressBar* bar;
};
}

struct Progress::Impl
{
    Impl( Progress* parent, Controller& controller )
        : _parent( parent )
        , _controller( controller )
        , _progress( 0 )
    {
        _ui.setupUi( parent );
        _ui.verticalLayout->setAlignment( Qt::AlignTop );

        _progress.setUpdatedFunction(
            [parent]() { emit parent->updated(); });
        _controller.subscribe( _progress );
    }

    ~Impl()
    {
        _controller.unsubscribe( _progress );
    }

    void onUpdated()
    {
        const std::string& name = _progress.getOperationString();
        OperationUI* ui = _operations[ name ];

        if( _progress.getAmount() >= 1.f )
        {
            _operations.erase( name );
            delete ui;
        }
        else
        {
            if( !ui )
            {
                ui = new OperationUI( _parent );
                _ui.verticalLayout->addLayout( ui->layout );

                ui->bar->setMaximum( 1000 );
                ui->name->setText( name.c_str( ));
                _operations[ name ] = ui;
            }

            ui->bar->setValue( _progress.getAmount() * 1000 );
        }

        _parent->parentWidget()->setHidden( _operations.empty( ));
    }

private:
    Progress* const _parent;
    Ui::Progress _ui;
    Controller& _controller;
    ::zerobuf::data::Progress _progress;

    std::map< std::string, OperationUI* > _operations;
};

Progress::Progress( Controller& controller, QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new Progress::Impl( this, controller ))
{
    connect( this, &Progress::updated, this, &Progress::onUpdated );
}

Progress::~Progress( )
{}

void Progress::onUpdated()
{
    _impl->onUpdated();
}

}
