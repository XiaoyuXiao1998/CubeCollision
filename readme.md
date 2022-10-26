# UE4 simulating 1000 cubes free fall with compute shader
This project is a starting point use compute shaders to do physics simulation in UE5. It implements a 1000 particles collsion simulation in the GPU.



https://user-images.githubusercontent.com/55775855/198024470-e80c07cb-a7de-44fd-b210-d0887ea225d5.mp4



### Impletion details
**1. simlation method**

I use Discrete Element Method(DEM) to simulate particle collision.

**2. Accleration method of neighborhood search**

* I'm using a hashed grid to accelerate  the neighborhood search of particles.
* Be sure that the particle size must be smaller than the grid size.
* For detailed hashed algorithm, you can look into the refrence tutorial.


**3. TODO**
* Use prefix sum to accelerate neighborhood search.
* Use UInstanceStaticMeshComponent to speed up the rendering.



#### reference
1.[DEM method of particle collision simulation](https://zhuanlan.zhihu.com/p/563182093)

