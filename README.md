![logo](https://github.com/Phibedy/LMS/blob/master/lms_banner.jpg)

[![Build Status](https://travis-ci.org/lms-org/LMS.svg?branch=master)](https://travis-ci.org/lms-org/LMS)

LMS is a light-weight, highly modular and efficient framework capable of controlling
near real-time systems like robots, vehicles or airplanes. It runs on Unix-like systems,
Windows support is in progress. LMS manages dynamically loaded modules and data flow for you.

## Getting Started
Install all required packages on your system. For Ubuntu Linux this would be as follows:

```sh
sudo apt-get update
sudo apt-get install build-essential make git cmake
```

You may need additional packages depending on the use case or the required libraries.

Clone and install one of config repositories owned by [lms-org](https://github.com/lms-org).

```sh
git clone https://github.com/lms-org/config_dependency_test.git
cd config_dependency_test
mkdir build
cd build
cmake ..
make
```

Now start lms by simply executing its binary `./lms`. If you need further assistance consider
looking at the [wiki](https://github.com/Phibedy/LMS/wiki) or contact one
of our developers by mail.

## Known Issues
- Signalhandler doesn't work properly.
- Doesn't fully work on windows (it compiles).

## Special thanks go to
- ngladitz from #cmake, who helped me a lot getting into cmake :)

## License
Copyright 2015 LMS Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
