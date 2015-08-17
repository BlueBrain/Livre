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

#ifndef _PropertyEditDelegate_h_
#define _PropertyEditDelegate_h_

#include <livreGUI/qtTypes.h>

#include <QItemDelegate>

namespace livre
{

/**
 * TableView widgets are delegated to this class in order to generate the
 * right widget for the table data.
 */
class PropertyEditDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    /**
     * @param parent Parent widget
     */
    explicit PropertyEditDelegate( QObject *parentWidget = 0 );

    QWidget* createEditor( QWidget* parent,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index) const final;
    void setEditorData( QWidget* editor,
                        const QModelIndex& index ) const final;
    void setModelData( QWidget* editor,
                       QAbstractItemModel* model,
                       const QModelIndex& index ) const final;
    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const final;
};

}

#endif
