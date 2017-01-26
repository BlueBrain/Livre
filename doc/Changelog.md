Changelog {#Changelog}
=========

# git master

* [#375](https://github.com/BlueBrain/Livre/pull/375):
  Add livre::DataSource::handles()
* [#373](https://github.com/BlueBrain/Livre/pull/373):
  Fix concurrent send & receive from livreGUI, which lead to crashes on GUI
  and/or connected clients
* [#372](https://github.com/BlueBrain/Livre/pull/372):
  Fix crash with histogram not being ready in synchronous mode while moving clip
  planes
* [#368](https://github.com/BlueBrain/Livre/pull/368):
  Make "Show axes" available in livreGUI and python API
* [#367](https://github.com/BlueBrain/Livre/pull/367):
  Add DataSourcePlugin::finishGL() to release OpenGL resources that are held
  by the plugin, e.g. Cubist
* [#363](https://github.com/BlueBrain/Livre/pull/363):
  Add detailed information on the volume URIs to help, wrap help to
  current terminal size, remove config file support
* [#365](https://github.com/BlueBrain/Livre/pull/365):
  Introduce livre::Engine to allow integration into Brayns
* [#364](https://github.com/BlueBrain/Livre/pull/364):
  Fix multi-node setups
* [#359](https://github.com/BlueBrain/Livre/pull/359):
  Fix exit crash
* [#358](https://github.com/BlueBrain/Livre/pull/358):
  Fix bugs occuring with 16-bits and anisotropic raw volumes.
* [#356](https://github.com/BlueBrain/Livre/pull/356):
  Implement element downscaling of raw data volumes

# Release 0.7 (09-12-2016)

* [#353](https://github.com/BlueBrain/Livre/pull/353):
  Allow specification of raw volumes without schema
* [#352](https://github.com/BlueBrain/Livre/pull/352):
  Fix missing redraw after expose.
  * Fix the %300 CPU usage.
* [#346](https://github.com/BlueBrain/Livre/pull/346):
  Implement axis and legend for full volume bounding box.
* [#333](https://github.com/BlueBrain/Livre/pull/333):
  Add animation-fps parameter to limit the animation frame rate.
* [#331](https://github.com/BlueBrain/Livre/pull/331):
  Added support for rendering high-resolution volumes at full resolution.
* [#320](https://github.com/BlueBrain/Livre/pull/3):
  Added support for floating point volumes.
* [#318](https://github.com/BlueBrain/Livre/pull/318):
  Added support for NRRD and Raw volumes.
* [#317](https://github.com/BlueBrain/Livre/pull/317):
  Added purge support for cache.
* [#313](https://github.com/BlueBrain/Livre/pull/313):
  Fixed the camera synchronization between applications.
  Replaced the data bounding box by the data to Livre spaces
  transform matrix computed by the datasource for camera
  synchronization.
  * Fixed resolution printing.

# Release 0.6 (30-06-2016)

* [#315](https://github.com/BlueBrain/Livre/pull/315):
  Clip planes support added. User can adjust the region
  of interest.
* [#302](https://github.com/BlueBrain/Livre/pull/302):
  Histogram visualisation added to GUI.
* [#301](https://github.com/BlueBrain/Livre/pull/301):
  Histogram computation and ZeroEQ publishing.
* [#303](https://github.com/BlueBrain/Livre/pull/303):
  Old renderer is revived for OSX. Fix #296
* [#256](https://github.com/BlueBrain/Livre/pull/256):
  A new architecture for loading data, executing rendering
  is implemented. There are preformance gains 2x to 10x
  in frame rate and data upload.
* [#291](https://github.com/BlueBrain/Livre/pull/291):
  Livre uses OpenGL 4.2 for rendering ( removed copying-from-to
  CPU memory )
* [#265](https://github.com/BlueBrain/Livre/pull/265):
  Livre uses the following ZeroBuf events: LookOut, imageJPEG, Frame,
  VolumeRendererParameters, LookupTable1D. LivreGUI can load and save
  the transfer function in .tf .1dt .lbb .lba formats
* [#281](https://github.com/BlueBrain/Livre/pull/281):
  LivreGUI: Show progress bar for ongoing operations
* [#279](https://github.com/BlueBrain/Livre/pull/279):
  Adapt to the renaming of zeq to ZeroEQ

# Release 0.5 (06-04-2016)

* [#270](https://github.com/BlueBrain/Livre/pull/270):
  Fix #113. SSE selection algorithm is simplified
  Fix #255, #221 Regression on visible selection fixed
  Default camera position is modified
* [#257](https://github.com/BlueBrain/Livre/pull/257):
  The schema is optional with uvf files
* [#236](https://github.com/BlueBrain/Livre/pull/236):
  Fix #225. Adapted shader alpha correction to the minimum number of samples
* [#238](https://github.com/BlueBrain/Livre/pull/238):
  Fix #230. The caches reporting different sizes
* [#234](https://github.com/BlueBrain/Livre/pull/234):
  Fix #227. Rendering set generation is only done once
* [#220](https://github.com/BlueBrain/Livre/pull/220):
  LivreGUI: Add renderer parameters panel
* [#219](https://github.com/BlueBrain/Livre/pull/219):
  LivreGUI: Editable frame range in animation panel

# Release 0.4 (09-11-2015)

* [#213](https://github.com/BlueBrain/Livre/pull/213):
  Fix [#207](https://github.com/BlueBrain/Livre/issues/207).
  Frame range needs to be open at the end, as the documentation says.
  Add FrameUtils class that operates with a valid frame range (based on
  user-specified boundaries) and contains methods to compute current and
  next frame.
* [#217](https://github.com/BlueBrain/Livre/pull/217):
  Improve #177 by moving the first ray's samples on the boundary of the volume
  AABB
* [#188](https://github.com/BlueBrain/Livre/pull/188):
  Fix #180, rendering has artifacts when the camera is inside
  the volume.
* [#185](https://github.com/BlueBrain/Livre/pull/185):
  Fix #179, the rendering is updated according to the
  latest frustum
* [#181](https://github.com/BlueBrain/Livre/pull/181):
  Fix #114, rendering artefacts with screen-aligned bricks caused by
  division by zero with axis aligned rays in the shader
* [#177](https://github.com/BlueBrain/Livre/pull/176):
  Ray sampling is adapted to the volume size to prevent
  artifacts
* [#168](https://github.com/BlueBrain/Livre/pull/168):
  Use simulation widget (from the MonsteerQt library) in livreGUI
* [#165](https://github.com/BlueBrain/Livre/pull/165):
  UVF time support added
* [#145](https://github.com/BlueBrain/Livre/pull/145):
  Added multisampling per pixel and sparsity parameter for memory datasource
* [#105](https://github.com/BlueBrain/Livre/pull/105):
  Fix 32 bit issue with REST jpeg image request
* [#101](https://github.com/BlueBrain/Livre/pull/101):
  Implement event-driven rendering
* [#75](https://github.com/BlueBrain/Livre/issues/73):
  Fix blocked application while waiting for data in asynchronous mode
* [#86](https://github.com/BlueBrain/Livre/pull/86):
  Removed obsolete limitation to open only regular power-of-two volumes
* [#84](https://github.com/BlueBrain/Livre/pull/84):
  Cache statistics are printed when showing statistics (press 's' key)
* [#73](https://github.com/BlueBrain/Livre/issues/73):
  Merge dataCache and textureDataCache
* [#78](https://github.com/BlueBrain/Livre/pull/78):
  New command line parameter to load a transfer function from file (".1dt" file
  extension, based on the format used by ImageVis3D)
* [#75](https://github.com/BlueBrain/Livre/pull/75):
  Separate ZeroEQ communication to class zeq::Communicator
* [#88](https://github.com/BlueBrain/Livre/pull/88):
  Fix [LIV-157](https://bbpteam.epfl.ch/project/issues/browse/LIV-157)
  rendering of overlapping LOD nodes from different levels

# Release 0.3 (07-07-2015)

* [#37](https://github.com/BlueBrain/Livre/pull/37):
  New command line option to enable synchronous mode (wait until every block has
  been processed before rendering them)
* [#33](https://github.com/BlueBrain/Livre/pull/33):
  New command line option to enable ZeroEQ camera synchronization
* [#44](https://github.com/BlueBrain/Livre/pull/44):
  New command line option to specify a range of frames to render from the
  command line
* [#57](https://github.com/BlueBrain/Livre/pull/57):
  New command line parameter to set the camera lookAt vector to define its
  orientation
  * New command line parameter to set camera position
* [#17](https://github.com/BlueBrain/Livre/pull/17):
  New command line parameters for RESTBridge (hostname, port, ZeroEQ schema)
* [#53](https://github.com/BlueBrain/Livre/pull/53):
  New command line parameters to specify the minimum and maximum levels of
  detail to render
  * New heartbeat messaging mechanism
* [#43](https://github.com/BlueBrain/Livre/pull/43):
  New livre_batch.py script for offline image series rendering
  * New remote data source
* [#67](https://github.com/BlueBrain/Livre/pull/67):
  Frames can now be written to disk as PNG files
  * Add perf unit test for remote data source
  * Add response to Vocabulary requests
  * Add support to publish frames through ZeroEQ
* [#44](https://github.com/BlueBrain/Livre/pull/44):
  Add time support, implementing a new animation mode that triggers a new frame
  after the rendering of the previous one is finished
* [#43](https://github.com/BlueBrain/Livre/pull/43):
  Fix: do not setup ZeroEQ subscribers for REST if not requested
* [#56](https://github.com/BlueBrain/Livre/pull/56):
  Fix a problem that made the texture cache not to unload unused elements
  * Fix broken memory mapped reading in UVF
  * Fix GLEW_MX linking
* [#45](https://github.com/BlueBrain/Livre/pull/45):
  Fix the "Empty servus implementation" exception that was raised when zeroconf
  was not available
* [#58](https://github.com/BlueBrain/Livre/pull/58):
  Fix the LOD computation based on the screen height
* [#33](https://github.com/BlueBrain/Livre/pull/33):
  General cleanup of command line parameters
* [#22](https://github.com/BlueBrain/Livre/pull/22):
  Graceful exit when ZeroEQ EXIT event is received
  * Improve glslshader/raycastrenderer error handling and API
* [#28](https://github.com/BlueBrain/Livre/pull/28):
  Improve performance and memory usage through a new implicit, flat octree
  implementation
* [#48](https://github.com/BlueBrain/Livre/pull/48):
  Improve rendering performance through frustum culling
  * Optimization: use lunchbox::Buffer instead of std::vector in MemoryUnit and
    MemoryDataSource (+35% speed in remote datasource test)
* [#34](https://github.com/BlueBrain/Livre/pull/34):
  Refactor the code to setup the data and texture UploadProcessors, removing
  race conditions and fixing crashes (at exit and with multiple local GPUs) in
  the process
* [#35](https://github.com/BlueBrain/Livre/pull/35)
  [#36](https://github.com/BlueBrain/Livre/pull/36):
  Several fixes in the tree creation and the LOD-selection algorithm
* [#23](https://github.com/BlueBrain/Livre/pull/23):
  Upgrade to Qt5 for the transfer function editor
* [#70](https://github.com/BlueBrain/Livre/pull/70):
  Use one GPU by default
  * Use ZeroEQ for the transfer function editor GUI
* [#70](https://github.com/BlueBrain/Livre/pull/70):
  Window title has been improved to show the application name and its version
  number

# Release 0.2 (15-10-2014)

* BBP internal release
