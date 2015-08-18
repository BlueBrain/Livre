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

#include <livreGUI/stimuliController/PropertyEditDelegate.h>
#include <livreGUI/stimuliController/GeneratorPropertiesModel.h>

#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QModelIndex>

namespace livre
{

PropertyEditDelegate::PropertyEditDelegate( QObject* parent_ )
    : QItemDelegate( parent_ )
{
}


QWidget* PropertyEditDelegate::createEditor( QWidget* parent_,
                                             const QStyleOptionViewItem&,
                                             const QModelIndex& index ) const
{
    if( index.column() == 0 )
        return 0;

    const QVariant& data = index.data();

    QVariant::Type type = data.type( );
    switch( type )
    {
        case QVariant::Double:
        {
            QLineEdit* editor = new QLineEdit( parent_ );
            editor->setValidator( new QDoubleValidator( editor ));
            return editor;
        }
        case QVariant::Int:
        {
            QSpinBox* editor = new QSpinBox( parent_ );
            return editor;
        }
        case QVariant::Bool:
        {
            QCheckBox* editor = new QCheckBox( parent_ );
            return editor;
        }
        case QVariant::List:
        {
            QLineEdit* editor = new QLineEdit( parent_ );
            return editor;
        }
        default:
        {
            QLineEdit* editor = new QLineEdit( parent_ );
            return editor;
        }
    }
}

void PropertyEditDelegate::setEditorData( QWidget* editor_,
                                          const QModelIndex& index ) const
{
    const QVariant& data = index.data();

    switch( data.type( ))
    {
        case QVariant::Double:
        {
            QLineEdit* editor = static_cast< QLineEdit *>( editor_ );
            editor->setText( QString::number( data.toDouble( )));
            break;
        }
        case QVariant::Int:
        {
            QSpinBox* editor = static_cast< QSpinBox *>( editor_ );
            editor->setValue( data.toInt( ));
            break;
        }
        case QVariant::Bool:
        {
            QCheckBox* editor = static_cast< QCheckBox *>( editor_ );
            const bool value = data.toBool();
            editor->setCheckState( value ? Qt::Checked : Qt::Unchecked );
            break;
        }
        case QVariant::List:
        {
            QLineEdit* editor = static_cast< QLineEdit *>( editor_ );
            QString str;
            BOOST_FOREACH( const QVariant& var, data.toList( ))
                str += QString::number( var.toDouble( )) + ",";
            editor->setText( str );
            break;
        }
        default:
        {
            break;
        }
    }
}

void PropertyEditDelegate::setModelData( QWidget* editor_,
                                         QAbstractItemModel* model_,
                                         const QModelIndex& index ) const
{
    const QVariant& data = index.data();
    GeneratorPropertiesModel* model =
            static_cast< GeneratorPropertiesModel* >( model_ );

    switch( data.type( ))
    {
        case QVariant::Double:
        {
            QLineEdit* editor = static_cast< QLineEdit *>( editor_ );
            model->setData( index, editor->text().toDouble(), Qt::EditRole );
            break;
        }
        case QVariant::Int:
        {
            QSpinBox* editor = static_cast< QSpinBox *>( editor_ );
            editor->interpretText();
            model->setData( index, editor->value(), Qt::EditRole );
            break;
        }
        case QVariant::Bool:
        {
            QCheckBox* editor = static_cast< QCheckBox *>( editor_ );
            const bool value = editor->checkState() == Qt::Checked;
            model->setData( index, value, Qt::EditRole );
            break;
        }
        case QVariant::List:
        {
            QLineEdit* editor = static_cast< QLineEdit *>( editor_ );
            QStringList list = editor->text().split( "," );
            QVariantList varList;
            QDoubleValidator validator;
            BOOST_FOREACH( QString& str, list )
            {
                if( str == "" )
                    continue;

                int pos = 0;
                if( validator.validate( str, pos ) == QValidator::Acceptable )
                    varList.push_back( QVariant( str.toDouble( )));
                else
                    varList.push_back( QVariant( -1.0 ));

            }
            model->setData( index, varList, Qt::EditRole );
            break;
        }
        default:
        {
            break;
        }
    }
}

void PropertyEditDelegate::updateEditorGeometry( QWidget* editor_,
                                                 const QStyleOptionViewItem& option_,
                                                 const QModelIndex& ind ) const
{
    const QVariant& data = ind.data();
    if( data.type() == QVariant::Bool )
    {
        QRect r( option_.rect );
        r.setLeft( r.left() + 40 );
        editor_->setGeometry( r );
    }
    else
        editor_->setGeometry( option_.rect );
}

}
