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
 * The LoadPriority enum is used to state the load priority.
 */
enum LoadPriority
{
    LP_NONE, //!< No priority.
    LP_VISIBLE, //!< Visibles first.
    LP_TEXTURE, //!< Textures first.
    LP_DATA, //!< Data first.
    LP_ALL //!< Load all.
};

/**
 * The ThreadOperation enum is used to send commands to the threads.
 */
enum ThreadOperation
{
    TO_NONE, //!< Do nothing.
    TO_PAUSE, //!< Pause.
    TO_EXIT, //!< Exit.
    TO_CONTINUE //!< Resume.
};

/**
 * The data request type.
 */
enum RequestType
{
    DRT_VISIBILE    = 0x00000001,
    DRT_DATA        = 0x00000002,
    DRT_TEXTURE     = 0x00000004
};

/**
 * The DashRenderNode class is an helper class to struct and access the dash nodes attributes.
 */
class DashRenderNode
{

public:

    /**
     * @param dashNode The dash node to access.
     * @warning Dash node has to be initialized before with \see DashRenderNode::initializeDashNode.
     */
    DashRenderNode( dash::NodePtr dashNode );

    /**
     * @return The dash node.
     */
    dash::NodePtr getDashNode( ) const { return dashNode_; }

    /**
     * @return The parent of the dash node, if none the parent node is empty.
     */
    DashRenderNode getParent( ) const;

    /**
     * @return The LODNode.
     */
    const LODNode& getLODNode( ) const;

    /**
     * @return \see TextureDataObject, if not initialized in the tree, the returned object is \see EmptyCacheObject.
     */
    ConstCacheObjectPtr getTextureDataObject( ) const;

    /**
     * @return \see TextureObject, if not initialized in the tree, the returned object is \see EmptyCacheObject.
     */
    ConstCacheObjectPtr getTextureObject( ) const;

    /**
     * @return True, if object is visible.
     * @warning This condition is set from outside of the object.
     */
    bool isVisible( ) const;

    /**
     * @return True, if data for the rendering is requested from disk.
     * @warning This condition is set from outside of the object.
     */
    bool isDataRequested( ) const;


    /**
     * @return True, if texture for the rendering is requested.
     * @warning This condition is set from outside of the object.
     */
    bool isTextureRequested( ) const;

    /**
     * Sets the \see LODNode for the dash node.
     * @param node Sets the \see which is an abstract rendering information ( size of block, position, etcc )
     */
    void setLODNode( const LODNode& node );

    /**
     * Sets the texture data object.
     * @param textureData Sets the texture data object, if \see EmptyCacheObject is given the reference count decreases
     * for the corresponding \see CacheObject, therefore \see Cache can clean the object.
     */
    void setTextureDataObject( ConstCacheObjectPtr textureData );

    /**
     * Sets the texture object.
     * @param texture Sets the texture object, if \see EmptyCacheObject is given the reference count decreases
     * for the corresponding \see CacheObject, therefore \see Cache can clean the object.
     */
    void setTextureObject( ConstCacheObjectPtr texture );

    /**
     * Sets visibilty of node.
     * @param visibility If parameter is true, node is visible.
     */
    void setVisible( bool visibility );

    /**
     * Sets data request for the node.
     * @param isRequested If parameter is true, node data is requested.
     */
    void setDataRequested( bool isRequested );

    /**
     * Sets data texture request for the node.
     * @param isRequested If parameter is true, node texture is requested.
     */
    void setTextureRequested( bool isRequested );

   /**
     * Generalizes the set request methods.
     * @param requestType The request type for @see CacheObject. ( data, texture data or texture )
     * @param isRequested If parameter is true, the requestType is requested.
     */
    void setRequested( RequestType requestType, bool isRequested );

    /**
     * @return True if dash node has a parent.
     */
    bool hasParent( ) const;

    /**
     * Initializes an empty dash node with attributes.
     * @param dashNode Input dash node to initialize.
     */
    static void initializeDashNode( dash::NodePtr dashNode );

    /**
     * Initializes the root node of the tree with some root node specific attributes.
     * @param dashNode Input dash node to initialize.
     */
    static void initializeRootNode( dash::NodePtr dashNode );

    /**
     * @return The tree priority ( data, texture data or texture ). Used at data loading stages.
     * @warning Valid only for the root node.
     */
    LoadPriority rootGetLoadPriority_( ) const;

    /**
     * Sets the tree priority ( data, texture data or texture ). Used at data loading stages.
     * @param priority Tree access priority. It can be data, texture data or texture.
     * @warning Valid only for the root node.
     */
    void rootSetTreePriority_( LoadPriority priority );

    /**
     * @return The current frame id.
     * @warning Valid only for the root node.
     */
    uint64_t rootGetFrameID_( ) const;

    /**
     * Sets the frame id of the node.
     * @param frameId The id for the rendered frame.
     * @warning Valid only for the root node.
     */
    void rootSetFrameID_( const uint64_t frameId );

    /**
     * @return The current rendering frustum.
     * @warning Valid only for the root node.
     */
    Frustum rootGetFrustum_( ) const;

    /**
     * Sets the current rendering frustum.
     * @param frustum Current rendering frustum.
     * @warning Valid only for the root node.
     */
    void rootSetFrustum_( const Frustum& frustum );

    /**
     * @return The thread command issued.
     * @warning Valid only for the root node.
     */
    ThreadOperation rootGetThreadOp_( ) const;

    /**
     * Issues a thread command ( stop, pause, continue etc ). Helpful for pipeline.
     * @param op Thread operation to be issued.
     * @warning Valid only for the root node.
     */
    void rootSetThreadOp_( const ThreadOperation op );

private:

    template< class T >
    T getAttribute_( const uint32_t nodeType ) const;

    dash::NodePtr dashNode_;
};

}
#endif // _DashRenderNode_h_
