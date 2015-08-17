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


#ifndef _GeneratorPropertiesModel_h_
#define _GeneratorPropertiesModel_h_

#include <livreGUI/types.h>

#include <QAbstractTableModel>

namespace livre
{

/**
 * GeneratorPropertiesModel has the model data for the property table view.
 * It updates the NEST properties when data is changed.
 */
class GeneratorPropertiesModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    /**
     * @param parentWgt Parent widget.
     */
    explicit GeneratorPropertiesModel( QObject *parentObj );
     ~GeneratorPropertiesModel( );

    int columnCount( const QModelIndex& parent = QModelIndex( )) const final;
    int rowCount( const QModelIndex& parent = QModelIndex( )) const final;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const final;
    Qt::ItemFlags flags(const QModelIndex &index ) const final;
    QVariant headerData(int section, Qt::Orientation orientation, int role ) const final;
    bool setData( const QModelIndex& index, const QVariant& value, int role )  final;

    void setProperties( const PropertyList& properties );
    const PropertyList& getProperties() const;

private:

    PropertyList _properties;
};

}
#endif
