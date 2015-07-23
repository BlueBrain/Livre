Changelog {#Changelog}
=========

# Release 0.4 (git master) {#Release040}

* [#90](https://github.com/BlueBrain/Livre/pull/90):
  Fix [#75](https://github.com/BlueBrain/Livre/issues/73):
  Do not block application while waiting for data in asynchronous mode
* [#86](https://github.com/BlueBrain/Livre/pull/86):
  Removed obsolete limitation to open only regular power-of-two volumes
* [#84](https://github.com/BlueBrain/Livre/pull/84):
  Cache statistics are printed when showing statistics (press 's' key)
* [#84](https://github.com/BlueBrain/Livre/pull/84):
  Fix [#73](https://github.com/BlueBrain/Livre/issues/73): Merge data- and
  textureDataCache, commandline options for changing the cache sizes are now
  --gpu-cache-mem (instead of --texture-cache-mem) and --cpu-cache-mem (instead
  of --data-cache-mem and --texturedata-cache-mem)
* [#78](https://github.com/BlueBrain/Livre/pull/78):
  New command line parameter to load a transfer function from file (".1dt" file
  extension, based on the format used by ImageVis3D)
* [#75](https://github.com/BlueBrain/Livre/pull/75):
  Separate ZeroEQ communication to class zeq::Communicator
* [#88](https://github.com/BlueBrain/Livre/pull/88):
  Fix [LIV-157)](https://bbpteam.epfl.ch/project/issues/browse/LIV-157)
  rendering of overlapping LOD nodes from different levels

# Release 0.3 (2015-07-07) {#Release030}

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

# Release 0.2 (2014-10-15) {#Release020}

* BBP internal release
