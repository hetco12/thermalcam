read me changes here are simplified. the idea is to update this branch and when all modes tested, and features tested post to main branch 

in this iteration:

1) autorange temp feature, great for looking at heat at a distance, or more color details of current object
2) pixel writes to 64 color rectangles at a time, however overhead of transfer seems to undermine gains so far
3) working on main loops unrolling, and methods to do so nicely. seems up to 760 microseconds in calulations for 64x64




coming soon or already in some form or another

buffer removal 16x16 [ ] if checked done

buffer removal 32x32 [ ] if checked done

resolution increased to 128x128 [ ]

include code for parallel display[ ]



