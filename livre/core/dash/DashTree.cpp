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

#include <livre/core/visitor/NodeVisitor.h>

#include <livre/core/dash/DashTree.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashRenderStatus.h>

#include <livre/core/data/DataSource.h>
#include <livre/core/data/NodeId.h>

namespace livre
{

typedef std::unordered_map< Identifier, dash::NodePtr > IdDashNodeMap;

struct DashTree::Impl
{
public:
    explicit Impl( const DataSource& dataSource )
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

    ~Impl()
    {
        _localContext.commit();
        for( DashContextPtr ctx: dashContexts )
        {
            ctx->commit();
        }
        delete _renderStatus;
    }

    DashContextPtr createContext()
    {
        WriteLock writeLock( _mutex );
        _localContext.commit();
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

    dash::NodePtr getDashNode( const NodeId& nodeId) const
    {
        ReadLock readLock( _mutex );
        IdDashNodeMap::const_iterator it = _dashNodeMap.find( nodeId.getId( ));
        return it == _dashNodeMap.end() ? dash::NodePtr() : it->second;
    }

    dash::NodePtr getDashNode( const NodeId& nodeId )
    {
        LBASSERT( &_localContext != &dash::Context::getCurrent() );
        IdDashNodeMap::const_iterator it = _dashNodeMap.find( nodeId.getId( ));

        // "Double-Checked Locking" idiom is used below.
        {
            ReadLock readLock( _mutex );
            if( it != _dashNodeMap.end( ) && it->second )
                return it->second;
        }

        WriteLock writeLock( _mutex );
        it = _dashNodeMap.find( nodeId.getId( ));
        if( it != _dashNodeMap.end( ))
            return it->second;

        dash::Context& prevCtx = dash::Context::getCurrent();
        _localContext.setCurrent();

        const LODNode& lodNode = _dataSource.getNode( nodeId );
        if( !lodNode.isValid( ))
        {
            prevCtx.setCurrent();
            return dash::NodePtr();
        }

        dash::NodePtr node = new dash::Node();
        DashRenderNode::initializeDashNode( node );
        DashRenderNode renderNode( node );
        renderNode.setLODNode( lodNode );

        _localContext.commit();
        for( DashContextPtr ctx: dashContexts )
        {
            _localContext.map( node, *ctx );
        }
        prevCtx.setCurrent();
        _dashNodeMap[ nodeId.getId() ] = node;
        return node;
    }

    const DataSource& _dataSource;
    IdDashNodeMap _dashNodeMap;
    DashRenderStatus* _renderStatus;
    mutable ReadWriteMutex _mutex;
    dash::Context& _localContext;
    std::vector< DashContextPtr > dashContexts;
};

DashTree::DashTree( const DataSource& dataSource )
    : _impl( new Impl( dataSource ))
{}

const DataSource& DashTree::getDataSource() const
{
    return _impl->_dataSource;
}

DashTree::~DashTree()
{}

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

dash::NodePtr DashTree::getDashNode( const NodeId& nodeId) const
{
     return _impl->getDashNode( nodeId ) ;
}

}
