# Pathingman 2.0

An unbiased pathtracer. Upgraded and it's much faster.

![pathingman](https://user-images.githubusercontent.com/16845654/43672771-b4675076-9769-11e8-9ca7-5e0e685ecd7a.png)

## Internals

0. Unbiased pathtracer: pure Monte Carlo integrator with lambert surface model.
1. Image is rendered in 64x64 tiles, multithreaded.
2. Primitives are partitioned by a bounding volume hierarchy, using Surface Area Heuristic splitting method.

## Screenshots

Scene: 25577 triangles, 2048 samples per pixel

Hardware: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80Hz

Performance: 19 minutes and 48 seconds


![capture](https://user-images.githubusercontent.com/16845654/43681979-e06b2594-9818-11e8-84b5-d237e47ce272.JPG)