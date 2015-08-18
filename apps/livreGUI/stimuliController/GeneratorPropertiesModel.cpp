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

#include <livreGUI/stimuliController/GeneratorPropertiesModel.h>

#include <QFont>
#include <QBrush>

namespace livre
{


GeneratorPropertiesModel::GeneratorPropertiesModel( QObject *parentObj )
    : QAbstractTableModel( parentObj )
{

}

GeneratorPropertiesModel::~GeneratorPropertiesModel()
{

}

int GeneratorPropertiesModel::columnCount( const QModelIndex& ) const
{
    return 2;
}

int GeneratorPropertiesModel::rowCount( const QModelIndex& ) const
{
    return _properties.size();
}

QVariant GeneratorPropertiesModel::data( const QModelIndex& ind,
                                         int role ) const
{
    switch( role )
    {
        case Qt::DisplayRole:
        {
            const Property& prop = _properties[ ind.row() ];
            if( ind.column() == 0 )
            {
                 return QString( prop.first.c_str( ));
            }
            else if( ind.column() == 1 )
            {
                 return prop.second;
            }
            break;
        }
        case Qt::FontRole:
        {
            if( ind.column() == 0 )
            {
                QFont boldFont;
                boldFont.setBold( true );
                return boldFont;
            }
            break;
        }
        case Qt::BackgroundRole:
        {
            if( ind.column() == 0 )
            {
                QBrush background( Qt::lightGray );
                return background;
            }
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags GeneratorPropertiesModel::flags( const QModelIndex& ind ) const
{
    switch( ind.column() )
    {
    case 0:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    case 1:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant GeneratorPropertiesModel::headerData(
                                   int section,
                                   Qt::Orientation orientation,
                                   int role ) const
{
    switch( role )
    {
        case Qt::DisplayRole:
        {
            if( orientation == Qt::Horizontal )
            {
                switch (section)
                {
                case 0:
                    return QString( "Stimuli Property" );
                case 1:
                    return QString( "Value" );
                }
            }
        }
    }
    return QVariant();
}

bool GeneratorPropertiesModel::setData( const QModelIndex& ind,
                                        const QVariant& data_,
                                        int role )
{
    if( role == Qt::EditRole )
    {
        Property& prop = _properties[ ind.row() ];
        prop.second = data_;
        emit dataChanged( ind, ind );
        return true;
    }
    return false;
}

void GeneratorPropertiesModel::setProperties( const PropertyList& properties )
{
    _properties = properties;
    emit layoutChanged();
}

const PropertyList& GeneratorPropertiesModel::getProperties() const
{
   return _properties;

}

}
