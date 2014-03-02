XNEOTERICS, v0.90

xNeoterics is an old program that I wrote as to introduce myself to
neural networks and X-Windows programming. It is a port of the neat
little Macintosh alife program written in 1995, "Neoterics" by Kevin
Coble, from the Macintosh to X-Windows.  It is a cellular automaton
simulation containing agents with fixed-structure neural net brains
that process input signals into responses allowing the agents to
forage and interact in a changing environment of resources. The nets'
weights and thresholds evolve via standard genetic operators
(mutation, crossover, etc.) while they try to survive and reproduce by
ingesting enough "energy" to survive.

For all of the original info about the structure of the nnets and the
evolution, please see the original Neoterics documentation in the
archives at http://surf.de.uu.net/zooland/download/packages/neoterics/

Other than porting this to X-Windows using the (now LGPL) XForms
library (required to compile), I added a large number of improvements
and enhancements that: (1) significantly speed up the simulation, (2)
add more features and controls, and (3) enable a more thorough
investigation of the effects of various environmental parameters on
the evolution.

TO COMPILE:

Download and install the open source v1.0 version of the xforms
library (get the source or binary packages from
ftp://ftp.lyx.org/pub/lyx/contrib/, or for Mac OSX, use Fink).

Edit the Makefile if necessary (probably not necessary).

Type "make" to build the "neo" binary.

TO RUN:

"neo -h" lists the various command-line options, most of which can
also be modified during runtime using the "Options" dialog:

-A: do not allow asexual reproduction
-S: do not allow sexual reproduction
-H: do not give the creatures a head start
-K: keep a minimum population at desired level (default is zero)
-U: use remaining survivors to clone if at minimum population (for -K)
-s: set terrain size (default is 128)
-c: set display scale (default is 7)
-p: set default probability of mutation (default is 0.008000)
-P: set default probability of crossover (default is 0.002000)
-I: set default probability of insertions/deletions (default is 0.002000)
-f: set number of initial food locations (default is 2000)
-F: set number of initial meat locations (default is 1000)
-m: set maximum population (default is 1000)
-i: set initial population (default is 200)
-a: set age factor (default is 9)
-d: set carcass decay rate (default is 16384)
-w: set waste decay rate (default is 50)
-W: set poison decay rate (default is 50)
-D: do not start off updating the display (faster)
-R: do not show the display at all (for running remotely or on a text console)
-V: continuously vary the total energy available in the environment
-v: start the varying after desired number of time steps
-x: set the food +/- button increment size (default is 100000)
-k: keep trying until a viable population is reached (more than 50000 steps)
-B: read initial bugs from a creature file (default is creature)
-b: number of bugs to seed from initial creature file (default is 1)
-o: output global stats once every n steps (default is 50)
-n: automatically save to file every n timesteps (default is none)
-N: name of file to save/load simulation to (default is ./neo.out)
-X: also print all creature genome characteristics to the file ./neo.out.stats
-Q: automatically quit when the population falls to zero

Individual creatures may be selected and their neural nets displayed
during runtime. They may then be saved to a file to be used as seeds
for later simulation runs. Statistics may be written at regular
intervals to a text file, and other statistics may be displayed via
histograms during a run.

Additional information:

I am not currently developing this program anymore. It still has some
bugs and may crash for no particular reason. Use at your own risk.

However it does run pretty stably for me. The graph/statistics window
may be particularly buggy. Although I rarely have any problems and it
does not cause any system-wide problems, use at your own risk.

I wanted to release the software so others could play with it, perhaps
modify it, and use it to learn about artificial life and neural
nets. The source code is distributed under the GPL (see COPYING).

Though not required, I would be eager to hear about any ideas or
modifications that anyone chooses to make to the software.

------------------------------------------------------------

Thanks to Sourceforge (http://sourceforge.net) for kindly hosting this
project.
