# rtems-ecap
Combining RTEMS and the TI AM335x eCAP peripheral for 1D Markov localization

Hi! This is a very old repo that implements 1D Markov localization in C running in an [RTEMS](https://www.rtems.org) task on the BeagleBone Black, and making use of the TI AM335x' eCAP peripheral. The reason to do this is to determine the angle of a rotating toothed wheel without having to have separate code paths for every possible configuration (extra-tooth, missing-tooth, etc)

I haven't touched it in a _very_ long time, but I still think the idea is _super_ cool, so here it is.

I've got kind of a similar implementation in Erlang that uses a NIF to read/write to the eCAP. The eventual goal was to run the BEAM directly on the hardware, but that never really panned out.
