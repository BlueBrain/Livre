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

#include <livre/core/Visitor/NodeVisitor.h>

#include <livre/core/Dash/DashTree.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Dash/DashRenderStatus.h>

#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/NodeId.h>

namespace livre
{

namespace detail
{

class DashTree : public boost::noncopyable
{

public:

    DashTree( ConstVolumeDataSourcePtr dataSource )
        : _dataSource( dataSource ),
          _localContext( dash::Context::getMain( ))
    {
        dash::Context& prevCtx = dash::Context::getCurrent();
        _localContext.setCurrent();
        // Instantiation should be done in the context dash tree knows
        _renderStatus = new DashRenderStatus();
        _localContext.commit();
        prevCtx.setCurrent();
    }

    ~DashTree( )
    {
        _localContext.commit();
        BOOST_FOREACH( DashContextPtr ctx, dashContexts )
        {
            ctx->commit();
        }
        delete _renderStatus;
    }

    DashContextPtr createContext()
    {
        DashContextPtr ctx( new dash::Context( ));
        dashContexts.push_back( ctx );
        _localContext.map( _renderStatus->getDashNode(), *dashContexts.back() );
        return ctx;
    }

    const dash::NodePtr getParentNode( const NodeId& nodeId )
    {
        if( nodeId.isRoot( ))
            return dash::NodePtr();

        const NodeId& parentNodeId = nodeId.getParent();
        return getDashNode( parentNodeId ) ;
    }

    dash::NodePtr getDashNode( const NodeId& nodeId )
    {
        // "Double-Checked Locking" idiom is used below.
        LBASSERT( &_localContext != &dash::Context::getCurrent() );
        ReadLock readLock( _mutex );
        NodeIDDashNodePtrMap::const_iterator it = _dashNodeMap.find( nodeId );
        if( it != _dashNodeMap.end() )
            return it->second;

        readLock.unlock();

        WriteLock writeLock( _mutex );
        it = _dashNodeMap.find( nodeId );
        if( it != _dashNodeMap.end() )
            return it->second;

        dash::Context& prevCtx = dash::Context::getCurrent();
        _localContext.setCurrent();

        dash::NodePtr node = new dash::Node();
        DashRenderNode::initializeDashNode( node );
        ConstLODNodePtr lodNodePtr = _dataSource->getNodeFromNodeID( nodeId );
        DashRenderNode renderNode( node );
        renderNode.setLODNode( *lodNodePtr );

        _localContext.commit();
        BOOST_FOREACH( DashContextPtr ctx, dashContexts )
        {
            _localContext.map( node, *ctx );
        }
        prevCtx.setCurrent();
        _dashNodeMap[ nodeId ] = node;
        return node;

    }

    ConstVolumeDataSourcePtr _dataSource;
    NodeIDDashNodePtrMap _dashNodeMap;
    DashRenderStatus* _renderStatus;
    mutable ReadWriteMutex _mutex;
    dash::Context& _localContext;
    std::vector< DashContextPtr > dashContexts;
};

}

DashTree::DashTree( ConstVolumeDataSourcePtr datasSource )
    : _impl( new detail::DashTree( datasSource ))
{}

ConstVolumeDataSourcePtr DashTree::getDataSource() const
{
    return _impl->_dataSource;
}

DashTree::~DashTree()
{
    delete _impl;
}

DashContextPtr DashTree::createContext()
{
    return _impl->createContext();
}

const DashRenderStatus& DashTree::getRenderStatus() const
{
    return *_impl->_renderStatus;
}

DashRenderStatus& DashTree::getRenderStatus()
{
    return *_impl->_renderStatus;
}

const dash::NodePtr DashTree::getParentNode( const NodeId& nodeId )
{
    return _impl->getParentNode( nodeId ) ;
}

dash::NodePtr DashTree::getDashNode( const NodeId& nodeId )
{
    return _impl->getDashNode( nodeId ) ;
}

}
