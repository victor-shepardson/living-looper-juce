# RAVE-audition
VST/AU Plugin for Auditioning RAVE Models in Real-time

### Building the C++ project
- Add Juce 6.0 as a directory called JUCE in the main working directory or clone the repository.
  - `ln -s <JUCE-DIR> .`  OR
  - `git clone --depth 1 --branch 6.0.8 https://github.com/juce-framework/JUCE`
- Use cmake to build. Tested with cmake 3.20, clang 12.0.5 on MacOS 11.2.3. This will download the PyTorch libraries.
  - `mkdir build; cd build`
  - `cmake .. -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build . --config Release -j 4`
