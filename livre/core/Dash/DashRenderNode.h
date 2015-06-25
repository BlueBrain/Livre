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

#ifndef _DashRenderNode_h_
#define _DashRenderNode_h_

#include <dash/dash.h>

#include <livre/core/types.h>

#include <livre/core/Visitor/VisitState.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Render/Frustum.h>
#include <livre/core/Cache/EmptyCacheObject.h>

namespace livre
{

/**
 * The DashRenderNode class is an helper class to access the dash nodes attributes.
 */
class DashRenderNode
{

public:

    /**
     * @param dashNode The dash node to access.
     */
    DashRenderNode( dash::NodePtr dashNode );

    /**
     * @return The dash node.
     */
    dash::NodePtr getDashNode( ) const { return _dashNode; }

    /**
     * @return The LODNode.
     */
    const LODNode& getLODNode( ) const;

    /**
     * @return \see TextureDataObject, if not initialized in the tree, the returned object
     * is \see EmptyCacheObject.
     */
    ConstCacheObjectPtr getTextureDataObject() const;

    /**
     * @return \see TextureObject, if not initialized in the tree, the returned object is
     * \see EmptyCacheObject.
     */
    ConstCacheObjectPtr getTextureObject() const;

    /**
     * @return True, if object is visible.
     * @warning This condition is set from outside of the object.
     */
    bool isVisible() const;

    /**
     * @return True, if object is in frustum.
     * @warning This condition is set from outside of the object.
     */
    bool isInFrustum() const;

    /**
     * Sets the \see LODNode for the dash node.
     * @param node Sets the \see which is an abstract rendering information ( size of block, position, etcc )
     */
    void setLODNode( const LODNode& node );

    /**
     * Sets the texture data object.
     * @param textureData Sets the texture data object, if \see EmptyCacheObject is
     * given the reference count decreases
     * for the corresponding \see CacheObject, therefore \see Cache can clean the object.
     */
    void setTextureDataObject( ConstCacheObjectPtr textureData );

    /**
     * Sets the texture object.
     * @param texture Sets the texture object, if \see EmptyCacheObject is given
     * the reference count decreases
     * for the corresponding \see CacheObject, therefore \see Cache can clean the object.
     */
    void setTextureObject( ConstCacheObjectPtr texture );

    /**
     * Sets visibilty of node.
     * @param visibility If parameter is true, node is visible.
     */
    void setVisible( bool visibility );

    /**
     * Sets frustum status of node.
     * @param visibility If parameter is true, node is visible.
     */
    void setInFrustum( bool visibility );

    /**
     * Initializes an empty dash node with attributes.
     * @param dashNode Input dash node to initialize.
     */
    static void initializeDashNode( dash::NodePtr dashNode );

private:

    template< class T >
    T getAttribute_( const uint32_t nodeType ) const;

    dash::NodePtr _dashNode;
};

}
#endif // _DashRenderNode_h_
