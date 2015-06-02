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

#ifndef _DashRenderStatus_h_
#define _DashRenderStatus_h_

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
 * The DashRenderStatus class enable access to render status between different dash contexts.
 */
class DashRenderStatus : public boost::noncopyable
{

public:

    DashRenderStatus();

    /**
     * @return The dash node.
     */
    dash::NodePtr getDashNode() const { return _dashNode; }

    /**
     * @return The tree priority ( data, texture data or texture ). Used at data loading stages.
     */
    LoadPriority getLoadPriority() const;

    /**
     * Sets the tree priority ( data, texture data or texture ). Used at data loading stages.
     * @param priority Tree access priority. It can be data, texture data or texture.
     */
    void setLoadPriority( const LoadPriority priority );

    /**
     * @return The current frame id.
     */
    uint64_t getFrameID() const;

    /**
     * Sets the frame id of the node.
     * @param frameId The id for the rendered frame.
     */
    void setFrameID( const uint64_t frameId );

    /**
     * @return The current rendering frustum.
     */
    Frustum getFrustum() const;

    /**
     * Sets the current rendering frustum.
     * @param frustum Current rendering frustum.
     */
    void setFrustum( const Frustum& frustum );

    /**
     * @return The thread command issued.
     */
    ThreadOperation getThreadOp() const;

    /**
     * Issues a thread command ( stop, pause, continue etc ). Helpful for pipeline.
     * @param op Thread operation to be issued.
     */
    void setThreadOp( const ThreadOperation op );

private:

    template< class T >
    T _getAttribute( const uint32_t nodeType ) const;

    dash::NodePtr _dashNode;
};

}
#endif // _DashRenderStatus_h_
