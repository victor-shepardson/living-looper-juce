# living-looper-juce
VST/AU Plugin for the Living Looper


### Building the C++ project
- Add Juce 6.1.4 as a directory called JUCE in the main working directory or clone the repository.
  - `ln -s <JUCE-DIR> .`  OR
  - `git clone --depth 1 --branch 6.1.4 https://github.com/juce-framework/JUCE`
- Use cmake to build. Tested with cmake 3.23.2, clang 13.1.6, Xcode 13.4.1 on MacOS 12.5.1 (arm). (NOTE: I am using libtorch acquired with `brew install pytorch`)
  - `mkdir build; cd build`
  - `cmake .. -DCMAKE_BUILD_TYPE=Release -DTORCH_PATH=/opt/homebrew/Cellar/pytorch/1.12.1`
  - `cmake --build . --config Release -j 4` or  `cmake -G Xcode -B build`


### Acknowledgements

Antoine Caillon's RAVE autoencoder: https://github.com/acids-ircam/RAVE

Andrew Fyfe and Bogdan Teleaga's RAVE-audition JUCE plugin: https://github.com/Fyfe93/RAVE-audition
