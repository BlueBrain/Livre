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

#include <livreGUI/stimuliController/GeneratorModel.h>
#include <livreGUI/stimuliController/nestData.h>

namespace livre
{

GeneratorModel::GeneratorModel( QObject* parentObj )
    : QAbstractTableModel( parentObj )
{

}

GeneratorModel::~GeneratorModel()
{

}

int GeneratorModel::columnCount( const QModelIndex& ) const
{
    return 1;
}

int GeneratorModel::rowCount( const QModelIndex& ) const
{
    const Strings& generators = getGenerators();
    return generators.size();
}

QVariant GeneratorModel::data( const QModelIndex& ind, int role ) const
{
    if( role == Qt::DisplayRole )
    {
        const Strings& generators = getGenerators();
        return generators[ ind.row() ].c_str();
    }
    return QVariant();
}

Qt::ItemFlags GeneratorModel::flags( const QModelIndex& ) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant GeneratorModel::headerData( int section,
                                   Qt::Orientation orientation,
                                   int role ) const
{
    if( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Horizontal ) {
            switch (section)
            {
            case 0:
                return QString("Generator");
            }
        }
    }
    return QVariant();
}

}
