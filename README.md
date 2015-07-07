![logo](https://github.com/Phibedy/LMS/blob/master/lms_banner.jpg)

LMS is a ... //TODO
will be a cross-plattform system that allows easy module-handling and sharing.

#####Notice:
(Work in progress) = there is no stable version and it might doesn't work at all :)

###Getting Started
//TODO links and more
  * [Have a look at the wiki](https://github.com/Phibedy/LMS/wiki)
  * Work with it
    * Get source
    * Check out useful modules
    * Write own modules
  * [Contribute](https://github.com/Phibedy/LMS/blob/master/CONTRIBUTING.md)

###Featured Libraries
 * [ogre_binding](https://github.com/Phibedy/ogre_binding)
 * [imaging](https://github.com/syxolk/imaging)
 * [cereal](https://github.com/syxolk/cereal)
 * [sense_link](https://github.com/Bitfroest/sense_link)
 * [gamepad_lib](https://github.com/Phibedy/gamepad_lib)
 * [socket_connection](https://github.com/Phibedy/socket_connection)
 * [sensor_utils](https://github.com/Phibedy/sensor_utils)
 * [street_environment](https://github.com/Phibedy/street_environment)
 * [imaging_detection](https://github.com/Phibedy/imaging_detection)
 * [math_lib](https://github.com/syxolk/math_lib)
 * [googletest_lib](https://github.com/syxolk/googletest_lib)
 * BlueTooth-Adapter (not implemented yet)
 * OpenCV (not implemented yet)
 * Audio
   * Some basic audio library (not implemented yet)
   * text to speech
   * speech to text
 

###Featured Modules
 * [Scheduler](https://github.com/Phibedy/Scheduler)
 * [camera_importer](https://github.com/Phibedy/camera_importer)
 * [ogre_window_manager](https://github.com/Phibedy/ogre_window_manager)
 * [camera_renderer](https://github.com/Phibedy/camera_renderer)
 * [socket_data](https://github.com/Phibedy/socket_data) (work in progress)
 * [virtual_sense_link](https://github.com/syxolk/virtual_sense_link)
 * [virtual_controller](https://github.com/Bitfroest/virtual_controller)
 * [image_converter](https://github.com/syxolk/image_converter)
 * [image_logger](https://github.com/syxolk/image_logger)
 * [image_loader](https://github.com/syxolk/image_loader)
 * [usb_sense_link](https://github.com/Bitfroest/usb_sense_link)
 * [ueye_importer](https://github.com/Phibedy/ueye_importer)
 * [importer_senseboard2015](https://github.com/Phibedy/importer_senseboard2015)
 * [image_renderer](https://github.com/Phibedy/image_renderer)
 * [image_hint_worker](https://github.com/Phibedy/image_hint_worker)
 * [car_controller](https://github.com/Phibedy/car_controller)
 * [image_hint_merger](https://github.com/Phibedy/image_hint_merger)
 * [gamepad_controller](https://github.com/Phibedy/gamepad_controller)
 * [sensor_logger](https://github.com/Bitfroest/sensor_logger)
 * [car_to_sense_link](https://github.com/syxolk/car_to_sense_link)
 * [image_hint_transformer](https://github.com/syxolk/image_hint_transformer)
 * [channel_serializer](https://github.com/syxolk/channel_serializer)
 * [image_proxy](https://github.com/syxolk/image_proxy)
 * [image_persistent](https://github.com/syxolk/image_persistent)
 * [ogre_input](https://github.com/syxolk/ogre_input)
 * [gamepad_visualizer](https://github.com/syxolk/gamepad_visualizer)
 * [virtual_sense_link](https://github.com/syxolk/virtual_sense_link)
 * [trajectory_point_controller](https://github.com/Corni33/trajectory_point_controller)
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
  * We could use [Boost](http://www.boost.org/doc/libs/1_57_0/index.html)
  * Maybe it's we should generate the loadConfig from the CMakeLists of modules (Don't think it would be worth the effort atm)
  * [Arduino support](http://stackoverflow.com/questions/16224746/how-to-use-c11-to-program-the-arduino)
  * [Matlab](http://de.mathworks.com/help/matlab/calling-matlab-engine-from-c-c-and-fortran-programs.html) test/support 
  * [ROS](http://www.ros.org/) wrapper (run ros modules)

###Special thanks go to
  *  ngladitz from #cmake, who helped me a lot getting into cmake :)
