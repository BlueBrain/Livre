
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "gpuAsyncLoaderBase.h"

#include "../EQ/window.h"

#include <eq/client/system.h>
#ifdef AGL
#  include "aglWindowShared.h"
#endif
#ifdef GLX
#  include "glXWindowShared.h"
#endif

#include <eq/client/cudaContext.h>

#include <ctime>

namespace massVolVis
{

namespace
{
eq::SystemWindow* initSharedContextWindow( eq::Window* wnd,
                                                  eq::ComputeContext** computeCtx )
{
    LBASSERT( wnd );

    // store old drawable of window and set window's drawable to FBO,
    // create another (shared) osWindow and restore original drowable
    const int32_t drawable =
        wnd->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    const int32_t stencil =
        wnd->getIAttribute( eq::Window::IATTR_PLANES_STENCIL );
    wnd->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, eq::OFF );

    eq::Pipe* pipe = wnd->getPipe();
    LBASSERT( pipe );

    const std::string& ws = pipe->getWindowSystem().getName();
    eq::SystemWindow* sharedContextWindow = 0;
    LBINFO << "Using " << ws << " window" << std::endl;
#ifdef GLX
    if( ws == "GLX" )
        sharedContextWindow = new GLXWindowShared( wnd );
#endif
#ifdef AGL
    if( ws == "AGL" )
        sharedContextWindow = new eq::agl::Window( wnd );
#endif
#ifdef WGL
    if( ws == "WGL" )
        sharedContextWindow = new eq::wgl::Window( wnd );
#endif

    if( !sharedContextWindow )
    {
        LBERROR << "Window system " << ws << " not implemented or supported"
                << std::endl;
        LBASSERT( sharedContextWindow );
        return 0;
    }
    LBASSERT( sharedContextWindow );

    if( !sharedContextWindow->configInit( ))
    {
        LBWARN << ws << " window initialization failed: " << std::endl;
        delete sharedContextWindow;
        sharedContextWindow = 0;
        return 0;
    }

    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
    wnd->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, stencil );

    sharedContextWindow->makeCurrent();

/*
    LBASSERT( (*computeCtx) == 0 );
    *computeCtx = new eq::CUDAContext( pipe );

    if( !(*computeCtx)->configInit() )
    {
        LBASSERT( pipe->getError() != eq::ERROR_NONE );
        LBERROR << "GPU Computing context initialization failed: "
                << pipe->getError() << std::endl;
        delete *computeCtx;
    }
    pipe->setComputeContext( *computeCtx );
    *computeCtx = 0;
*/
    LBWARN << "Async fetcher initialization finished" << std::endl;
    return sharedContextWindow;
}


void deleteSharedContextWindow( eq::Window* wnd,
                                eq::SystemWindow**  sharedContextWindow,
                                eq::ComputeContext** computeCtx )
{
    LBWARN << "Deleting shared context" << std::endl;
    if( !sharedContextWindow || !*sharedContextWindow )
        return;

    delete *computeCtx;
    *computeCtx = 0;

    const int32_t drawable =
        wnd->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    (*sharedContextWindow)->configExit(); // mb set window to 0 before that?

    delete *sharedContextWindow;
    *sharedContextWindow = 0;

    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
}
} // namespace


GPUAsyncLoaderBase::GPUAsyncLoaderBase( Window* wnd )
    : lunchbox::Thread()
    , _wnd( wnd )
    , _sharedContextWindow( 0 )
    , _computeCtx( 0 )
{
}


GPUAsyncLoaderBase::~GPUAsyncLoaderBase()
{
    if( _wnd && _sharedContextWindow )
        deleteSharedContextWindow( _wnd, &_sharedContextWindow, &_computeCtx );
}


const GLEWContext* GPUAsyncLoaderBase::glewGetContext() const
{
    return _sharedContextWindow->glewGetContext();
}


/**
 *  Function for creating and holding of shared context.
 */
void GPUAsyncLoaderBase::run()
{
    LBASSERT( !_sharedContextWindow );
    _sharedContextWindow = initSharedContextWindow( _wnd, &_computeCtx );

    onInit();

    if( !getSharedContextWindow() )
        return;

    runLocal();
    cleanup();

    deleteSharedContextWindow( _wnd, &_sharedContextWindow, &_computeCtx );
}

} //namespace massVolVis
