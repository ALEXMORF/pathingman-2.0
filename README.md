# Pathingman 2.0

An unbiased pathtracer. Upgraded and it's much faster.

![512spp_sphinx](https://user-images.githubusercontent.com/16845654/48231093-f796fb00-e361-11e8-9710-b7be9c944631.png)

## Internals

0. Unbiased pathtracer: pure Monte Carlo integrator with lambert surface model.
1. Image is rendered in 64x64 tiles, multithreaded.
2. Primitives are partitioned by a bounding volume hierarchy, using Surface Area Heuristic splitting method.

## Screenshots

Scene: 25577 triangles, 2048 samples per pixel

Hardware: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80Hz

Performance: 19 minutes and 48 seconds

![capture](https://user-images.githubusercontent.com/16845654/43681979-e06b2594-9818-11e8-84b5-d237e47ce272.JPG)

Scene: 25577 triangles, 512 samples per pixel

Performance: 2 minute and 47 seconds

![512spp_sphinx](https://user-images.githubusercontent.com/16845654/48231093-f796fb00-e361-11e8-9710-b7be9c944631.png)
