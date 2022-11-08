# living-looper-juce
VST/AU Plugin for the Living Looper


### Building the C++ project
- Add Juce 6.1.4 as a directory called JUCE in the main working directory or clone the repository.
  - `ln -s <JUCE-DIR> .`  OR
  - `git clone --depth 1 --branch 6.1.4 https://github.com/juce-framework/JUCE`
- Use cmake to build. Tested with cmake X.X.X, clang X.X.X, Xcode 13.4.1 on MacOS 12.5.1 (arm). This will download the PyTorch libraries.
  - `mkdir build; cd build`
  - `cmake .. -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build . --config Release -j 4` or  `cmake -G Xcode -B build`


### Acknowledgements

Antoine Caillon's RAVE autoencoder: https://github.com/acids-ircam/RAVE

Andrew Fyfe and Bogdan Teleaga's RAVE-audition JUCE plugin: https://github.com/Fyfe93/RAVE-audition
