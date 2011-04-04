About perftest
--------------
Perftest is a low-level benchmark tool for GL and D3D9.


Command line parameters
----------------------
Run "perfest -help" to see a list of command line options and keyboard toggles.


Running all tests
-----------------
On any system with a bash shell, you can run "run_tests.sh" to loop through all
off the different graphics options.  This will dump a comma-separated list of
values to stdout which can be imported directly into a spreadsheet program.

Example:
  ./run_test.sh > nvidia9600_MAC_10.6.3.csv

will run all of the tests, and dump the results to the file
"nvidia9600_MAC_10.6.3.csv"


Running on Windows
------------------
You can run the standalone "perftest.exe" on Windows as any other program.
However, to run all of the automated tests, you'll need to install either
Cygwin's base installation or MinGW in order to get a bash shell.


Running on Mac
--------------
Mac has an extra command line option you can toggle, "-use_multithreaded_gl".
This enables the multi-threaded GL engine, which should provide a framerate
boost in some cases.  This is not toggled by default in the run_tests.sh
script, so to enable this, type:

  ./run_tests.sh -use_multithreaded_gl > output_file.csv


Compiling the source code
-------------------------
On Mac and Linux, you need glut or freeglut installed.
To cross-compile for Windows, you need to install mingw32.
Then, type:

  make -f Makefile.????

where ???? is your target OS: MacOS, linux, or mingw32.
