#include <livre/core/Visitor/RenderNodeVisitor.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Dash/DashTree.h>

namespace livre
{

namespace detail
{

class RenderNodeVisitor
{
public:

    RenderNodeVisitor(  DashTreePtr dashTree )
        : _dashTree( dashTree )
    {}

    dash::NodePtr getDashNode( const NodeId& nodeId )
    {
        return _dashTree->getDashNode( nodeId );
    }

    DashTreePtr getDashTree( ) { return _dashTree; }
    DashTreePtr _dashTree;
};

}

RenderNodeVisitor::RenderNodeVisitor( DashTreePtr dashTree )
    : _impl( new detail::RenderNodeVisitor( dashTree ))
{

}

RenderNodeVisitor::~RenderNodeVisitor()
{
    delete _impl;
}

DashTreePtr RenderNodeVisitor::getDashTree()
{
    return _impl->getDashTree();
}

void RenderNodeVisitor::onVisitBegin( const NodeId& nodeId,
                                      const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitBegin( renderNode, state );
}

void RenderNodeVisitor::onVisitEnd( const NodeId& nodeId,
                                    const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitEnd( renderNode, state );
}

void RenderNodeVisitor::onVisitChildrenBegin(
                           const NodeId& nodeId,
                           const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitChildrenBegin( renderNode, state );
}

void RenderNodeVisitor::onVisitChildrenEnd( const NodeId& nodeId,
                                            const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitChildrenEnd( renderNode, state );
}

void RenderNodeVisitor::visit( const NodeId& nodeId,
                             VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    visit( renderNode, state );
}

}


