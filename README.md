![logo](https://github.com/Phibedy/LMS/blob/master/lms_banner.jpg)

LMS is a.... //TODO
will be a cross-plattform system that allows easy module-handling and sharing.

#####Notice:
(Work in progress) = there is no stable version and it might doesn't work at all :)

###Getting Started
//TODO links and more
  * [Have a look at the wiki](https://github.com/Phibedy/LMS/wiki)
  * Work with it
    * Get source
    * Check out usefull modules
    * Write own modules
  * Contribute

###Featured Libraries
 * [Sockets (Client & Server)](https://github.com/Phibedy/SocketConnection) (Work in progress)
 * [Ogre-Binding](https://github.com/Phibedy/ogrea_binding)
 * [Imaging](https://github.com/syxolk/imaging)
 * [Cereal](https://github.com/syxolk/cereal)
 * [sense_link](https://github.com/Bitfroest/sense_link)
 * [gamepad_lib](https://github.com/Phibedy/gamepad_lib)
 * BlueTooth-Adapter (not implemented yet)
 * OpenCV (not implemented yet)
 * Audio
   * Some basic audio library (not implemented yet)
   * text to speech
   * speech to text
 

###Featured Modules
 * [Scheduler](https://github.com/Phibedy/Scheduler)
 * [Camera importer](https://github.com/Phibedy/camera)
 * [Ogre window handler](https://github.com/Phibedy/ogre_window_manager)
 * [camera renderer](https://github.com/Phibedy/camera_renderer)
 * [socket data](https://github.com/Phibedy/socket_data) (work in progress)
 * [socket connection](https://github.com/Phibedy/socket_connection)
 * [virtual_sense_link](https://github.com/syxolk/virtual_sense_link)
 * [virtual_controller](https://github.com/Bitfroest/virtual_controller)
 * [image_converter](https://github.com/syxolk/image_converter)
 * [image_logger](https://github.com/syxolk/image_logger)
 * [image_loader](https://github.com/syxolk/image_loader)
 * [usb_sense_link](https://github.com/Bitfroest/usb_sense_link)
 * CommandHandler (not implemented yet - may become a library)
 * SocketChat (not implemented)

###Extras
 * [Senseboard_16](https://github.com/Bitfroest/Senseboard_16) - Arduino implementation for *sense_link*

###Downloads SA
  * Get the source from github :P
  * //TODO install binaries


###License
  LMS is licensed under [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0.html)

###TODO
  * If first version is finished -> Exclude modules for testing
  * More docs
  * License
   * Add license-header to files
   * Add contributer-agreement-file


###Known bugs
  * Signalhandler doesn't work properly
  * Doesn't fully work on windows (it compiles)

###Thoughts:
  * We could use [Boost] http://www.boost.org/doc/libs/1_57_0/index.html
  * Maybe it's we should generate the loadConfig from the CMakeLists of modules (Don't think it would be worth the effort atm)
  * [Arduino support](http://stackoverflow.com/questions/16224746/how-to-use-c11-to-program-the-arduino)
  * [Matlab](http://de.mathworks.com/help/matlab/calling-matlab-engine-from-c-c-and-fortran-programs.html) test/support 
  * [ROS](http://www.ros.org/) wrapper (run ros modules)

###Special thanks go to
  *  ngladitz from #cmake, who helped me a lot getting into cmake :)
