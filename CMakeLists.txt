cmake_minimum_required (VERSION 3.11)
project(pkimage)

 # add the source files as usual
add_executable(${CMAKE_PROJECT_NAME}
  qoi.c
  qoi.h
  pkimage.c
  sokol_args.h
  stb.c
  stb_image.h
  stb_image_write.h
  stb_image_resize2.h
)

