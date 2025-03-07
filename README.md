# Proctoring Lib #
- Gives easier start since finding the libraries needed to interact with winAPI and using them can be hard sometimes.
- Not sure if there are memory leaks


# main.py #
- backend

# visual studio 2022 Solution #
- Link all the libraries in the pragma once statements in Source.cpp
- ### Linker > Input > additional libraries
  Add these:
  - Gdiplus.lib
  - mf.lib
  - mfplat.lib
  - mfuuid.lib
  - ole32.lib
  - mfreadwrite.lib
  
## socket.h ##
- Sockets needed for sending screen shots and webcam data (Also audio later, not sure if it being recorded)

## Source.cpp ##
- code for taking screen shots programmatically
- code for filming webcame. (Might need to run visual studio in admin mode)

### Goal ###
- Take screen shots every 10 seconds or so and upload it.
- Then they know what you are doing.
