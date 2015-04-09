### safeio-win32



Safe/atomic file i/o routines for environments where
multiple threads/processes can write to the same
output file. This can happen in build systems where the
dependencies aren't known (or it is too expensive to find
out), and rather than building things once it's easier/faster
to take the hit of doing some work multiple times.

Amit Bakshi
Jan 2015


## The Problem

Demonstrates how to safely write to output files in tools where
you may have many processes trying to read from and write to the
same files. This usually happens when you can't (or don't want to)
work out the dependencies between threads ahead of time. For example,
if you have 3D models that references materials, and they reference
textures.


Car.mb   ->  lambert1.mat -> white.png
						  -> foo1.png
						  -> foo2.png

Chair.mb ->  lambert2.mat -> foo3.png
						  -> white.png
						  -> foo1.png

Door.mb  ->  lambert3.mat -> white.png
						  -> foo4.png
						  -> foo5.png


Let's say you have 2 threads and are using 1 per mb file. As each thread
descends into the materials and gets a list of texture pngs, you discover
that the .dds (compressed) version of the pngs are missing. At time 1 (T1)
process P1 and P2 start.

T1:
	P1 white.png (missing) -> comp white.dds OK
	P2 foo3.png  (missing) -> comp foo3.dds  OK

T2:
	P1 foo1.png (missing)  -> comp foo1.dds  OK?
	P2 white.dds (exists)  -> load white.dds
	P2 foo1.dds (exists)   -> load foo1.dds  FAIL?


There are several issues for T2.

1. If P1 comes along first and starts writing to the output file (foo1.dds),
then P2 would see that foo1.dds already exists. How would P2 know the
wether it is still being processed or it is done? If it opened an in-flight
foo1.dds then P2 would read an incomplete/corrupt file, or it would fail to
open it due to a sharing violation.

2. Another problem is that what if the build process is interrupted (either
a crash, or cancelling a build, or Ctrl-C) and you're in the middle of
conversion? You leave behind a corrupt, half-written-to file, and there is
no way for you to know which one is causing the problems because the timestamp
checks that most build systems use don't tell you that you didn't finish foo1.dds.

Coordinating this, especially across process boundaries, is expensive. It happens
happens enough times that we should fix it, but we should consider simplicity
and speed in the design.

## The Solution

Luckily there is a well known, very easy solution to this. Write to a temporary
file first, then rename this file to the final output. The temp file name has to
be unique across all processes. The easiest way to generate a safe temp file name
is to simply append the PID and/or TID to the name. Let's look at T2 again:


T2:
	P1 foo1.png (missing)  -> comp foo1.dds9213 -> rename foo1.dds
	P2 white.dds (exists)  -> load white.dds
	P2 foo1.dds (exists)   -> comp foo1.dds8869 -> rename foo1.dds


Now you have both processes writing to their own unique foo1.dds<thread_id>, and
the rename ensures that the creation of foo1.dds is seen as a transaction. Either
the file is there and it's complete or it's not there. This takes care of
problem (2).

The question is what happens if both processes try to rename their respective
foo1xx.dds to foo1.dds at the exact same time? The answer is `it depends`.

## POSIX

On a POSIX system (such as Linux or OSX), `rename(2)` is atomic and you have reference
counted inodes. That is there is never a 'hole' in the existence of the file.
In the case of the target existing, the target isn't quickly deleted, and the old one
renamed. This would make it non-atomic. There's a point in time where the file
doesn't exist. If another process has the old target open, that's ok, it's refcounted.
The filesystem has a refcount, and your process has a refcount. When you remove it
from the filesystem, it's gone from its point of view, and the process can keep
reading from the 'zombie' file as long as it holds it open. As soon as the process
exits or closes the handle, the underlying inodes and data is freed. So the answer
on POSIX is _always use rename(2)_. That's it.


## Windows




