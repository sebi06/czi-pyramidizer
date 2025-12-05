# Usage Examples

Practical scenarios for `czi-pyramidizer`.

## 1. Checking for presence of pyramid

### Case where pyramid is needed

```
>czi-pyramidizer -s  /hdd/data/CZI/TileRegion-Stitching-01.czi  --check-only
For the specified document a pyramid should be created.
> echo $?
10
```

### Case where pyramid is not needed

```
>czi-pyramidizer -s  /hdd/data/CZI/New-01.czi  --check-only
The specified document does not need a pyramid or already contains it.
> echo $?
0
```

## 2. Generating a pyramid

### Standard Usage
```
>czi-pyramidizer -s  /hdd/data/CZI/New-01.czi  -d /hdd/data/CZI/New-01-pyramid.czi
The specified document does not need a pyramid or already contains it.
> echo $?
11
```

The default behavior is to skip pyramid creation if not needed - as in this case. The tool
will exit with code 11 to indicate no pyramid was created. No output file is created.

Here is an example where a pyramid is actually created:
```
>czi-pyramidizer -s /hdd/data/CZI/TileRegion-Stitching-01.czi -d out.czi
Operational Parameters
----------------------

Source-CZI-URI: /hdd/data/CZI/TileRegion-Stitching-01.czi
Destination-CZI-URI: out.czi
Pyramid-Tile-Size: 1024
Max-Top-Level-Pyramid-Size: 1024
Compression-Mode: zstd1 (from source file)
Background Color: (0,0,0)

SubBlock-Statistics
-------------------

SubBlock-Count: 6518
Bounding-Box (All)   : X=0 Y=0 W=5743 H=286
Bounding-Box (Layer0): X=0 Y=0 W=5743 H=286
M-Index: min=0 max=0
Bounds:
  Z -> start=0 size=3259
  C -> start=0 size=2
  S -> start=0 size=1
  H -> start=0 size=1
Scene-BoundingBoxes:
  Scene 0:
    All   : X=0 Y=0 W=5743 H=286
    Layer0: X=0 Y=0 W=5743 H=286

Copy Attachments: 3 attachments done.                 
```

### Forcing Pyramid Creation

With the switch `-n Always`, a pyramid will be created regardless of whether it is needed:

```
>czi-pyramidizer -s  /hdd/data/CZI/New-01.czi  -d /hdd/data/CZI/New-01-pyramid.czi -n Always
Operational Parameters
----------------------

Source-CZI-URI: /hdd/data/CZI/New-01.czi
Destination-CZI-URI: /hdd/data/CZI/New-01-pyramid.czi
Pyramid-Tile-Size: 1024
Max-Top-Level-Pyramid-Size: 1024
Compression-Mode: zstd1 (from source file)
Background Color: (0,0,0)

SubBlock-Statistics
-------------------

SubBlock-Count: 45626
Bounding-Box (All)   : X=0 Y=0 W=5744 H=288
Bounding-Box (Layer0): X=0 Y=0 W=5743 H=286
M-Index: min=0 max=0
Bounds:
  Z -> start=0 size=3259
  C -> start=0 size=2
  S -> start=0 size=1
  H -> start=0 size=1
Scene-BoundingBoxes:
  Scene 0:
    All   : X=0 Y=0 W=5744 H=288
    Layer0: X=0 Y=0 W=5743 H=286

Pyramidize: plane Z798C1S0H0C, 1 of 1 subblocks done. 
```

The tool will create the pyramid even though the source document does not require one or it is already present. It prints out operational parameters and
progress information.
