# pk-image
Quick'n'dirty tool for converting and resizing images.
It outputs .png or .qoi images.
Arguments: \
in=path/to/image \
out=path/to/image \
r=shrink factor as int (optional)\
mask=path/to/image (optional)\
The mask argument packs the red channel of the specified image into the alpha channel of the output. Useful for packing specular/roughness maps or just alpha masks.
