# Arvand2011

This is the "official" distribution of Arvand2011.

## Licence

Arvand2011 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Arvand2011 is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

## Build Instructions

Arvand2011 is built on top of Fast Downward (FD) code base: http://www.fast-downward.org.
Hence, the building process is the same:

```
git clone https://github.com/nhootan/Arvand2011.git
cd Arvand2011
./build_all
```

Parts of FD are implemented in C++, and parts are implemented
in Python. You will need a Python interpreter, version 2.7  to
run it.

## Running the Planner

To run the algorithm once to find a single plan, use:
```
cd Arvand2011
./plan domainfile problemfile --search "mrw(ff())"
```

To iteratively generate better quality plans, use:
```
./plan domainfile problemfile "iterated(mrw(ff()), repeat_last=true)"
```

To generate many plans, use:
```
./plan domainfile problemfile "iterated(mrw(ff()), repeat_last=true, pass_bound=false)"
```

To get a description of the available parameters for Arvand use
```
search/downward-2 --help mrw
```

Please control the memory and time limits (ulimit) from outside the program.

## Questions and Feedback

Email (in domain gmail.com): nhootan

References

H. Nakhost. Random Walk Planning: Theory, Practice, and Application. PhD thesis, University of Alberta, 2013. Dissertation Award for best PhD thesis from the Canadian Artificial Intelligence Association.

