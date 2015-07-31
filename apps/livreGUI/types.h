/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _GuiTypes_h_
#define _GuiTypes_h_

#include <livre/core/lunchboxTypes.h>
#include <livre/eq/types.h>

namespace livre
{
/**
 * Class definitions
 */
class AnchorGraphCore;
class ConnectDialog;
class Controller;
class DoubleGaussGraphCore;
class GaussGraphCore;
class MainWindow;
class RampGraphCore;
class RecentFiles;
class TransferFunctionEditor;
class TransferFunctionGraph;
class TransferFunctionGraphCore;

/**
 * SmartPtr definitions
 */
typedef boost::shared_ptr< ConnectDialog > ConnectDialogPtr;
typedef boost::shared_ptr< Controller > ControllerPtr;
typedef boost::shared_ptr< TransferFunctionGraph > TransferFunctionGraphPtr;
typedef boost::shared_ptr< AnchorGraphCore > AnchorGraphCorePtr;
typedef boost::shared_ptr< const AnchorGraphCore > ConstAnchorGraphCorePtr;
typedef boost::shared_ptr< GaussGraphCore > GaussGraphCorePtr;
typedef boost::shared_ptr< const GaussGraphCore > ConstGaussGraphCorePtr;
typedef boost::shared_ptr< TransferFunctionGraphCore > GraphCorePtr;
typedef boost::shared_ptr< const TransferFunctionGraphCore > ConstGraphCorePtr;

}

#endif // _GuiTypes_h_
