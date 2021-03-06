Building the SSR as MEX-file for GNU Octave and MATLAB
======================================================

Compile for GNU Octave:

    make octave

Compile for Matlab:

    make matlab

Matlab comes with a very old default GCC, so you may have to create
a symbolic link in `$MATLAB_DIR/sys/os/glnxa64/` pointing to
`/usr/lib/x86_64-linux-gnu/libstdc++.so.6` (or similar).

Remove all generated files:

    make clean

Usage example:

``` octave

inputblock = transpose(single([0 0 1 0 0 0 0 0]));
sources = size(inputblock, 2);
blocksize = size(inputblock, 1);
fs = 44100;
setup = '../data/reproduction_setups/circle.asd';
ssr_nfc_hoa('init', setup, sources, blocksize, fs)
positions = [0; 2];  % one column for each source
orientations = -90;  % row vector of angles in degrees
models = { 'plane' };
ssr_nfc_hoa('source', 'position', positions)
ssr_nfc_hoa('source', 'orientation', orientations)
ssr_nfc_hoa('source', 'model', models{:})
% TODO: set more source parameters
% process (and discard) one block for interpolation
outputblock = ssr_nfc_hoa('process', single(zeros(blocksize, sources)));
% now the source parameters have reached their desired values
outputblock = ssr_nfc_hoa('process', inputblock);
% do something with 'outputblock' ...
% repeat for each block ...
ssr_nfc_hoa out_channels
ssr_nfc_hoa clear
ssr_nfc_hoa help

```
